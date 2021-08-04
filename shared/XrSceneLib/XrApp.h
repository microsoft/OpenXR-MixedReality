// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "Scene.h"
#include "Context.h"
#include "ProjectionLayer.h"

namespace engine {
    class XrApp {
    public:
        virtual ~XrApp() = default;

        virtual Context& Context() const = 0;

        virtual void AddScene(std::unique_ptr<Scene> scene) = 0;
        virtual const std::vector<std::unique_ptr<Scene>>& Scenes() const = 0;

        virtual void Run() = 0;
        virtual bool Step() = 0;
        virtual void Stop() = 0;

        virtual ProjectionLayers& ProjectionLayers() = 0;

    };

    struct XrAppConfiguration {
        XrAppConfiguration(const xr::NameVersion appInfo)
            : AppInfo(std::move(appInfo)) {
        }

        const xr::NameVersion AppInfo;
        std::vector<std::string> RequestedExtensions;
        std::vector<std::string> InteractionProfilesFilter;
        bool SingleThreadedD3D11Device{false};
        bool RenderSynchronously{false};
        std::optional<XrHolographicWindowAttachmentMSFT> HolographicWindowAttachment{std::nullopt};
    };

    std::unique_ptr<XrApp> CreateXrApp(XrAppConfiguration appConfiguration);
} // namespace engine

