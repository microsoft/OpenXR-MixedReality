////////////////////////////////////////////////////////////////////////////////
// Copyright (C) Microsoft Corporation.  All Rights Reserved

#ifndef XR_MSFT_SCENE_MARKER_H_
#define XR_MSFT_SCENE_MARKER_H_ 1

#ifndef DEFINE_ENUM_VALUE
#ifdef __cplusplus
#define DEFINE_ENUM_VALUE(Type, Name, Value) constexpr Type Name = static_cast<Type>(Value)
#else
#define DEFINE_ENUM_VALUE(Type, Name, Value) Type Name = (Type)(Value)
#endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

/// This extension enables marker tracking inputs through scene understanding extension.
#define XR_LIST_EXTENSIONS_MSFT_scene_marker(_) _(XR_MSFTX_scene_marker, 148)

#define XR_MSFTX_scene_marker 1
#define XR_MSFTX_scene_marker_SPEC_VERSION 1
#define XR_MSFTX_SCENE_MARKER_EXTENSION_NAME "XR_MSFTX_scene_marker"
/// The application must: also enable the `<<XR_MSFT_scene_understanding>>` extension in order to use this extension.
/// The runtime will call RequestAccessAsync() for the first flink:xrComputeNewSceneMSFT function call.
/// The application can use flink:xrLocateSceneComponentsMSFT to locate a QR code.
/// This extension include QR code types that HL2 supports today, and easy to extend in future:
/// https://docs.microsoft.com/en-us/windows/mixed-reality/develop/advanced-concepts/qr-code-tracking-overview#supported-qr-code-versions
/// The convention of each marker's location is similar to XrScenePlane.
///   - The Position of the marker is most stable / accurate point of the marker from a detection point of view;
///     e.g. for a QR Code, it's one of the marker corners.
///   - The XY axis is within the plane of the marker
///   - The +Z direction is pointing to the normal direction of the front face of the marker.
///   - The XrSceneMarkerMSFT.center are the coordinates of the marker's center in the XY plane related to the marker's position.
///   - The XrSceneMarkerMSFT.size has width/height corresponding to the XY direction.
/// The markerId returned in XrSceneComponent.id is used to identify each marker in related functions.
///     This markerId has type of XrUuidMSFT.
/// The application should do
///     1. Call xrCreateSceneObserverMSFT
///     2. Call xrEnumerateSceneComputeFeaturesMSFT to check if observer supports XR_SCENE_COMPUTE_FEATURE_MARKER_MSFT
///     3. If supported, call xrComputeNewSceneMSFT including XR_SCENE_COMPUTE_FEATURE_MARKER_MSFT
///     4. Poll xrGetSceneComputeStateMSFT, waiting for compute complete
///     5. Call xrCreateSceneMSFT to get compute result
///     6. Call xrGetSceneComponentsMSFT with XR_SCENE_COMPONENT_TYPE_MARKER_MSFT to query for markers.
///         6.1 Chain XrSceneMarkersMSFT to the next of XrSceneComponents to receive marker type, and lastSeenTime
///         6.2 Alternative, chain XrSceneMarkerTypeFilter to the next of XrSceneComponentsGetInfoMSFT when calling xrGetSceneComponentsMSFT
///         to filter down the result to only specific marker type.
///             If the XrSceneMarkerQrCodes are in next chain of XrSceneMarker on the output XrSceneComponents
///             when calling xrGetSceneComponentsMSFT, get additional marker type specifc info. When marker filter is specified, the output
///             struct must have matching struct.  E.g. if marker filter is asking for other types of markers than QR Codes but the
///             output structs have XrSceneMarkersQRCodesMSFT struct, the function returns XR_ERROR_VALIDATION_FAILURE.
///             Note: the filters on xrGetSceneComponents works with "and" logic when multiple filters are provided.
///             Therefore, if two marker types filter are specified with 2 different types, the result list will be empty.
///     7. Call xrDecodeMarkerStringMSFT to get the string encoded in a given markerId using the two call idions.
///     8. Optionally, call xrGetSceneMarkerRawDataMSFT to get raw data of given markerId as byte arrays using two call idioms.
///     9. Call xrLocateSceneComponentsMSFT to locate each marker using the markerId.

