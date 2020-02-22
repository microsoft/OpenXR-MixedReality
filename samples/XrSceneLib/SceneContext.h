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

#include <pbr/PbrResources.h>
#include <XrUtility/XrString.h>
#include <XrUtility/XrExtensions.h>
#include <XrUtility/XrInstanceContext.h>
#include <XrUtility/XrActionContext.h>

// Session-related resources shared across multiple Scenes.
struct SceneContext final {
    SceneContext(xr::InstanceContext instance,
                 xr::SystemContext system,
                 XrSession session,
                 XrSpace sceneSpace,
                 Pbr::Resources pbrResources,
                 winrt::com_ptr<ID3D11Device> device,
                 winrt::com_ptr<ID3D11DeviceContext> deviceContext,
                 xr::ActionContext& actionContext,
                 XrEnvironmentBlendMode primaryViewConfigEnvironmentBlendMode)
        : Instance(std::move(instance))
        , System(std::move(system))
        , Extensions(std::cref(Instance.Extensions))
        , Session(session)
        , SceneSpace(sceneSpace)
        , PbrResources(std::move(pbrResources))
        , Device(std::move(device))
        , DeviceContext(std::move(deviceContext))
        , PrimaryViewConfigEnvironmentBlendMode(primaryViewConfigEnvironmentBlendMode)
        , ActionContext(actionContext)
        , LeftHand(xr::StringToPath(Instance.Handle(), "/user/hand/left"))
        , RightHand(xr::StringToPath(Instance.Handle(), "/user/hand/right"))
        , GamePad(xr::StringToPath(Instance.Handle(), "/user/gamepad")) {
    }

    const xr::InstanceContext Instance;
    const xr::SystemContext System;
    const xr::ExtensionContext& Extensions;

    const XrSession Session;
    const XrEnvironmentBlendMode PrimaryViewConfigEnvironmentBlendMode;
    const XrSpace SceneSpace;

    const winrt::com_ptr<ID3D11DeviceContext> DeviceContext;
    const winrt::com_ptr<ID3D11Device> Device;
    Pbr::Resources PbrResources;

    xr::ActionContext& ActionContext;
    std::atomic<XrSessionState> SessionState;

    const XrPath RightHand;
    const XrPath LeftHand;
    const XrPath GamePad;

    XrPath PrimaryHand() const {
        return m_primaryIsRight ? RightHand : LeftHand;
    }
    XrPath SecondaryHand() const {
        return m_primaryIsRight ? LeftHand : RightHand;
    }
    void SetPrimaryHand(XrPath path) {
        assert(path == RightHand || path == LeftHand);
        m_primaryIsRight = (path == RightHand);
    }

private:
    bool m_primaryIsRight{true};
};
