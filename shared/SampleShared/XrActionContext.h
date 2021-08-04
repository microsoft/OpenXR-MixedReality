// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <set>
#include <list>
#include <unordered_map>
#include <unordered_set>

#include "XrUtility/XrString.h"
#include "XrUtility/XrHandle.h"

namespace sample {
    class ActionSet {
    public:
        ActionSet(XrInstance instance, const char* name, const char* localizedName, uint32_t priority = 0)
            : m_instance(instance) {
            XrActionSetCreateInfo actionSetCreateInfo{XR_TYPE_ACTION_SET_CREATE_INFO};
            strcpy_s(actionSetCreateInfo.actionSetName, name);
            strcpy_s(actionSetCreateInfo.localizedActionSetName, localizedName);
            actionSetCreateInfo.priority = priority;
            CHECK_XRCMD(xrCreateActionSet(m_instance, &actionSetCreateInfo, m_actionSet.Put()));
        }

        XrAction CreateAction(const char* actionName,
                              const char* localizedName,
                              XrActionType actionType,
                              const std::vector<std::string>& subactionPaths) {
            std::vector<XrPath> subActionXrPaths = xr::StringsToPaths(m_instance, subactionPaths);

            XrActionCreateInfo actionCreateInfo{XR_TYPE_ACTION_CREATE_INFO};
            actionCreateInfo.actionType = actionType;
            actionCreateInfo.countSubactionPaths = static_cast<uint32_t>(subActionXrPaths.size());
            actionCreateInfo.subactionPaths = subActionXrPaths.data();
            strcpy_s(actionCreateInfo.actionName, actionName);
            strcpy_s(actionCreateInfo.localizedActionName, localizedName);

            for (XrPath subactionPath : subActionXrPaths) {
                m_declaredSubactionPaths.insert(subactionPath); // std::set will remove duplication.
            }

            xr::ActionHandle action;
            CHECK_XRCMD(xrCreateAction(m_actionSet.Get(), &actionCreateInfo, action.Put()));

            m_actions.push_back(std::move(action));

            return m_actions.back().Get();
        }

        bool Active() const {
            return m_active;
        }
        void SetActive(bool active) {
            m_active = active;
        }

        XrActionSet Handle() const {
            return m_actionSet.Get();
        }

        const std::set<XrPath>& DeclaredSubactionPaths() const {
            return m_declaredSubactionPaths;
        }

    private:
        const XrInstance m_instance;
        xr::ActionSetHandle m_actionSet;
        std::vector<xr::ActionHandle> m_actions;
        bool m_active{true};
        std::set<XrPath> m_declaredSubactionPaths;
    };

    //
    // OpenXR requires one xrSuggestInteractionProfileBindings call for each interaction profile
    // and one xrAttachSessionActionSets for each session.
    // The xrSyncActions call must be done for all active actionset together.
    // ActionContext class collects action and actionset metadata from multiple places in an app
    // and finalize the binding and attach to session together.
    //
    struct ActionContext {
        struct ActionBinding {
            XrAction action;
            std::string binding;
        };

        explicit ActionContext(XrInstance instance)
            : m_instance(instance) {
        }

        ActionSet& CreateActionSet(const char* name, const char* localizedName, uint32_t priority = 0) {
            return m_actionSets.emplace_back(ActionSet{m_instance, name, localizedName, priority});
        }

        void SuggestInteractionProfileBindings(const char* interactionProfile, const std::vector<ActionBinding>& suggestedBindings) {
            const XrPath profilePath = xr::StringToPath(m_instance, interactionProfile);
            for (const auto& actionBinding : suggestedBindings) {
                m_actionBindings[profilePath].emplace_back(actionBinding);
            }
        }

    private:
        XrInstance m_instance;
        std::list<ActionSet> m_actionSets;
        std::unordered_map<XrPath /*interaction profile*/, std::vector<ActionBinding>> m_actionBindings;

