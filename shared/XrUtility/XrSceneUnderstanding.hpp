// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <optional>
#include <XrUtility/XrExtensions.h>
#include <XrUtility/XrUuid.h>
#include <XrUtility/XrSceneUnderstanding.h>

// C++ wrapper on the OpenXR Scene Understanding extension.
namespace xr::su {
    struct SceneObject { // XR_SCENE_COMPONENT_TYPE_OBJECT_MSFT
        using Id = TypedUuid<SceneObject>;
        using Type = ::XrSceneObjectTypeMSFT;
        Id id;
        SceneObject::Id parentId;
        XrTime updateTime;
        Type type;
    };

    struct ScenePlane { // XR_SCENE_COMPONENT_TYPE_PLANE_MSFT
        using Id = TypedUuid<ScenePlane>;
        using Alignment = ::XrScenePlaneAlignmentTypeMSFT;
        using Extent = XrExtent2Df;
        ScenePlane::Id id;
        SceneObject::Id parentId;
        XrTime updateTime;
        Alignment alignment;
        Extent size;
        uint64_t meshBufferId;
        bool supportsIndicesUint16;
    };

    struct SceneMesh { // XR_SCENE_COMPONENT_TYPE_VISUAL_MESH_MSFT
        using Id = TypedUuid<SceneMesh>;
        SceneMesh::Id id;
        SceneObject::Id parentId;
        XrTime updateTime;
        uint64_t meshBufferId;
        bool supportsIndicesUint16;
    };

    struct SceneColliderMesh { // XR_SCENE_COMPONENT_TYPE_COLLIDER_MESH_MSFT
        using Id = TypedUuid<SceneColliderMesh>;
        SceneColliderMesh::Id id;
        SceneObject::Id parentId;
        XrTime updateTime;
        uint64_t meshBufferId;
        bool supportsIndicesUint16;
    };

    // Gets the list of scene objects in the scene.
    // If filterObjectType is not empty then only the scene objects that match any of the given types will be returned.
    inline std::vector<SceneObject> GetSceneObjects(XrSceneMSFT scene,
                                                    const xr::ExtensionDispatchTable& extensions,
                                                    const std::vector<SceneObject::Type>& filterObjectType = {}) {
        XrSceneComponentsGetInfoMSFT getInfo{XR_TYPE_SCENE_COMPONENTS_GET_INFO_MSFT};
        getInfo.componentType = XR_SCENE_COMPONENT_TYPE_OBJECT_MSFT;

        XrSceneObjectTypesFilterInfoMSFT typesFilter{XR_TYPE_SCENE_OBJECT_TYPES_FILTER_INFO_MSFT};
        if (!filterObjectType.empty()) {
            typesFilter.objectTypeCount = static_cast<uint32_t>(filterObjectType.size());
            typesFilter.objectTypes = filterObjectType.data();
            xr::InsertExtensionStruct(getInfo, typesFilter);
        }

        XrSceneComponentsMSFT sceneComponents{XR_TYPE_SCENE_COMPONENTS_MSFT};
        CHECK_XRCMD(extensions.xrGetSceneComponentsMSFT(scene, &getInfo, &sceneComponents));
        const uint32_t count = sceneComponents.componentCountOutput;

        std::vector<XrSceneComponentMSFT> components(count);
        sceneComponents.componentCapacityInput = count;
        sceneComponents.components = components.data();

        std::vector<XrSceneObjectMSFT> objects(count);
        XrSceneObjectsMSFT sceneObjects{XR_TYPE_SCENE_OBJECTS_MSFT};
        sceneObjects.sceneObjectCount = count;
        sceneObjects.sceneObjects = objects.data();
        xr::InsertExtensionStruct(sceneComponents, sceneObjects);

        CHECK_XRCMD(extensions.xrGetSceneComponentsMSFT(scene, &getInfo, &sceneComponents));

        std::vector<SceneObject> result(count);
        for (uint32_t k = 0; k < count; k++) {
            result[k].id = components[k].id;
            result[k].updateTime = components[k].updateTime;
            result[k].type = objects[k].objectType;
        }
        return result;
    }

