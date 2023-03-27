// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <optional>
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

    struct SceneMarker { // XR_SCENE_COMPONENT_TYPE_MARKER_MSFT
        using Id = TypedUuid<SceneMarker>;
        using MarkerType = ::XrSceneMarkerTypeMSFT;
        SceneMarker::Id id;
        MarkerType markerType;
        XrTime lastSeenTime;
        XrOffset2Df center;
        XrExtent2Df size;
    };

    struct SceneQRCode : public SceneMarker {
        XrSceneMarkerQRCodeSymbolTypeMSFT symbolType;
        uint8_t qrVersion;
    };

    // Gets the list of scene objects in the scene.
    // If filterObjectType is not empty then only the scene objects that match any of the given types will be returned.
    inline std::vector<SceneObject> GetSceneObjects(XrSceneMSFT scene, const std::vector<SceneObject::Type>& filterObjectType = {}) {
        XrSceneComponentsGetInfoMSFT getInfo{XR_TYPE_SCENE_COMPONENTS_GET_INFO_MSFT};
        getInfo.componentType = XR_SCENE_COMPONENT_TYPE_OBJECT_MSFT;

        XrSceneObjectTypesFilterInfoMSFT typesFilter{XR_TYPE_SCENE_OBJECT_TYPES_FILTER_INFO_MSFT};
        if (!filterObjectType.empty()) {
            typesFilter.objectTypeCount = static_cast<uint32_t>(filterObjectType.size());
            typesFilter.objectTypes = filterObjectType.data();
            xr::InsertExtensionStruct(getInfo, typesFilter);
        }

        XrSceneComponentsMSFT sceneComponents{XR_TYPE_SCENE_COMPONENTS_MSFT};
        CHECK_XRCMD(xrGetSceneComponentsMSFT(scene, &getInfo, &sceneComponents));
        const uint32_t count = sceneComponents.componentCountOutput;

        std::vector<XrSceneComponentMSFT> components(count);
        sceneComponents.componentCapacityInput = count;
        sceneComponents.components = components.data();

        std::vector<XrSceneObjectMSFT> objects(count);
        XrSceneObjectsMSFT sceneObjects{XR_TYPE_SCENE_OBJECTS_MSFT};
        sceneObjects.sceneObjectCount = count;
        sceneObjects.sceneObjects = objects.data();
        xr::InsertExtensionStruct(sceneComponents, sceneObjects);

        CHECK_XRCMD(xrGetSceneComponentsMSFT(scene, &getInfo, &sceneComponents));

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
        CHECK_XRCMD(xrGetSceneComponentsMSFT(scene, &getInfo, &sceneComponents));
        const uint32_t count = sceneComponents.componentCountOutput;

        std::vector<XrSceneComponentMSFT> components(count);
        sceneComponents.componentCapacityInput = count;
        sceneComponents.components = components.data();

        std::vector<XrScenePlaneMSFT> planes(count);
        XrScenePlanesMSFT scenePlanes{XR_TYPE_SCENE_PLANES_MSFT};
        scenePlanes.scenePlaneCount = count;
        scenePlanes.scenePlanes = planes.data();
        xr::InsertExtensionStruct(sceneComponents, scenePlanes);

        CHECK_XRCMD(xrGetSceneComponentsMSFT(scene, &getInfo, &sceneComponents));

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
        CHECK_XRCMD(xrGetSceneComponentsMSFT(scene, &getInfo, &sceneComponents));
        const uint32_t count = sceneComponents.componentCountOutput;

        std::vector<XrSceneComponentMSFT> components(count);
        sceneComponents.componentCapacityInput = count;
        sceneComponents.components = components.data();

        std::vector<XrSceneMeshMSFT> meshes(count);
        XrSceneMeshesMSFT sceneMeshes{XR_TYPE_SCENE_MESHES_MSFT};
        sceneMeshes.sceneMeshCount = count;
        sceneMeshes.sceneMeshes = meshes.data();
        xr::InsertExtensionStruct(sceneComponents, sceneMeshes);

        CHECK_XRCMD(xrGetSceneComponentsMSFT(scene, &getInfo, &sceneComponents));

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
        CHECK_XRCMD(xrGetSceneComponentsMSFT(scene, &getInfo, &sceneComponents));
        const uint32_t count = sceneComponents.componentCountOutput;

        std::vector<XrSceneComponentMSFT> components(count);
        sceneComponents.componentCapacityInput = count;
        sceneComponents.components = components.data();

        std::vector<XrSceneMeshMSFT> meshes(count);
        XrSceneMeshesMSFT sceneMeshes{XR_TYPE_SCENE_MESHES_MSFT};
        sceneMeshes.sceneMeshCount = count;
        sceneMeshes.sceneMeshes = meshes.data();
        xr::InsertExtensionStruct(sceneComponents, sceneMeshes);

        CHECK_XRCMD(xrGetSceneComponentsMSFT(scene, &getInfo, &sceneComponents));

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

    template<typename T>
    class GetMarkerExtension {
    public:
        GetMarkerExtension(int count) {}

        template <typename XrStruct>
        void Insert(XrStruct& xrStruct) {}

        void CopyTo(T*, int k){}
    };

    template <>
    class GetMarkerExtension<SceneQRCode> {
        std::vector<XrSceneMarkerQRCodeMSFT> m_qrCodes;
        XrSceneMarkerQRCodesMSFT m_sceneQRCodes{XR_TYPE_SCENE_MARKER_QR_CODES_MSFT};

    public:
        GetMarkerExtension(int count)
            : m_qrCodes(count){
            m_sceneQRCodes.qrCodeCount = count;
            m_sceneQRCodes.qrCodes = m_qrCodes.data();
        }

        template <typename XrStruct>
        void Insert(XrStruct& xrStruct) {
            xr::InsertExtensionStruct(xrStruct, m_sceneQRCodes);
        }

        void CopyTo(SceneQRCode* target, int k) {
            target->qrVersion= m_qrCodes[k].qrVersion;
            target->symbolType = m_qrCodes[k].symbolType;
        }
    };

    template <typename T>
    inline std::vector<T> GetSceneMarkers(XrSceneMSFT scene) {
        XrSceneComponentsGetInfoMSFT getInfo{XR_TYPE_SCENE_COMPONENTS_GET_INFO_MSFT};
        getInfo.componentType = XR_SCENE_COMPONENT_TYPE_MARKER_MSFT;

        XrSceneMarkerTypeFilterMSFT typesFilter{XR_TYPE_SCENE_MARKER_TYPE_FILTER_MSFT};
        XrSceneMarkerTypeMSFT markerTypes[1] = {XrSceneMarkerTypeMSFT::XR_SCENE_MARKER_TYPE_QR_CODE_MSFT };
        typesFilter.markerTypeCount = static_cast<uint32_t>(std::size(markerTypes));
        typesFilter.markerTypes = markerTypes;
        xr::InsertExtensionStruct(getInfo, typesFilter);
        XrSceneComponentsMSFT sceneComponents{XR_TYPE_SCENE_COMPONENTS_MSFT};

        // Get the number of markers
        CHECK_XRCMD(xrGetSceneComponentsMSFT(scene, &getInfo, &sceneComponents));
        const uint32_t count = sceneComponents.componentCountOutput;

        std::vector<XrSceneComponentMSFT> components(count);
        sceneComponents.componentCapacityInput = count;
        sceneComponents.components = components.data();

        std::vector<XrSceneMarkerMSFT> markers(count);
        XrSceneMarkersMSFT sceneMarkers{XR_TYPE_SCENE_MARKERS_MSFT};
        sceneMarkers.sceneMarkerCount = count;
        sceneMarkers.sceneMarkers = markers.data();
        xr::InsertExtensionStruct(sceneComponents, sceneMarkers);

        GetMarkerExtension<T> extension(count);
        extension.Insert(sceneComponents);

        CHECK_XRCMD(xrGetSceneComponentsMSFT(scene, &getInfo, &sceneComponents));

        std::vector<T> result(count);
        for (uint32_t k = 0; k < count; k++) {
            auto& m = result[k];
            m.id = components[k].id;
            m.markerType = markers[k].markerType;
            m.lastSeenTime = markers[k].lastSeenTime;
            m.center = markers[k].center;
            m.size = markers[k].size;
            extension.CopyTo(&m, k);
        }
        return result;
    }

    inline void GetSceneMarkerRawData(XrSceneMSFT scene, const SceneMarker::Id& markerId, std::vector<uint8_t>& data) {
        uint32_t dataSize;
        CHECK_XRCMD(xrGetSceneMarkerRawDataMSFT(scene, (XrUuidMSFT*)&markerId, 0, &dataSize, nullptr));
        data.resize(dataSize);
        CHECK_XRCMD(xrGetSceneMarkerRawDataMSFT(scene, (XrUuidMSFT*)&markerId, dataSize, &dataSize, data.data()));
    }

    inline std::string GetSceneMarkerDecodedString(XrSceneMSFT scene, const SceneMarker::Id& markerId) {
        uint32_t characterCount;
        CHECK_XRCMD(xrGetSceneMarkerDecodedStringMSFT(scene, (XrUuidMSFT*)&markerId, 0, &characterCount, nullptr));
        std::string ret;
        ret.resize(characterCount);
        CHECK_XRCMD(xrGetSceneMarkerDecodedStringMSFT(scene, (XrUuidMSFT*)&markerId, characterCount, &characterCount, ret.data()));
        // characterCount includes the null terminator needed to fill the buffer but we need to trim it
        ret.resize(characterCount - 1);
        return ret;
    }

    // Locate components given space and time.
    template <typename TUuid>
    void LocateObjects(XrSceneMSFT scene,
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

        CHECK_XRCMD(xrLocateSceneComponentsMSFT(scene, &locateInfo, &componentLocations));
    }

    // C++ wrapper for XrSceneMSFT
    struct Scene {
        Scene(XrSceneObserverMSFT sceneObserver)
            : m_scene(xr::CreateScene(sceneObserver)) {
        }

        // Gets the scene objects in this scene.
        inline std::vector<SceneObject> GetObjects(const std::vector<SceneObject::Type>& filterObjectType = {}) const {
            return GetSceneObjects(m_scene.Get(), filterObjectType);
        }

        inline std::vector<ScenePlane> GetPlanes(const std::vector<SceneObject::Type>& filterObjectType = {},
                                                 const std::vector<ScenePlane::Alignment>& filterAlignment = {}) const {
            return GetScenePlanes(m_scene.Get(), {}, filterObjectType, filterAlignment);
        }

        inline std::vector<SceneMesh> GetVisualMeshes(const std::vector<SceneObject::Type>& filterObjectType = {}) const {
            return GetSceneVisualMeshes(m_scene.Get(), {}, filterObjectType);
        }

        inline std::vector<SceneColliderMesh> GetColliderMeshes(const std::vector<SceneObject::Type>& filterObjectType = {}) const {
            return GetSceneColliderMeshes(m_scene.Get(), {}, filterObjectType);
        }

        inline std::vector<ScenePlane> GetChildrenPlanes(SceneObject::Id parentId,
                                                         const std::vector<SceneObject::Type>& filterObjectType = {},
                                                         const std::vector<ScenePlane::Alignment>& filterAlignment = {}) const {
            return GetScenePlanes(m_scene.Get(), parentId, filterObjectType, filterAlignment);
        }

        inline std::vector<SceneMesh> GetChildrenVisualMeshes(SceneObject::Id parentId,
                                                              const std::vector<SceneObject::Type>& filterObjectType = {}) const {
            return GetSceneVisualMeshes(m_scene.Get(), parentId, filterObjectType);
        }

        inline std::vector<SceneColliderMesh> GetChildrenColliderMeshes(SceneObject::Id parentId,
                                                                        const std::vector<SceneObject::Type>& filterObjectType = {}) const {
            return GetSceneColliderMeshes(m_scene.Get(), parentId, filterObjectType);
        }

        template <typename T>
        inline std::vector<T> GetMarkers() const {
            return GetSceneMarkers<T>(m_scene.Get());
        }

        inline std::string GetMarkerDecodedString(const SceneMarker::Id& markerId) {
            return GetSceneMarkerDecodedString(m_scene.Get(), markerId);
        }

        inline void GetMarkerRawData(const SceneMarker::Id& markerId, std::vector<uint8_t>& data) {
            GetSceneMarkerRawData(m_scene.Get(), markerId, data);
        }

        inline XrSceneMSFT Handle() const noexcept {
            return m_scene.Get();
        }

    private:
        xr::SceneHandle m_scene;
    };

    // C++ wrapper for XrSceneObserverMSFT
    struct SceneObserver {
        SceneObserver(XrSession session)
            : m_sceneObserver(xr::CreateSceneObserver(session)) {
        }

        inline void ComputeNewScene(const std::vector<XrSceneComputeFeatureMSFT>& requestedFeatures,
                                    const SceneBounds& bounds,
                                    XrSceneComputeConsistencyMSFT consistency = XR_SCENE_COMPUTE_CONSISTENCY_SNAPSHOT_COMPLETE_MSFT,
                                    std::optional<XrMeshComputeLodMSFT> visualMeshLevelOfDetail = {}) const {
            xr::ComputeNewScene(m_sceneObserver.Get(), requestedFeatures, bounds, consistency, visualMeshLevelOfDetail);
        }

        inline XrSceneComputeStateMSFT GetSceneComputeState() const {
            XrSceneComputeStateMSFT state{XR_SCENE_COMPUTE_STATE_NONE_MSFT};
            CHECK_XRCMD(xrGetSceneComputeStateMSFT(m_sceneObserver.Get(), &state));
            return state;
        }

        inline bool IsSceneComputeCompleted() const {
            const XrSceneComputeStateMSFT state = GetSceneComputeState();
            return state == XR_SCENE_COMPUTE_STATE_COMPLETED_MSFT || state == XR_SCENE_COMPUTE_STATE_COMPLETED_WITH_ERROR_MSFT;
        }

        inline std::unique_ptr<Scene> CreateScene() const {
            return std::make_unique<Scene>(m_sceneObserver.Get());
        }

        inline XrSceneObserverMSFT Handle() const noexcept {
            return m_sceneObserver.Get();
        }

    private:
        xr::SceneObserverHandle m_sceneObserver;
    };
} // namespace xr::su