        friend void AttachActionsToSession(XrInstance instance,
                                           XrSession session,
                                           const std::vector<const ActionContext*>& actionContexts,
                                           const std::vector<std::string>& interactionProfilesFilter);
        friend void SyncActions(XrSession session, const std::vector<const ActionContext*>& actionContexts);
    };

    inline void AttachActionsToSession(XrInstance instance,
                                       XrSession session,
                                       const std::vector<const ActionContext*>& actionContexts,
                                       const std::vector<std::string>& interactionProfilesFilter) {
        const bool hasProfileFilter = !interactionProfilesFilter.empty();
        std::unordered_set<XrPath> enabledProfiles;
        if (hasProfileFilter) {
            for (const auto& profileString : interactionProfilesFilter) {
                XrPath path = XR_NULL_PATH;
                CHECK_XRCMD(xrStringToPath(instance, profileString.c_str(), &path));
                enabledProfiles.insert(path);
            }
        }

        // Collect action bindings for each context and summarize using interaction profile path as key.
        std::unordered_map<XrPath, std::vector<XrActionSuggestedBinding>> allBindings;
        for (const ActionContext* actionContext : actionContexts) {
            for (const auto& [profilePath, stringBindings] : actionContext->m_actionBindings) {
                if (hasProfileFilter && enabledProfiles.find(profilePath) == enabledProfiles.end()) {
                    continue;    // skip the profile if app didn't ask for it.
                }
                for (const auto& [actionPath, binding] : stringBindings) {
                    allBindings[profilePath].emplace_back(
                        XrActionSuggestedBinding{actionPath, xr::StringToPath(instance, binding.c_str())});
                }
            }
        }

        for (const auto& [interactionProfile, suggestedBindings] : allBindings) {
            XrInteractionProfileSuggestedBinding bindings{XR_TYPE_INTERACTION_PROFILE_SUGGESTED_BINDING};
            bindings.interactionProfile = interactionProfile;
            bindings.suggestedBindings = suggestedBindings.data();
            bindings.countSuggestedBindings = static_cast<uint32_t>(suggestedBindings.size());
            CHECK_XRCMD(xrSuggestInteractionProfileBindings(instance, &bindings));
        }

        std::vector<XrActionSet> actionSetHandles;
        for (const ActionContext* actionContext : actionContexts) {
            for (const ActionSet& actionSet : actionContext->m_actionSets) {
                actionSetHandles.push_back(actionSet.Handle());
            }
        }

        if (!std::empty(actionSetHandles)) {
            XrSessionActionSetsAttachInfo attachInfo{XR_TYPE_SESSION_ACTION_SETS_ATTACH_INFO};
            attachInfo.countActionSets = static_cast<uint32_t>(std::size(actionSetHandles));
            attachInfo.actionSets = actionSetHandles.data();
            CHECK_XRCMD(xrAttachSessionActionSets(session, &attachInfo));
        }
    }

    inline void SyncActions(XrSession session, const std::vector<const ActionContext*>& actionContexts) {
        std::vector<XrActiveActionSet> activeActionSets;

        for (const ActionContext* actionContext : actionContexts) {
            for (const ActionSet& actionSet : actionContext->m_actionSets) {
                if (!actionSet.Active()) {
                    continue;
                }
                if (std::empty(actionSet.DeclaredSubactionPaths())) {
                    activeActionSets.emplace_back(XrActiveActionSet{actionSet.Handle(), XR_NULL_PATH});
                } else {
                    for (const XrPath& subactionPath : actionSet.DeclaredSubactionPaths()) {
                        activeActionSets.emplace_back(XrActiveActionSet{actionSet.Handle(), subactionPath});
                    }
                }
            }
        }

        if (!activeActionSets.empty()) {
            XrActionsSyncInfo syncInfo{XR_TYPE_ACTIONS_SYNC_INFO};
            syncInfo.countActiveActionSets = static_cast<uint32_t>(std::size(activeActionSets));
            syncInfo.activeActionSets = activeActionSets.data();
            CHECK_XRCMD(xrSyncActions(session, &syncInfo));
        }
    }

} // namespace sample