    // Gets the list of scene planes in the scene.
    // - If parentId is set then only the scene planes that are children of that scene object will be returned.
    // - If filterObjectType is not empty then only the scene planes that match any of the given types will be returned.
    // - If filterAlignment is not empty then only the scene planes that match any of the given alignments will be returned.
    // - If filterObjectType and filterAlignment are not empty then the scene plane must pass both filters.
    inline std::vector<ScenePlane> GetScenePlanes(XrSceneMSFT scene,
                                                  const xr::ExtensionDispatchTable& extensions,
                                                  std::optional<SceneObject::Id> parentId = {},
                                                  const std::vector<SceneObject::Type>& filterObjectType = {},
                                                  const std::vector<ScenePlane::Alignment>& filterAlignment = {}) {
        XrSceneComponentsGetInfoMSFT getInfo{XR_TYPE_SCENE_COMPONENTS_GET_INFO_MSFT};
        getInfo.componentType = XR_SCENE_COMPONENT_TYPE_PLANE_MSFT;

        XrSceneComponentParentFilterInfoMSFT parentFilter{XR_TYPE_SCENE_COMPONENT_PARENT_FILTER_INFO_MSFT};
        if (parentId.has_value()) {
            parentFilter.parentId = static_cast<XrUuidMSFT>(parentId.value());
            xr::InsertExtensionStruct(getInfo, parentFilter);
        }

        XrSceneObjectTypesFilterInfoMSFT typesFilter{XR_TYPE_SCENE_OBJECT_TYPES_FILTER_INFO_MSFT};
        if (!filterObjectType.empty()) {
            typesFilter.objectTypeCount = static_cast<uint32_t>(filterObjectType.size());
            typesFilter.objectTypes = filterObjectType.data();
            xr::InsertExtensionStruct(getInfo, typesFilter);
        }

        XrScenePlaneAlignmentFilterInfoMSFT alignmentFilter{XR_TYPE_SCENE_PLANE_ALIGNMENT_FILTER_INFO_MSFT};
        if (!filterAlignment.empty()) {
            alignmentFilter.alignmentCount = static_cast<uint32_t>(filterAlignment.size());
            alignmentFilter.alignments = filterAlignment.data();
            xr::InsertExtensionStruct(getInfo, alignmentFilter);
        }

        XrSceneComponentsMSFT sceneComponents{XR_TYPE_SCENE_COMPONENTS_MSFT};
        CHECK_XRCMD(extensions.xrGetSceneComponentsMSFT(scene, &getInfo, &sceneComponents));
        const uint32_t count = sceneComponents.componentCountOutput;

        std::vector<XrSceneComponentMSFT> components(count);
        sceneComponents.componentCapacityInput = count;
        sceneComponents.components = components.data();

        std::vector<XrScenePlaneMSFT> planes(count);
        XrScenePlanesMSFT scenePlanes{XR_TYPE_SCENE_PLANES_MSFT};
        scenePlanes.scenePlaneCount = count;
        scenePlanes.scenePlanes = planes.data();
        xr::InsertExtensionStruct(sceneComponents, scenePlanes);

        CHECK_XRCMD(extensions.xrGetSceneComponentsMSFT(scene, &getInfo, &sceneComponents));

        std::vector<ScenePlane> result(count);
        for (uint32_t k = 0; k < count; k++) {
            result[k].id = components[k].id;
            result[k].parentId = components[k].parentId;
            result[k].updateTime = components[k].updateTime;
            result[k].alignment = planes[k].alignment;
            result[k].size = planes[k].size;
            result[k].meshBufferId = planes[k].meshBufferId;
            result[k].supportsIndicesUint16 = planes[k].supportsIndicesUint16;
        }
        return result;
    }

