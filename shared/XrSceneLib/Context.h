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
#include <XrUtility/XrInstanceContext.h>
#include <XrUtility/XrExtensionContext.h>
#include <XrUtility/XrSystemContext.h>
#include <XrUtility/XrSessionContext.h>

namespace engine {

    // Session-related resources shared across multiple Scenes.
    struct Context final {
        Context(xr::InstanceContext instance,
                xr::ExtensionContext extensions,
                xr::SystemContext system,
                xr::SessionContext session,
                XrSpace appSpace,
                Pbr::Resources pbrResources,
                winrt::com_ptr<ID3D11Device> device,
                winrt::com_ptr<ID3D11DeviceContext> deviceContext)
            : Instance(std::move(instance))
            , Extensions(std::move(extensions))
            , System(std::move(system))
            , Session(std::move(session))
            , AppSpace(appSpace)
            , PbrResources(std::move(pbrResources))
            , Device(std::move(device))
            , DeviceContext(std::move(deviceContext)) {
        }

        const xr::InstanceContext Instance;
        const xr::ExtensionContext Extensions;
        const xr::SystemContext System;
        const xr::SessionContext Session;

        const XrSpace AppSpace;

        const winrt::com_ptr<ID3D11DeviceContext> DeviceContext;
        const winrt::com_ptr<ID3D11Device> Device;
        Pbr::Resources PbrResources;
    };

} // namespace engine
