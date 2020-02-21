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
#include "pch.h"
#include "ActionContext.h"

#include <XrUtility/XrString.h>

ActionContext::ActionContext(XrInstance instance)
    : m_instance(instance)
    , RightHand(xr::StringToPath(instance, "/user/hand/right"))
    , LeftHand(xr::StringToPath(instance, "/user/hand/left"))
    , GamePad(xr::StringToPath(instance, "/user/gamepad")) {
    m_primaryHandPath = RightHand;
    m_secondaryHandPath = LeftHand;
}

ActionContext::~ActionContext() {
    for (const auto& action : m_actions) {
        CHECK_XRCMD(xrDestroyAction(action));
    }
    for (const auto& actionSetData : m_actionSets) {
        CHECK_XRCMD(xrDestroyActionSet(actionSetData->ActionSet));
    }
}

ActionSetData* ActionContext::CreateActionSet(const std::string& name, const std::string& localizedName, uint32_t priority /*= scenes::priorities::Default*/) {
    XrActionSet actionSet;
    XrActionSetCreateInfo actionSetCreateInfo{XR_TYPE_ACTION_SET_CREATE_INFO};
    strcpy_s(actionSetCreateInfo.actionSetName, name.c_str());
    strcpy_s(actionSetCreateInfo.localizedActionSetName, localizedName.c_str());
    actionSetCreateInfo.priority = priority;
    CHECK_XRCMD(xrCreateActionSet(m_instance, &actionSetCreateInfo, &actionSet));

    auto actionSetData = std::make_unique<ActionSetData>();
    actionSetData->ActionSet = actionSet;
    actionSetData->Active = true;

    std::lock_guard lock(m_mutex);
    return m_actionSets.emplace_back(std::move(actionSetData)).get();
}

XrAction ActionContext::CreateAction(XrActionSet actionSet,
                                     const std::string& actionName,
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

    XrAction action;
    CHECK_XRCMD(xrCreateAction(actionSet, &actionCreateInfo, &action));

    std::lock_guard lock(m_mutex);
    m_actions.push_back(action);

    return action;
}

void ActionContext::SuggestInteractionProfileBindings(const std::string& interactionProfile,
                                                      const std::vector<std::pair<XrAction, std::string>>& suggestedBindings) {
    const XrPath profilePath = xr::StringToPath(m_instance, interactionProfile.c_str());
    for (const auto& [action, suggestedBinding] : suggestedBindings) {
        m_actionBindings[profilePath].push_back({action, xr::StringToPath(m_instance, suggestedBinding.c_str())});
    }
}

void ActionContext::SyncActions(XrSession session) {
    std::lock_guard lock(m_mutex);

    std::vector<XrActiveActionSet> activeActionSets;

    for (const auto& actionSetData : m_actionSets) {
        if (!actionSetData->Active) {
            continue;
        }
        XrActiveActionSet activeActionSet;
        activeActionSet.actionSet = actionSetData->ActionSet;
        activeActionSet.subactionPath = XR_NULL_PATH;
        activeActionSets.push_back(activeActionSet);
    }

    if (!std::empty(activeActionSets)) {
        XrActionsSyncInfo syncInfo{XR_TYPE_ACTIONS_SYNC_INFO};
        syncInfo.countActiveActionSets = static_cast<uint32_t>(std::size(activeActionSets));
        syncInfo.activeActionSets = activeActionSets.data();
        CHECK_XRCMD(xrSyncActions(session, &syncInfo));
    }
}

void ActionContext::AttachActionsToSession(XrSession session) {
    std::lock_guard lock(m_mutex);

    // TODO 20504296 add print out of interaction profile events / result of getters
    for (const auto& [interactionProfile, bindingsList] : m_actionBindings) {
        XrInteractionProfileSuggestedBinding bindings{XR_TYPE_INTERACTION_PROFILE_SUGGESTED_BINDING};
        bindings.interactionProfile = interactionProfile;
        bindings.suggestedBindings = bindingsList.data();
        bindings.countSuggestedBindings = static_cast<uint32_t>(bindingsList.size());
        CHECK_XRCMD(xrSuggestInteractionProfileBindings(m_instance, &bindings));
    }

    {
        std::vector<XrActionSet> actionSetHandles;
        for (const auto& actionSetData : m_actionSets) {
            actionSetHandles.push_back(actionSetData->ActionSet);
        }
        XrSessionActionSetsAttachInfo attachInfo{XR_TYPE_SESSION_ACTION_SETS_ATTACH_INFO};
        attachInfo.countActionSets = static_cast<uint32_t>(std::size(actionSetHandles));
        attachInfo.actionSets = actionSetHandles.data();
        CHECK_XRCMD(xrAttachSessionActionSets(session, &attachInfo));
    }
}

XrPath ActionContext::PrimaryHand() const {
    return m_primaryHandPath;
}

XrPath ActionContext::SecondaryHand() const {
    return m_secondaryHandPath;
}

void ActionContext::SetPrimaryHand(XrPath path) {
    assert(path == RightHand || path == LeftHand);
    m_primaryHandPath = path;
    m_secondaryHandPath = (path == RightHand) ? LeftHand : RightHand;
}
