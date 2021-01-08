// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "XrStruct.h"
#include "XrHandle.h"
#include "XrMath.h"
#include "XrExtensions.h"

namespace xr {
    class SceneObserverHandle : public xr::UniqueExtHandle<XrSceneObserverMSFT> {};
    class SceneHandle : public xr::UniqueExtHandle<XrSceneMSFT> {};

    inline xr::SceneObserverHandle CreateSceneObserver(const xr::ExtensionDispatchTable& extensions, XrSession session) {
        xr::SceneObserverHandle sceneObserverHandle;
        XrSceneObserverCreateInfoMSFT createInfo{XR_TYPE_SCENE_OBSERVER_CREATE_INFO_MSFT};
        CHECK_XRCMD(
            extensions.xrCreateSceneObserverMSFT(session, &createInfo, sceneObserverHandle.Put(extensions.xrDestroySceneObserverMSFT)));
        return sceneObserverHandle;
    }

    inline xr::SceneHandle CreateScene(const xr::ExtensionDispatchTable& extensions, XrSceneObserverMSFT sceneObserver) {
        xr::SceneHandle sceneHandle;
        XrSceneCreateInfoMSFT createInfo{XR_TYPE_SCENE_CREATE_INFO_MSFT};
        CHECK_XRCMD(extensions.xrCreateSceneMSFT(sceneObserver, &createInfo, sceneHandle.Put(extensions.xrDestroySceneMSFT)));
        return sceneHandle;
    }

    struct SceneBounds {
        XrSpace space;
        XrTime time;
        std::vector<XrSceneSphereBoundMSFT> sphereBounds;
        std::vector<XrSceneFrustumBoundMSFT> frustumBounds;
        std::vector<XrSceneOrientedBoxBoundMSFT> boxBounds;
    };

    inline void ComputeNewScene(XrSceneObserverMSFT sceneObserver, const xr::ExtensionDispatchTable& extensions, const SceneBounds& bounds, bool disableInferredSceneObjects = false) {
        XrNewSceneComputeInfoMSFT computeInfo{XR_TYPE_NEW_SCENE_COMPUTE_INFO_MSFT};
        computeInfo.bounds.space = bounds.space;
        computeInfo.bounds.time = bounds.time;
        computeInfo.bounds.boxCount = static_cast<uint32_t>(bounds.boxBounds.size());
        computeInfo.bounds.boxes = bounds.boxBounds.data();
        computeInfo.bounds.frustumCount = static_cast<uint32_t>(bounds.frustumBounds.size());
        computeInfo.bounds.frustums = bounds.frustumBounds.data();
        computeInfo.bounds.sphereCount = static_cast<uint32_t>(bounds.sphereBounds.size());
        computeInfo.bounds.spheres = bounds.sphereBounds.data();
        computeInfo.disableInferredSceneObjects = false;
        CHECK_XRCMD(extensions.xrComputeNewSceneMSFT(sceneObserver, &computeInfo));
    }

    // Read mesh vertex/index buffers
    struct SceneMeshBuffers {
        std::vector<XrVector3f> vertexBuffer;
        std::vector<uint32_t> indexBuffer;
    };

    inline void
    ReadMeshBuffers(XrSceneMSFT scene, const xr::ExtensionDispatchTable& extensions, uint64_t meshBufferId, SceneMeshBuffers& buffers) {
        XrSceneMeshBuffersGetInfoMSFT meshGetInfo{XR_TYPE_SCENE_MESH_BUFFERS_GET_INFO_MSFT};
        meshGetInfo.meshBufferId = meshBufferId;

        XrSceneMeshBuffersMSFT meshBuffers{XR_TYPE_SCENE_MESH_BUFFERS_MSFT};
        CHECK_XRCMD(extensions.xrGetSceneMeshBuffersMSFT(scene, &meshGetInfo, &meshBuffers));

        buffers.vertexBuffer.resize(meshBuffers.vertexCountOutput);
        buffers.indexBuffer.resize(meshBuffers.indexCountOutput);
        meshBuffers.vertexCapacityInput = static_cast<uint32_t>(buffers.vertexBuffer.size());
        meshBuffers.indexCapacityInput = static_cast<uint32_t>(buffers.indexBuffer.size());
        meshBuffers.vertices = buffers.vertexBuffer.data();
        meshBuffers.indices = buffers.indexBuffer.data();
        CHECK_XRCMD(extensions.xrGetSceneMeshBuffersMSFT(scene, &meshGetInfo, &meshBuffers));
        buffers.vertexBuffer.resize(meshBuffers.vertexCountOutput);
        buffers.indexBuffer.resize(meshBuffers.indexCountOutput);
    }
} // namespace xr