    // Gets the list of scene visual meshes in the scene.
    // - If parentId is set then only the scene meshes that are children of that scene object will be returned.
    // - If filterObjectType is not empty then only the scene objects that match any of the given types will be returned.
    inline std::vector<SceneMesh> GetSceneVisualMeshes(XrSceneMSFT scene,
                                                       const xr::ExtensionDispatchTable& extensions,
                                                       std::optional<SceneObject::Id> parentId = {},
                                                       const std::vector<SceneObject::Type>& filterObjectType = {}) {
        XrSceneComponentsGetInfoMSFT getInfo{XR_TYPE_SCENE_COMPONENTS_GET_INFO_MSFT};
        getInfo.componentType = XR_SCENE_COMPONENT_TYPE_VISUAL_MESH_MSFT;

        XrSceneComponentParentFilterInfoMSFT parentFilter{XR_TYPE_SCENE_COMPONENT_PARENT_FILTER_INFO_MSFT};
        if (parentId.has_value()) {
            parentFilter.parentId = static_cast<XrUuidMSFT>(parentId.value());
            xr::InsertExtensionStruct(getInfo, parentFilter);
        }

        XrSceneObjectTypesFilterInfoMSFT typesFilter{XR_TYPE_SCENE_OBJECT_TYPES_FILTER_INFO_MSFT};
        if (!filterObjectType.empty()) {
            typesFilter.objectTypeCount = static_cast<uint32_t>(filterObjectType.size());
            typesFilter.objectTypes = filterObjectType.data();
            xr::InsertExtensionStruct(getInfo, typesFilter);
        }

        XrSceneComponentsMSFT sceneComponents{XR_TYPE_SCENE_COMPONENTS_MSFT};
        CHECK_XRCMD(extensions.xrGetSceneComponentsMSFT(scene, &getInfo, &sceneComponents));
        const uint32_t count = sceneComponents.componentCountOutput;

        std::vector<XrSceneComponentMSFT> components(count);
        sceneComponents.componentCapacityInput = count;
        sceneComponents.components = components.data();

        std::vector<XrSceneMeshMSFT> meshes(count);
        XrSceneMeshesMSFT sceneMeshes{XR_TYPE_SCENE_MESHES_MSFT};
        sceneMeshes.sceneMeshCount = count;
        sceneMeshes.sceneMeshes = meshes.data();
        xr::InsertExtensionStruct(sceneComponents, sceneMeshes);

        CHECK_XRCMD(extensions.xrGetSceneComponentsMSFT(scene, &getInfo, &sceneComponents));

        std::vector<SceneMesh> result(count);
        for (uint32_t k = 0; k < count; k++) {
            result[k].id = components[k].id;
            result[k].parentId = components[k].parentId;
            result[k].updateTime = components[k].updateTime;
            result[k].meshBufferId = meshes[k].meshBufferId;
            result[k].supportsIndicesUint16 = meshes[k].supportsIndicesUint16;
        }
        return result;
    }

