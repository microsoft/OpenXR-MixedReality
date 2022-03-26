// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "XrStruct.h"
#include "XrHandle.h"
#include "XrMath.h"

namespace xr {
    class SceneObserverHandle : public xr::UniqueXrHandle<XrSceneObserverMSFT> {};
    class SceneHandle : public xr::UniqueXrHandle<XrSceneMSFT> {};

    inline std::vector<XrSceneComputeFeatureMSFT> EnumerateSceneComputeFeatures(XrInstance instance, XrSystemId systemId) {
        uint32_t count = 0;
        CHECK_XRCMD(xrEnumerateSceneComputeFeaturesMSFT(instance, systemId, 0, &count, nullptr));

        std::vector<XrSceneComputeFeatureMSFT> features(count);
        CHECK_XRCMD(xrEnumerateSceneComputeFeaturesMSFT(instance, systemId, count, &count, features.data()));
        features.resize(count);
        return features;
    }

    inline xr::SceneObserverHandle CreateSceneObserver(XrSession session) {
        xr::SceneObserverHandle sceneObserverHandle;
        XrSceneObserverCreateInfoMSFT createInfo{XR_TYPE_SCENE_OBSERVER_CREATE_INFO_MSFT};
        CHECK_XRCMD(xrCreateSceneObserverMSFT(session, &createInfo, sceneObserverHandle.Put(xrDestroySceneObserverMSFT)));
        return sceneObserverHandle;
    }

    inline xr::SceneHandle CreateScene(XrSceneObserverMSFT sceneObserver) {
        xr::SceneHandle sceneHandle;
        XrSceneCreateInfoMSFT createInfo{XR_TYPE_SCENE_CREATE_INFO_MSFT};
        CHECK_XRCMD(xrCreateSceneMSFT(sceneObserver, &createInfo, sceneHandle.Put(xrDestroySceneMSFT)));
        return sceneHandle;
    }

    struct SceneBounds {
        XrSpace space;
        XrTime time;
        std::vector<XrSceneSphereBoundMSFT> sphereBounds;
        std::vector<XrSceneFrustumBoundMSFT> frustumBounds;
        std::vector<XrSceneOrientedBoxBoundMSFT> boxBounds;
    };

    // Begins computing a new scene asynchronously.
    // - visualMeshLevelOfDetail only applies to visual meshes
    inline void ComputeNewScene(XrSceneObserverMSFT sceneObserver,
                                const std::vector<XrSceneComputeFeatureMSFT>& requestedFeatures,
                                const SceneBounds& bounds,
                                XrSceneComputeConsistencyMSFT consistency = XR_SCENE_COMPUTE_CONSISTENCY_SNAPSHOT_COMPLETE_MSFT,
                                std::optional<XrMeshComputeLodMSFT> visualMeshLevelOfDetail = {}) {
        XrNewSceneComputeInfoMSFT computeInfo{XR_TYPE_NEW_SCENE_COMPUTE_INFO_MSFT};
        computeInfo.requestedFeatureCount = static_cast<uint32_t>(requestedFeatures.size());
        computeInfo.requestedFeatures = requestedFeatures.data();
        computeInfo.bounds.space = bounds.space;
        computeInfo.bounds.time = bounds.time;
        computeInfo.bounds.boxCount = static_cast<uint32_t>(bounds.boxBounds.size());
        computeInfo.bounds.boxes = bounds.boxBounds.data();
        computeInfo.bounds.frustumCount = static_cast<uint32_t>(bounds.frustumBounds.size());
        computeInfo.bounds.frustums = bounds.frustumBounds.data();
        computeInfo.bounds.sphereCount = static_cast<uint32_t>(bounds.sphereBounds.size());
        computeInfo.bounds.spheres = bounds.sphereBounds.data();
        computeInfo.consistency = consistency;

        XrVisualMeshComputeLodInfoMSFT computeLod{XR_TYPE_VISUAL_MESH_COMPUTE_LOD_INFO_MSFT};
        if (visualMeshLevelOfDetail.has_value()) {
            computeLod.lod = visualMeshLevelOfDetail.value();
            xr::InsertExtensionStruct(computeInfo, computeLod);
        }
        CHECK_XRCMD(xrComputeNewSceneMSFT(sceneObserver, &computeInfo));
    }

