// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <array>
#include <optional>
#include "XrStruct.h"
#include "XrHandle.h"
#include "XrGuid.h"

namespace xr {
    class SpatialGraphStaticNodeBindingHandle : public xr::UniqueXrHandle<XrSpatialGraphStaticNodeBindingMSFT> {};

// GUID_DEFINED is defined in guiddef.h
#ifdef GUID_DEFINED

    struct SpatialGraphStaticNodeBindingProperties {
        GUID nodeId;
        XrPosef poseInNodeSpace;
    };

    inline void SetNodeId(XrSpatialGraphNodeSpaceCreateInfoMSFT& createInfo, const GUID& guid) {
#ifdef __STDC_LIB_EXT1__
        memcpy_s(&createInfo.nodeId, sizeof(createInfo.nodeId), &guid, sizeof(guid));
#else
        memcpy(&createInfo.nodeId, &guid, sizeof(guid));
#endif
    }

    inline const GUID& GetNodeIdAsGuid(const XrSpatialGraphStaticNodeBindingPropertiesMSFT& properties) {
        return reinterpret_cast<GUID const&>(properties.nodeId);
    }

    inline xr::SpaceHandle
    CreateSpatialGraphNodeSpace(XrSession session, XrSpatialGraphNodeTypeMSFT nodeType, const GUID& nodeId, const XrPosef& pose) {
        XrSpatialGraphNodeSpaceCreateInfoMSFT spaceCreateInfo{XR_TYPE_SPATIAL_GRAPH_NODE_SPACE_CREATE_INFO_MSFT};
        spaceCreateInfo.nodeType = nodeType;
        spaceCreateInfo.pose = pose;
        SetNodeId(spaceCreateInfo, nodeId);
        xr::SpaceHandle space;
        CHECK_XRCMD(xrCreateSpatialGraphNodeSpaceMSFT(session, &spaceCreateInfo, space.Put(xrDestroySpace)));
        return space;
    }

    inline xr::SpatialGraphStaticNodeBindingHandle
    TryCreateSpatialGraphStaticNodeBinding(XrSession session, XrSpace space, XrPosef poseInSpace, XrTime time) {
        xr::SpatialGraphStaticNodeBindingHandle nodeBinding;
        XrSpatialGraphStaticNodeBindingCreateInfoMSFT createInfo{XR_TYPE_SPATIAL_GRAPH_STATIC_NODE_BINDING_CREATE_INFO_MSFT};
        createInfo.space = space;
        createInfo.poseInSpace = poseInSpace;
        createInfo.time = time;
        CHECK_XRCMD(xrTryCreateSpatialGraphStaticNodeBindingMSFT(
            session, &createInfo, nodeBinding.Put(xrDestroySpatialGraphStaticNodeBindingMSFT)));
        return nodeBinding;
    }

    inline SpatialGraphStaticNodeBindingProperties
    GetSpatialGraphStaticNodeBindingProperties(XrSpatialGraphStaticNodeBindingMSFT nodeBinding) {
        XrSpatialGraphStaticNodeBindingPropertiesMSFT properties{XR_TYPE_SPATIAL_GRAPH_STATIC_NODE_BINDING_PROPERTIES_MSFT};
        CHECK_XRCMD(xrGetSpatialGraphStaticNodeBindingPropertiesMSFT(nodeBinding, nullptr, &properties));
        return {GetNodeIdAsGuid(properties), properties.poseInNodeSpace};
    }
#endif // GUID_DEFINED
} // namespace xr
