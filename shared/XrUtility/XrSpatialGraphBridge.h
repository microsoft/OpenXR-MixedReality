// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <array>
#include <optional>
#include "XrStruct.h"
#include "XrHandle.h"
#include "XrGuid.h"
#include "XrExtensions.h"

namespace xr {
    class SpatialGraphNodeBindingHandle : public xr::UniqueExtHandle<XrSpatialGraphNodeBindingMSFT> {};

// GUID_DEFINED is defined in guiddef.h
#ifdef GUID_DEFINED

    struct SpatialGraphNodeBindingProperties {
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

    inline const GUID& GetNodeIdAsGuid(const XrSpatialGraphNodeBindingPropertiesMSFT& properties) {
        return reinterpret_cast<GUID const&>(properties.nodeId);
    }

    inline xr::SpaceHandle CreateSpatialGraphNodeSpace(const xr::ExtensionDispatchTable& extensions,
                                                       XrSession session,
                                                       XrSpatialGraphNodeTypeMSFT nodeType,
                                                       const GUID& nodeId,
                                                       const XrPosef& pose) {
        XrSpatialGraphNodeSpaceCreateInfoMSFT spaceCreateInfo{XR_TYPE_SPATIAL_GRAPH_NODE_SPACE_CREATE_INFO_MSFT};
        spaceCreateInfo.nodeType = nodeType;
        spaceCreateInfo.pose = pose;
        SetNodeId(spaceCreateInfo, nodeId);
        xr::SpaceHandle space;
        CHECK_XRCMD(extensions.xrCreateSpatialGraphNodeSpaceMSFT(session, &spaceCreateInfo, space.Put()));
        return space;
    }

    inline xr::SpatialGraphNodeBindingHandle TryCreateSpatialGraphStaticNodeBinding(
        const xr::ExtensionDispatchTable& extensions, XrSession session, XrSpace space, XrPosef poseInSpace, XrTime time) {
        xr::SpatialGraphNodeBindingHandle nodeBinding;
        XrSpatialGraphStaticNodeBindingCreateInfoMSFT createInfo{XR_TYPE_SPATIAL_GRAPH_STATIC_NODE_BINDING_CREATE_INFO_MSFT};
        createInfo.space = space;
        createInfo.poseInSpace = poseInSpace;
        createInfo.time = time;
        CHECK_XRCMD(extensions.xrTryCreateSpatialGraphStaticNodeBindingMSFT(
            session, &createInfo, nodeBinding.Put(extensions.xrDestroySpatialGraphNodeBindingMSFT)));
        return nodeBinding;
    }

    inline SpatialGraphNodeBindingProperties GetSpatialGraphNodeBindingProperties(const xr::ExtensionDispatchTable& extensions,
                                                                                  XrSpatialGraphNodeBindingMSFT nodeBinding) {
        XrSpatialGraphNodeBindingPropertiesMSFT properties{XR_TYPE_SPATIAL_GRAPH_NODE_BINDING_PROPERTIES_MSFT};
        CHECK_XRCMD(extensions.xrGetSpatialGraphNodeBindingPropertiesMSFT(nodeBinding, nullptr, &properties));
        return {GetNodeIdAsGuid(properties), properties.poseInNodeSpace};
    }
#endif // GUID_DEFINED
} // namespace xr