    inline std::vector<SceneColliderMesh> GetSceneColliderMeshes(XrSceneMSFT scene,
                                                                 const xr::ExtensionDispatchTable& extensions,
                                                                 std::optional<SceneObject::Id> parentId = {},
                                                                 const std::vector<SceneObject::Type>& filterObjectType = {}) {
        XrSceneComponentsGetInfoMSFT getInfo{XR_TYPE_SCENE_COMPONENTS_GET_INFO_MSFT};
        getInfo.componentType = XR_SCENE_COMPONENT_TYPE_COLLIDER_MESH_MSFT;

        XrSceneComponentParentFilterInfoMSFT parentFilter{XR_TYPE_SCENE_COMPONENT_PARENT_FILTER_INFO_MSFT};
        if (parentId.has_value()) {
            parentFilter.parentId = static_cast<XrUuidMSFT>(parentId.value());
            xr::InsertExtensionStruct(getInfo, parentFilter);
        }

        XrSceneObjectTypesFilterInfoMSFT typesFilter{XR_TYPE_SCENE_OBJECT_TYPES_FILTER_INFO_MSFT};
        if (!filterObjectType.empty()) {
            typesFilter.objectTypeCount = static_cast<uint32_t>(filterObjectType.size());
            typesFilter.objectTypes = filterObjectType.data();
            xr::InsertExtensionStruct(getInfo, typesFilter);
        }

        XrSceneComponentsMSFT sceneComponents{XR_TYPE_SCENE_COMPONENTS_MSFT};
        CHECK_XRCMD(extensions.xrGetSceneComponentsMSFT(scene, &getInfo, &sceneComponents));
        const uint32_t count = sceneComponents.componentCountOutput;

        std::vector<XrSceneComponentMSFT> components(count);
        sceneComponents.componentCapacityInput = count;
        sceneComponents.components = components.data();

        std::vector<XrSceneMeshMSFT> meshes(count);
        XrSceneMeshesMSFT sceneMeshes{XR_TYPE_SCENE_MESHES_MSFT};
        sceneMeshes.sceneMeshCount = count;
        sceneMeshes.sceneMeshes = meshes.data();
        xr::InsertExtensionStruct(sceneComponents, sceneMeshes);

        CHECK_XRCMD(extensions.xrGetSceneComponentsMSFT(scene, &getInfo, &sceneComponents));

        std::vector<SceneColliderMesh> result(count);
        for (uint32_t k = 0; k < count; k++) {
            result[k].id = components[k].id;
            result[k].parentId = components[k].parentId;
            result[k].updateTime = components[k].updateTime;
            result[k].meshBufferId = meshes[k].meshBufferId;
            result[k].supportsIndicesUint16 = meshes[k].supportsIndicesUint16;
        }
        return result;
    }

    // Locate components given space and time.
    template <typename TUuid>
    void LocateObjects(XrSceneMSFT scene,
                       const xr::ExtensionDispatchTable& extensions,
                       XrSpace baseSpace,
                       XrTime time,
                       const std::vector<TUuid>& componentIds,
                       std::vector<XrSceneComponentLocationMSFT>& locations) {
        XrSceneComponentsLocateInfoMSFT locateInfo{XR_TYPE_SCENE_COMPONENTS_LOCATE_INFO_MSFT};
        locateInfo.baseSpace = baseSpace;
        locateInfo.time = time;
        locateInfo.componentIdCount = static_cast<uint32_t>(componentIds.size());
        static_assert(sizeof(TUuid) == sizeof(XrUuidMSFT));
        locateInfo.componentIds = static_cast<const XrUuidMSFT*>(componentIds.data());

        locations.resize(componentIds.size());
        XrSceneComponentLocationsMSFT componentLocations{XR_TYPE_SCENE_COMPONENT_LOCATIONS_MSFT};
        componentLocations.locationCount = static_cast<uint32_t>(locations.size());
        componentLocations.locations = locations.data();

        CHECK_XRCMD(extensions.xrLocateSceneComponentsMSFT(scene, &locateInfo, &componentLocations));
    }

    // C++ wrapper for XrSceneMSFT
    struct Scene {
        Scene(const xr::ExtensionDispatchTable& extensions, XrSceneObserverMSFT sceneObserver)
            : m_extensions(extensions)
            , m_scene(xr::CreateScene(extensions, sceneObserver)) {
        }

        // Gets the scene objects in this scene.
        inline std::vector<SceneObject> GetObjects(const std::vector<SceneObject::Type>& filterObjectType = {}) const {
            return GetSceneObjects(m_scene.Get(), m_extensions, filterObjectType);
        }