DEFINE_ENUM_VALUE(XrSceneComputeFeatureMSFT, XR_SCENE_COMPUTE_FEATURE_MARKER_MSFT, 1000147000);
DEFINE_ENUM_VALUE(XrSceneComponentTypeMSFT, XR_SCENE_COMPONENT_TYPE_MARKER_MSFT, 1000147000);
DEFINE_ENUM_VALUE(XrStructureType, XR_TYPE_SCENE_MARKERS_MSFT, 1000147000);
DEFINE_ENUM_VALUE(XrStructureType, XR_TYPE_SCENE_MARKER_TYPE_FILTER_MSFT, 1000147001);
DEFINE_ENUM_VALUE(XrStructureType, XR_TYPE_SCENE_MARKER_QR_CODES_MSFT, 1000147002);
DEFINE_ENUM_VALUE(XrResult, XR_SCENE_MARKER_DATA_NO_STRING_MSFT, 1000147000);

#undef DEFINE_ENUM_VALUE

#define XR_LIST_ENUM_XrStructureType_MSFT_scene_marker(_) \
    _(XR_TYPE_SCENE_MARKERS_MSFT, 1000147000)                     \
    _(XR_TYPE_SCENE_MARKER_TYPE_FILTER_MSFT, 1000147001)          \
    _(XR_TYPE_SCENE_MARKER_QR_CODES_MSFT, 1000147002)

#define XR_LIST_STRUCTURE_TYPES_MSFT_scene_marker(_)              \
    _(XrSceneMarkersMSFT, XR_TYPE_SCENE_MARKERS_MSFT)                     \
    _(XrSceneMarkerTypeFilterMSFT, XR_TYPE_SCENE_MARKER_TYPE_FILTER_MSFT) \
    _(XrSceneMarkerQRCodesMSFT, XR_TYPE_SCENE_MARKER_QR_CODES_MSFT)

typedef enum XrSceneMarkerTypeMSFT {
    XR_SCENE_MARKER_TYPE_QR_CODE_MSFT = 1,
    XR_SCENE_MARKER_TYPE_MAX_ENUM = 0x7FFFFFFF
} XrSceneMarkerTypeMSFT;

typedef enum XrSceneMarkerQRCodeSymbolTypeMSFT {
    XR_SCENE_MARKER_QR_CODE_SYMBOL_TYPE_QR_CODE_MSFT = 1,
    XR_SCENE_MARKER_QR_CODE_SYMBOL_TYPE_MICRO_QR_CODE_MSFT = 2,
    XR_SCENE_MARKER_QR_CODE_SYMBOL_TYPE_MAX_ENUM = 0x7FFFFFFF
} XrSceneMarkerQRCodeSymbolTypeMSFT;


/// Return from xrGetSceneComponents when using XR_SCENE_COMPONENT_TYPE_MARKER_MSFT
/// The markerId is returned from corresponding XrSceneComponent.Id.
/// Typically, corresponding XrSceneComponent.parentId is uuid_null
typedef struct XrSceneMarkerMSFT {
    XrSceneMarkerTypeMSFT markerType; // Use marker type to query for xrGetSceneMarkerProperties function for additional typed properties.
    XrTime lastSeenTime;              // lastSeenTime is earlier or equal to the XrSceneComponent.updateTime.
    XrOffset2Df center;               // The location of the center of the axis-aligned bounding box of marker in the XY plane of the marker's space.
    XrExtent2Df size;                 // The width and height of the axis-aligned bounding box of marker in the XY plane of the marker's space.
} XrSceneMarkerMSFT;

/// Return a list of of XrSceneMarkerMSFT from xrGetSceneComponents when using XR_SCENE_COMPONENT_TYPE_MARKER_MSFT
/// Chained to the next pointer of XrSceneComponentsMSFT and must keep the same array size.
typedef struct XrSceneMarkersMSFT {
    XrStructureType type;
    void* XR_MAY_ALIAS next;
    uint32_t sceneMarkerCount;       // must: be the same as corresponding XrSceneComponentsMSFT.componentCountInput
    XrSceneMarkerMSFT* sceneMarkers; // caller allocated buffer to receive the array of scene markers
} XrSceneMarkersMSFT;

/// Chain to next of XrSceneComponentsGetInfo to filter the xrGetSceneComponents to return only specific marker types.
typedef struct XrSceneMarkerTypeFilterMSFT {
    XrStructureType type;
    const void* XR_MAY_ALIAS next;
    uint32_t markerTypeCount;     // The number of markerTypes in the filter.
    XrSceneMarkerTypeMSFT* markerTypes; // Marker types to be returned.
} XrSceneMarkerTypeFilterMSFT;

