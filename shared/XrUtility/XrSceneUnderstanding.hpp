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
        using Kind = ::XrSceneObjectKindMSFT;
        Id id;
        Kind kind;
    };

    struct ScenePlane { // XR_SCENE_COMPONENT_TYPE_PLANE_MSFT
        using Id = TypedUuid<ScenePlane>;
        using Alignment = ::XrScenePlaneAlignmentTypeMSFT;
        using Extent = XrExtent2Df;
        ScenePlane::Id id;
        SceneObject::Id parentObjectId;
        Alignment alignment;
        Extent size;
    };

    struct SceneMesh { // XR_SCENE_COMPONENT_TYPE_OBJECT_MESH_MSFT
        using Id = TypedUuid<SceneMesh>;
        SceneMesh::Id id;
        SceneObject::Id parentObjectId;
        uint64_t meshBufferId;
    };

    // Gets the list of scene objects in the scene.
    // If filterObjectKind is not empty then only the scene objects that match any of the given kinds will be returned.
    inline std::vector<SceneObject> GetSceneObjects(XrSceneMSFT scene,
                                                    const xr::ExtensionDispatchTable& extensions,
                                                    const std::vector<SceneObject::Kind>& filterObjectKind = {}) {
        XrSceneComponentsGetInfoMSFT getInfo{XR_TYPE_SCENE_COMPONENTS_GET_INFO_MSFT};
        getInfo.componentType = XR_SCENE_COMPONENT_TYPE_OBJECT_MSFT;

        XrSceneObjectKindsFilterInfoMSFT kindsFilter{XR_TYPE_SCENE_OBJECT_KINDS_FILTER_INFO_MSFT};
        if (!filterObjectKind.empty()) {
            kindsFilter.objectKindCount = static_cast<uint32_t>(filterObjectKind.size());
            kindsFilter.objectKinds = filterObjectKind.data();
            xr::InsertExtensionStruct(getInfo, kindsFilter);
        }

        XrSceneComponentStatesMSFT sceneComponents{XR_TYPE_SCENE_COMPONENT_STATES_MSFT};
        CHECK_XRCMD(extensions.xrGetSceneComponentsMSFT(scene, &getInfo, &sceneComponents));
        const uint32_t count = sceneComponents.componentCountOutput;

        std::vector<XrSceneComponentStateMSFT> components(count);
        sceneComponents.componentCapacityInput = count;
        sceneComponents.components = components.data();

        std::vector<XrSceneObjectStateMSFT> objects(count);
        XrSceneObjectStatesMSFT sceneObjects{XR_TYPE_SCENE_OBJECT_STATES_MSFT};
        sceneObjects.sceneObjectCount = count;
        sceneObjects.sceneObjects = objects.data();
        xr::InsertExtensionStruct(sceneComponents, sceneObjects);

        CHECK_XRCMD(extensions.xrGetSceneComponentsMSFT(scene, &getInfo, &sceneComponents));

        std::vector<SceneObject> result(count);
        for (uint32_t k = 0; k < count; k++) {
            result[k].id = components[k].componentId;
            result[k].kind = objects[k].objectKind;
        }
        return result;
    }

    // Gets the list of scene planes in the scene.
    // - If parentObjectId is set then only the scene planes that are children of that scene object will be returned.
    // - If filterObjectKind is not empty then only the scene planes that match any of the given kinds will be returned.
    // - If filterAlignment is not empty then only the scene planes that match any of the given alignments will be returned.
    // - If filterObjectKind and filterAlignment are not empty then the scene plane must pass both filters.
    inline std::vector<ScenePlane> GetScenePlanes(XrSceneMSFT scene,
                                                  const xr::ExtensionDispatchTable& extensions,
                                                  std::optional<SceneObject::Id> parentObjectId = {},
                                                  const std::vector<SceneObject::Kind>& filterObjectKind = {},
                                                  const std::vector<ScenePlane::Alignment>& filterAlignment = {}) {
        XrSceneComponentsGetInfoMSFT getInfo{XR_TYPE_SCENE_COMPONENTS_GET_INFO_MSFT};
        getInfo.componentType = XR_SCENE_COMPONENT_TYPE_PLANE_MSFT;

        XrSceneComponentParentFilterInfoMSFT parentFilter{XR_TYPE_SCENE_COMPONENT_PARENT_FILTER_INFO_MSFT};
        if (parentObjectId.has_value()) {
            parentFilter.parentObjectId = static_cast<XrUuidMSFT>(parentObjectId.value());
            xr::InsertExtensionStruct(getInfo, parentFilter);
        }

        XrSceneObjectKindsFilterInfoMSFT kindsFilter{XR_TYPE_SCENE_OBJECT_KINDS_FILTER_INFO_MSFT};
        if (!filterObjectKind.empty()) {
            kindsFilter.objectKindCount = static_cast<uint32_t>(filterObjectKind.size());
            kindsFilter.objectKinds = filterObjectKind.data();
            xr::InsertExtensionStruct(getInfo, kindsFilter);
        }

        XrScenePlaneAlignmentFilterInfoMSFT alignmentFilter{XR_TYPE_SCENE_PLANE_ALIGNMENT_FILTER_INFO_MSFT};
        if (!filterAlignment.empty()) {
            alignmentFilter.alignmentCount = static_cast<uint32_t>(filterAlignment.size());
            alignmentFilter.alignments = filterAlignment.data();
            xr::InsertExtensionStruct(getInfo, alignmentFilter);
        }

        XrSceneComponentStatesMSFT sceneComponents{XR_TYPE_SCENE_COMPONENT_STATES_MSFT};
        CHECK_XRCMD(extensions.xrGetSceneComponentsMSFT(scene, &getInfo, &sceneComponents));
        const uint32_t count = sceneComponents.componentCountOutput;

        std::vector<XrSceneComponentStateMSFT> components(count);
        sceneComponents.componentCapacityInput = count;
        sceneComponents.components = components.data();

        std::vector<XrScenePlaneStateMSFT> planes(count);
        XrScenePlaneStatesMSFT scenePlanes{XR_TYPE_SCENE_PLANE_STATES_MSFT};
        scenePlanes.scenePlaneCount = count;
        scenePlanes.scenePlanes = planes.data();
        xr::InsertExtensionStruct(sceneComponents, scenePlanes);

        CHECK_XRCMD(extensions.xrGetSceneComponentsMSFT(scene, &getInfo, &sceneComponents));

        std::vector<ScenePlane> result(count);
        for (uint32_t k = 0; k < count; k++) {
            result[k].id = components[k].componentId;
            result[k].alignment = planes[k].alignment;
            result[k].size = planes[k].size;
            result[k].parentObjectId = planes[k].parentObjectId;
        }
        return result;
    }

    // Gets the list of scene meshes in the scene.
    // - If parentObjectId is set then only the scene meshes that are children of that scene object will be returned.
    // - If filterObjectKind is not empty then only the scene objects that match any of the given kinds will be returned.
    inline std::vector<SceneMesh> GetSceneMeshes(XrSceneMSFT scene,
                                                 const xr::ExtensionDispatchTable& extensions,
                                                 std::optional<SceneObject::Id> parentObjectId = {},
                                                 const std::vector<SceneObject::Kind>& filterObjectKind = {}) {
        XrSceneComponentsGetInfoMSFT getInfo{XR_TYPE_SCENE_COMPONENTS_GET_INFO_MSFT};
        getInfo.componentType = XR_SCENE_COMPONENT_TYPE_OBJECT_MESH_MSFT;

        XrSceneComponentParentFilterInfoMSFT parentFilter{XR_TYPE_SCENE_COMPONENT_PARENT_FILTER_INFO_MSFT};
        if (parentObjectId.has_value()) {
            parentFilter.parentObjectId = static_cast<XrUuidMSFT>(parentObjectId.value());
            xr::InsertExtensionStruct(getInfo, parentFilter);
        }

        XrSceneObjectKindsFilterInfoMSFT kindsFilter{XR_TYPE_SCENE_OBJECT_KINDS_FILTER_INFO_MSFT};
        if (!filterObjectKind.empty()) {
            kindsFilter.objectKindCount = static_cast<uint32_t>(filterObjectKind.size());
            kindsFilter.objectKinds = filterObjectKind.data();
            xr::InsertExtensionStruct(getInfo, kindsFilter);
        }

        XrSceneComponentStatesMSFT sceneComponents{XR_TYPE_SCENE_COMPONENT_STATES_MSFT};
        CHECK_XRCMD(extensions.xrGetSceneComponentsMSFT(scene, &getInfo, &sceneComponents));
        const uint32_t count = sceneComponents.componentCountOutput;

        std::vector<XrSceneComponentStateMSFT> components(count);
        sceneComponents.componentCapacityInput = count;
        sceneComponents.components = components.data();

        std::vector<XrSceneMeshStateMSFT> meshes(count);
        XrSceneMeshStatesMSFT sceneMeshes{XR_TYPE_SCENE_MESH_STATES_MSFT};
        sceneMeshes.sceneMeshCount = count;
        sceneMeshes.sceneMeshes = meshes.data();
        xr::InsertExtensionStruct(sceneComponents, sceneMeshes);

        CHECK_XRCMD(extensions.xrGetSceneComponentsMSFT(scene, &getInfo, &sceneComponents));

        std::vector<SceneMesh> result(count);
        for (uint32_t k = 0; k < count; k++) {
            result[k].id = components[k].componentId;
            result[k].parentObjectId = meshes[k].parentObjectId;
            result[k].meshBufferId = meshes[k].meshBufferId;
        }
        return result;
    }

    // Locate components given space and time.
    template <typename TUuid>
    void LocateObjects(XrSceneMSFT scene,
                       const xr::ExtensionDispatchTable& extensions,
                       XrSpace baseSpace,
                       XrTime time,
                       const std::vector<TUuid>& ids,
                       std::vector<XrSceneComponentLocationMSFT>& locations) {
        XrSceneComponentsLocateInfoMSFT locateInfo{XR_TYPE_SCENE_COMPONENTS_LOCATE_INFO_MSFT};
        locateInfo.baseSpace = baseSpace;
        locateInfo.time = time;
        locateInfo.idCount = static_cast<uint32_t>(ids.size());
        static_assert(sizeof(TUuid) == sizeof(XrUuidMSFT));
        locateInfo.ids = static_cast<const XrUuidMSFT*>(ids.data());

        locations.resize(ids.size());
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

        // Gets objects/planes/meshes in this scene.
        inline std::vector<SceneObject> GetObjects(const std::vector<SceneObject::Kind>& filterObjectKind = {}) const {
            return GetSceneObjects(m_scene.Get(), m_extensions, filterObjectKind);
        }

        inline std::vector<ScenePlane> GetPlanes(const std::vector<SceneObject::Kind>& filterObjectKind = {},
                                                 const std::vector<ScenePlane::Alignment>& filterAlignment = {}) const {
            return GetScenePlanes(m_scene.Get(), m_extensions, {}, filterObjectKind, filterAlignment);
        }

        inline std::vector<SceneMesh> GetMeshes(const std::vector<SceneObject::Kind>& filterObjectKind = {}) const {
            return GetSceneMeshes(m_scene.Get(), m_extensions, {}, filterObjectKind);
        }

        inline std::vector<ScenePlane> GetChildrenPlanes(SceneObject::Id parentObjectId,
                                                         const std::vector<SceneObject::Kind>& filterObjectKind = {},
                                                         const std::vector<ScenePlane::Alignment>& filterAlignment = {}) const {
            return GetScenePlanes(m_scene.Get(), m_extensions, parentObjectId, filterObjectKind, filterAlignment);
        }

        inline std::vector<SceneMesh> GetChildrenMeshes(SceneObject::Id parentObjectId,
                                                        const std::vector<SceneObject::Kind>& filterObjectKind = {}) const {
            return GetSceneMeshes(m_scene.Get(), m_extensions, parentObjectId, filterObjectKind);
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

        inline void ComputeNewScene(const SceneBounds& bounds, bool disableInferredSceneObjects = false) const {
            xr::ComputeNewScene(m_sceneObserver.Get(), m_extensions, bounds, disableInferredSceneObjects);
        }

        inline XrSceneComputeStateMSFT GetSceneComputeState() const {
            XrSceneComputeStateMSFT state{XR_SCENE_COMPUTE_STATE_NONE_MSFT};
            CHECK_XRCMD(m_extensions.xrGetSceneComputeStateMSFT(m_sceneObserver.Get(), &state));
            return state;
        }

        inline bool IsSceneComputeCompleted() const {
            return GetSceneComputeState() == XR_SCENE_COMPUTE_STATE_COMPLETED_MSFT;
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
