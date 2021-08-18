// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <pbr/PbrResources.h>
#include <XrUtility/XrString.h>
#include <XrUtility/XrExtensionContext.h>
#include <SampleShared/XrInstanceContext.h>
#include <SampleShared/XrSystemContext.h>
#include <SampleShared/XrSessionContext.h>

namespace engine {

    // Session-related resources shared across multiple Scenes.
    struct Context final {
        Context(sample::InstanceContext instance,
                xr::ExtensionContext extensions,
                sample::SystemContext system,
                sample::SessionContext session,
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

        const xr::ExtensionContext Extensions;
        const sample::InstanceContext Instance;
        const sample::SystemContext System;
        const sample::SessionContext Session;

        const XrSpace AppSpace;

        const winrt::com_ptr<ID3D11DeviceContext> DeviceContext;
        const winrt::com_ptr<ID3D11Device> Device;
        Pbr::Resources PbrResources;
    };

} // namespace engine