/// Returns the QRCode specific data, combine it with corresponding XrSceneMarker and XrSceneComponent.
/// Data and version are immutable for the lifetime of a markerId
typedef struct XrSceneMarkerQRCodeMSFT {
    XrSceneMarkerQRCodeSymbolTypeMSFT symbolType;
    uint8_t qrVersion; // runtime must: return a valid qr version according to ISO_IEC_18004.2015
                       // 1 to 40 for QR Versions 1 to 40
                       // 1 to 4 for Micro QR versions M1 to M4
} XrSceneMarkerQRCodeMSFT;

/// XrSceneMarkersQRCodesMSFT extends XrSceneComponentsMSFT
typedef struct XrSceneMarkerQRCodesMSFT {
    XrStructureType type;
    void* XR_MAY_ALIAS next;
    uint32_t qrCodeCount;              // must: be the same as corresponding XrSceneComponentsMSFT.componentCountInput
    XrSceneMarkerQRCodeMSFT* qrCodes; // caller allocated buffer to receive the returned XrSceneMarkerQRCodeMSFT
} XrSceneMarkerQRCodesMSFT;

/// xrGetSceneMarkerRawDataMSFT gets the raw data of a given markerId as a byte array, using two call idioms.
/// This function works for any marker type, and input markerIds can have different marker types.
/// Returns XR_ERROR_SCENE_COMPONENT_ID_INVALID_MSFT if markerId not a scene component id
/// Returns XR_ERROR_SCENE_COMPONENT_TYPE_MISMATCH_MSFT if markerId is not a marker id 
typedef XrResult(XRAPI_PTR* PFN_xrGetSceneMarkerRawDataMSFT)(
    XrSceneMSFT scene,          // A scene snapshot the result of a scene compute
    const XrUuidMSFT* markerId, // A valid uuid previously obtained from XrSceneComponent.Id
    uint32_t dataCapacityInput,
    uint32_t* dataCountOutput,
    uint8_t* data);

/// xrDecodeMarkerStringMSFT gets string content of a given markerId as an UTF8 string, using two call idioms.
/// Returns XR_ERROR_SCENE_COMPONENT_ID_INVALID_MSFT if markerId is not a scene component id
/// Returns XR_ERROR_SCENE_COMPONENT_TYPE_MISMATCH_MSFT if markerId is a scene component id but does not identify a marker
/// Returns XR_SCENE_MARKER_DATA_NO_STRING_MSFT if marker's data do not have a string representation
typedef XrResult(XRAPI_PTR* PFN_xrGetSceneMarkerDecodedStringMSFT)(
    XrSceneMSFT scene,          // A scene snapshot the result of a scene compute
    const XrUuidMSFT* markerId, // A valid uuid previously obtained from XrSceneComponent.Id
    uint32_t stringCapacityInput, // Capacity of the string in char including the null terminator
    uint32_t* stringCountOutput,  // Length of the marker string in char including the null terminator
    char* string);

#ifndef XR_NO_PROTOTYPES
#ifdef XR_EXTENSION_PROTOTYPES

XRAPI_ATTR XrResult XRAPI_CALL xrGetSceneMarkerRawDataMSFT(
    XrSceneMSFT scene, const XrUuidMSFT* markerId, uint32_t dataCapacityInput, uint32_t* dataCountOutput, uint8_t* data);

XRAPI_ATTR XrResult XRAPI_CALL xrGetSceneMarkerDecodedStringMSFT(
    XrSceneMSFT scene, const XrUuidMSFT* markerId, uint32_t stringCapacityInput, uint32_t* stringCountOutput, char* string);

#endif /* XR_EXTENSION_PROTOTYPES */
#endif /* !XR_NO_PROTOTYPES */

#define XR_LIST_FUNCTIONS_MSFT_scene_marker(_) \
    _(xrGetSceneMarkerRawDataMSFT) \
    _(xrGetSceneMarkerDecodedStringMSFT)

#define XR_LIST_ENUM_XrSceneMarkerTypeMSFT(_) \
    _(XR_SCENE_MARKER_TYPE_QR_CODE_MSFT, 1) \
    _(XR_SCENE_MARKER_TYPE_MAX_ENUM, 0x7FFFFFFF)

#define XR_LIST_ENUM_XrSceneMarkerQRCodeSymbolTypeMSFT(_) \
    _(XR_SCENE_MARKER_QR_CODE_SYMBOL_TYPE_QR_CODE_MSFT, 1)    \
    _(XR_SCENE_MARKER_QR_CODE_SYMBOL_TYPE_MICRO_QR_CODE_MSFT, 1) \
    _(XR_SCENE_MARKER_QR_CODE_SYMBOL_TYPE_MAX_ENUM, 0x7FFFFFFF)

#ifdef __cplusplus
}
#endif
#endif // XR_MSFT_SCENE_MARKER_H_