        inline std::vector<ScenePlane> GetPlanes(const std::vector<SceneObject::Type>& filterObjectType = {},
                                                 const std::vector<ScenePlane::Alignment>& filterAlignment = {}) const {
            return GetScenePlanes(m_scene.Get(), m_extensions, {}, filterObjectType, filterAlignment);
        }

        inline std::vector<SceneMesh> GetVisualMeshes(const std::vector<SceneObject::Type>& filterObjectType = {}) const {
            return GetSceneVisualMeshes(m_scene.Get(), m_extensions, {}, filterObjectType);
        }

        inline std::vector<SceneColliderMesh> GetColliderMeshes(const std::vector<SceneObject::Type>& filterObjectType = {}) const {
            return GetSceneColliderMeshes(m_scene.Get(), m_extensions, {}, filterObjectType);
        }

        inline std::vector<ScenePlane> GetChildrenPlanes(SceneObject::Id parentId,
                                                         const std::vector<SceneObject::Type>& filterObjectType = {},
                                                         const std::vector<ScenePlane::Alignment>& filterAlignment = {}) const {
            return GetScenePlanes(m_scene.Get(), m_extensions, parentId, filterObjectType, filterAlignment);
        }

        inline std::vector<SceneMesh> GetChildrenVisualMeshes(SceneObject::Id parentId,
                                                              const std::vector<SceneObject::Type>& filterObjectType = {}) const {
            return GetSceneVisualMeshes(m_scene.Get(), m_extensions, parentId, filterObjectType);
        }

        inline std::vector<SceneColliderMesh> GetChildrenColliderMeshes(SceneObject::Id parentId,
                                                                        const std::vector<SceneObject::Type>& filterObjectType = {}) const {
            return GetSceneColliderMeshes(m_scene.Get(), m_extensions, parentId, filterObjectType);
        }

        inline XrSceneMSFT Handle() const noexcept {
            return m_scene.Get();
        }

    private:
        const xr::ExtensionDispatchTable& m_extensions;
        xr::SceneHandle m_scene;
    };

    // C++ wrapper for XrSceneObserverMSFT
    struct SceneObserver {
        SceneObserver(const xr::ExtensionDispatchTable& extensions, XrSession session)
            : m_extensions(extensions)
            , m_sceneObserver(xr::CreateSceneObserver(extensions, session)) {
        }

        inline void ComputeNewScene(const std::vector<XrSceneComputeFeatureMSFT>& requestedFeatures,
                                    const SceneBounds& bounds,
                                    XrSceneComputeConsistencyMSFT consistency = XR_SCENE_COMPUTE_CONSISTENCY_SNAPSHOT_COMPLETE_MSFT,
                                    std::optional<XrMeshComputeLodMSFT> visualMeshLevelOfDetail = {}) const {
            xr::ComputeNewScene(
                m_sceneObserver.Get(), m_extensions, requestedFeatures, bounds, consistency, visualMeshLevelOfDetail);
        }

        inline XrSceneComputeStateMSFT GetSceneComputeState() const {
            XrSceneComputeStateMSFT state{XR_SCENE_COMPUTE_STATE_NONE_MSFT};
            CHECK_XRCMD(m_extensions.xrGetSceneComputeStateMSFT(m_sceneObserver.Get(), &state));
            return state;
        }

        inline bool IsSceneComputeCompleted() const {
            const XrSceneComputeStateMSFT state = GetSceneComputeState();
            return state == XR_SCENE_COMPUTE_STATE_COMPLETED_MSFT || state == XR_SCENE_COMPUTE_STATE_COMPLETED_WITH_ERROR_MSFT;
        }

        inline std::unique_ptr<Scene> CreateScene() const {
            return std::make_unique<Scene>(m_extensions, m_sceneObserver.Get());
        }

        inline XrSceneObserverMSFT Handle() const noexcept {
            return m_sceneObserver.Get();
        }

    private:
        const xr::ExtensionDispatchTable& m_extensions;
        xr::SceneObserverHandle m_sceneObserver;
    };
} // namespace xr::su
