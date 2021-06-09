// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "XrSceneUnderstanding.hpp"

namespace xr::su {
    struct SceneFragment {
        using Id = TypedUuid<xr::su::SceneObject>;
        Id id;
        XrTime updateTime;
    };

    inline std::vector<SceneFragment> GetSerializedSceneFragments(XrSceneMSFT scene, const xr::ExtensionDispatchTable& extensions) {
        XrSceneComponentsGetInfoMSFT getInfo{XR_TYPE_SCENE_COMPONENTS_GET_INFO_MSFT};
        getInfo.componentType = XR_SCENE_COMPONENT_TYPE_SERIALIZED_SCENE_FRAGMENT_MSFT;

        XrSceneComponentsMSFT sceneComponents{XR_TYPE_SCENE_COMPONENTS_MSFT};
        CHECK_XRCMD(extensions.xrGetSceneComponentsMSFT(scene, &getInfo, &sceneComponents));
        const uint32_t count = sceneComponents.componentCountOutput;

        std::vector<XrSceneComponentMSFT> components(count);
        sceneComponents.componentCapacityInput = count;
        sceneComponents.components = components.data();

        CHECK_XRCMD(extensions.xrGetSceneComponentsMSFT(scene, &getInfo, &sceneComponents));

        std::vector<SceneFragment> result(count);
        for (uint32_t k = 0; k < count; k++) {
            result[k].id = components[k].id;
            result[k].updateTime = components[k].updateTime;
        }
        return result;
    }

    inline std::vector<uint8_t> ReadSceneFragmentData(XrSceneMSFT scene,
                                                      const xr::ExtensionDispatchTable& extensions,
                                                      const SceneFragment::Id& id) {
        uint32_t readOutput = 0;
        XrSerializedSceneFragmentDataGetInfoMSFT getInfo{XR_TYPE_SERIALIZED_SCENE_FRAGMENT_DATA_GET_INFO_MSFT};
        getInfo.sceneFragmentId = static_cast<XrUuidMSFT>(id);
        CHECK_XRCMD(extensions.xrGetSerializedSceneFragmentDataMSFT(scene, &getInfo, 0, &readOutput, nullptr));

        std::vector<uint8_t> buffer(readOutput);
        CHECK_XRCMD(extensions.xrGetSerializedSceneFragmentDataMSFT(scene, &getInfo, readOutput, &readOutput, buffer.data()));
        return buffer;
    }

} // namespace xr::su