    // Reads mesh vertices and 32-bit indices.
    inline void
    ReadMeshBuffers(XrSceneMSFT scene, uint64_t meshBufferId, std::vector<XrVector3f>& vertexBuffer, std::vector<uint32_t>& indexBuffer) {
        XrSceneMeshBuffersGetInfoMSFT meshGetInfo{XR_TYPE_SCENE_MESH_BUFFERS_GET_INFO_MSFT};
        meshGetInfo.meshBufferId = meshBufferId;

        XrSceneMeshBuffersMSFT meshBuffers{XR_TYPE_SCENE_MESH_BUFFERS_MSFT};
        XrSceneMeshVertexBufferMSFT vertices{XR_TYPE_SCENE_MESH_VERTEX_BUFFER_MSFT};
        XrSceneMeshIndicesUint32MSFT indices{XR_TYPE_SCENE_MESH_INDICES_UINT32_MSFT};
        xr::InsertExtensionStruct(meshBuffers, vertices);
        xr::InsertExtensionStruct(meshBuffers, indices);
        CHECK_XRCMD(xrGetSceneMeshBuffersMSFT(scene, &meshGetInfo, &meshBuffers));

        vertexBuffer.resize(vertices.vertexCountOutput);
        indexBuffer.resize(indices.indexCountOutput);
        vertices.vertexCapacityInput = vertices.vertexCountOutput;
        indices.indexCapacityInput = indices.indexCountOutput;
        vertices.vertices = vertexBuffer.data();
        indices.indices = indexBuffer.data();
        CHECK_XRCMD(xrGetSceneMeshBuffersMSFT(scene, &meshGetInfo, &meshBuffers));
        vertexBuffer.resize(vertices.vertexCountOutput);
        indexBuffer.resize(indices.indexCountOutput);
    }

    // Reads mesh vertices and 16-bit indices.
    inline void
    ReadMeshBuffers(XrSceneMSFT scene, uint64_t meshBufferId, std::vector<XrVector3f>& vertexBuffer, std::vector<uint16_t>& indexBuffer) {
        XrSceneMeshBuffersGetInfoMSFT meshGetInfo{XR_TYPE_SCENE_MESH_BUFFERS_GET_INFO_MSFT};
        meshGetInfo.meshBufferId = meshBufferId;

        XrSceneMeshBuffersMSFT meshBuffers{XR_TYPE_SCENE_MESH_BUFFERS_MSFT};
        XrSceneMeshVertexBufferMSFT vertices{XR_TYPE_SCENE_MESH_VERTEX_BUFFER_MSFT};
        XrSceneMeshIndicesUint16MSFT indices{XR_TYPE_SCENE_MESH_INDICES_UINT16_MSFT};
        xr::InsertExtensionStruct(meshBuffers, vertices);
        xr::InsertExtensionStruct(meshBuffers, indices);
        CHECK_XRCMD(xrGetSceneMeshBuffersMSFT(scene, &meshGetInfo, &meshBuffers));

        vertexBuffer.resize(vertices.vertexCountOutput);
        indexBuffer.resize(indices.indexCountOutput);
        vertices.vertexCapacityInput = vertices.vertexCountOutput;
        indices.indexCapacityInput = indices.indexCountOutput;
        vertices.vertices = vertexBuffer.data();
        indices.indices = indexBuffer.data();
        CHECK_XRCMD(xrGetSceneMeshBuffersMSFT(scene, &meshGetInfo, &meshBuffers));
        vertexBuffer.resize(vertices.vertexCountOutput);
        indexBuffer.resize(indices.indexCountOutput);
    }
} // namespace xr
