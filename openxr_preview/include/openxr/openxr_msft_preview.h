#pragma once

////////////////////////////////////////////////////////////////////////////////
// Copyright (C) Microsoft Corporation.  All Rights Reserved

#include "preview/xr_ext_hand_interaction.h"
#include "preview/xr_msft_scene_marker.h"

// clang-format off

// Supports XrStructTraits and ValidateStruct
#define XR_LIST_STRUCTURE_TYPES_MSFT_PREVIEW(_) \
    XR_LIST_STRUCTURE_TYPES_EXT_hand_interaction(_) \
    XR_LIST_STRUCTURE_TYPES_MSFT_scene_marker(_)

// Supports IXrExtensionConfiguration.EnabledExtensions()
#define XR_LIST_EXTENSIONS_MSFT_PREVIEW(_) \
    XR_LIST_EXTENSIONS_EXT_hand_interaction(_) \
    XR_LIST_EXTENSIONS_MSFT_scene_marker(_)

// Supports xrStructureTypeToString and IsKnownStructType
#define XR_LIST_ENUM_XrStructureType_MSFT_PREVIEW(_) \
    XR_LIST_ENUM_XrStructureType_EXT_hand_interaction(_) \
    XR_LIST_ENUM_XrStructureType_MSFT_scene_marker(_)

// Supports XrDispatchTable.h
#define XR_LIST_FUNCTIONS_MSFT_PREVIEW(_) \
    XR_LIST_FUNCTIONS_MSFT_scene_marker(_)

// clang-format on
