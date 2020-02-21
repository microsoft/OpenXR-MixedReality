//*********************************************************
//    Copyright (c) Microsoft. All rights reserved.
//
//    Apache 2.0 License
//
//    You may obtain a copy of the License at
//    http://www.apache.org/licenses/LICENSE-2.0
//
//    Unless required by applicable law or agreed to in writing, software
//    distributed under the License is distributed on an "AS IS" BASIS,
//    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
//    implied. See the License for the specific language governing
//    permissions and limitations under the License.
//
//*********************************************************
#pragma once

#include <set>

#include <XrUtility/XrString.h>
#include <XrUtility/XrHandle.h>

namespace scenes::priorities {
    constexpr uint32_t Default = 0;
    constexpr uint32_t ControllerRendering = 0;
    constexpr uint32_t Menu = 1;
} // namespace scenes::priorities

class SceneActionSet {
public:
    SceneActionSet(XrInstance instance,
                   const std::string& name,
                   const std::string& localizedName,
                   uint32_t priority = scenes::priorities::Default)
        : m_instance(instance) {
        XrActionSetCreateInfo actionSetCreateInfo{XR_TYPE_ACTION_SET_CREATE_INFO};
        strcpy_s(actionSetCreateInfo.actionSetName, name.c_str());
        strcpy_s(actionSetCreateInfo.localizedActionSetName, localizedName.c_str());
        actionSetCreateInfo.priority = priority;
        CHECK_XRCMD(xrCreateActionSet(m_instance, &actionSetCreateInfo, m_actionSet.Put()));
    }

    XrAction CreateAction(const std::string& actionName,
                          const std::string& localizedName,
                          XrActionType actionType,
                          const std::vector<std::string>& subactionPaths) {
        std::vector<XrPath> subActionXrPaths = xr::StringsToPaths(m_instance, subactionPaths);

        XrActionCreateInfo actionCreateInfo{XR_TYPE_ACTION_CREATE_INFO};
        actionCreateInfo.actionType = actionType;
        actionCreateInfo.countSubactionPaths = static_cast<uint32_t>(subActionXrPaths.size());
        actionCreateInfo.subactionPaths = subActionXrPaths.data();
        actionName.copy(actionCreateInfo.actionName, sizeof(actionCreateInfo.actionName));
        localizedName.copy(actionCreateInfo.localizedActionName, sizeof(actionCreateInfo.localizedActionName));

        for (XrPath subactionPath : subActionXrPaths) {
            m_delcaredSubactionPaths.insert(subactionPath);
        }

        xr::ActionHandle action;
        CHECK_XRCMD(xrCreateAction(m_actionSet.Get(), &actionCreateInfo, action.Put()));

        m_actions.push_back(std::move(action));

        return m_actions.back().Get();
    }

    const std::set<XrPath> DeclaredSubactionPaths() const {
        return m_delcaredSubactionPaths;
    }

    XrActionSet Handle() const {
        return m_actionSet.Get();
    }

    bool Active() const {
        return m_active;
    }
    void SetActive(bool active) {
        m_active = active;
    }

private:
    const XrInstance m_instance;
    xr::ActionSetHandle m_actionSet;
    std::vector<xr::ActionHandle> m_actions;
    bool m_active{true};
    std::set<XrPath> m_delcaredSubactionPaths;
};

struct IActionContext {
    virtual ~IActionContext() = default;

    virtual SceneActionSet& CreateActionSet(const std::string& name, const std::string& localizedName, uint32_t priority = 0) = 0;
    virtual void SuggestInteractionProfileBindings(const std::string& interactionProfile,
                                                   const std::vector<std::pair<XrAction, std::string>>& suggestedBindings) = 0;
};

struct ActionContext : IActionContext {
    explicit ActionContext(XrInstance instance)
        : m_instance(instance) {
    }

    SceneActionSet& CreateActionSet(const std::string& name, const std::string& localizedName, uint32_t priority = 0) override;
    void SuggestInteractionProfileBindings(const std::string& interactionProfile,
                                           const std::vector<std::pair<XrAction, std::string>>& suggestedBindings) override;
    void SyncActions(XrSession session);
    void AttachActionsToSession(XrSession session);

private:
    XrInstance m_instance;
    std::list<SceneActionSet> m_actionSets;
    std::unordered_map<XrPath, std::vector<XrActionSuggestedBinding>> m_actionBindings;
};

inline SceneActionSet& ActionContext::CreateActionSet(const std::string& name,
                                                               const std::string& localizedName,
                                                               uint32_t priority) {
    return m_actionSets.emplace_back(m_instance, name, localizedName, priority);
}

inline void ActionContext::SuggestInteractionProfileBindings(
    const std::string& interactionProfile, const std::vector<std::pair<XrAction, std::string>>& suggestedBindings) {
    const XrPath profilePath = xr::StringToPath(m_instance, interactionProfile.c_str());
    for (const auto& [action, suggestedBinding] : suggestedBindings) {
        m_actionBindings[profilePath].push_back({action, xr::StringToPath(m_instance, suggestedBinding.c_str())});
    }
}

inline void ActionContext::SyncActions(XrSession session) {
    std::vector<XrActiveActionSet> activeActionSets;

    for (const auto& actionSet : m_actionSets) {
        if (!actionSet.Active()) {
            continue;
        }
        for (const auto& subactionPath : actionSet.DeclaredSubactionPaths()) {
            XrActiveActionSet activeActionSet;
            activeActionSet.actionSet = actionSet.Handle();
            activeActionSet.subactionPath = subactionPath;
            activeActionSets.push_back(activeActionSet);
        }
    }

    XrActionsSyncInfo syncInfo{XR_TYPE_ACTIONS_SYNC_INFO};
    syncInfo.countActiveActionSets = static_cast<uint32_t>(std::size(activeActionSets));
    syncInfo.activeActionSets = activeActionSets.data();
    CHECK_XRCMD(xrSyncActions(session, &syncInfo));
}

inline void ActionContext::AttachActionsToSession(XrSession session) {
    for (const auto& [interactionProfile, bindingsList] : m_actionBindings) {
        XrInteractionProfileSuggestedBinding bindings{XR_TYPE_INTERACTION_PROFILE_SUGGESTED_BINDING};
        bindings.interactionProfile = interactionProfile;
        bindings.suggestedBindings = bindingsList.data();
        bindings.countSuggestedBindings = static_cast<uint32_t>(bindingsList.size());
        CHECK_XRCMD(xrSuggestInteractionProfileBindings(m_instance, &bindings));
    }

    if (!m_actionSets.empty()) {
        std::vector<XrActionSet> actionSetHandles;
        for (const auto& actionSet : m_actionSets) {
            actionSetHandles.push_back(actionSet.Handle());
        }
        XrSessionActionSetsAttachInfo attachInfo{XR_TYPE_SESSION_ACTION_SETS_ATTACH_INFO};
        attachInfo.countActionSets = static_cast<uint32_t>(std::size(actionSetHandles));
        attachInfo.actionSets = actionSetHandles.data();
        CHECK_XRCMD(xrAttachSessionActionSets(session, &attachInfo));
    }
}
