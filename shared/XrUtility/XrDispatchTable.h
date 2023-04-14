// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <XrUtility/XrPlatformDependencies.h>
#include <openxr/openxr.h>
#include <openxr/openxr_platform.h>
#include <openxr/openxr_reflection.h>
#include <openxr/openxr_msft_preview.h>

#include "XrListFunctions.h"

/// <summary>
/// The xr::DispatchTable struct contains all available PFN pointers to xr functions including those in all extensions.
/// The application should call dispatchTable.Initialize() once creating a new XrInstance to initialize all function pointers.
/// The application should reset the dispatch table by `= {}` when destroying the instance handle.
///
/// #define ENABLE_GLOBAL_XR_DISPATCH_TABLE before #include "XrDispatchTable.h" will add the xrFunctions to the global namespace,
/// so that the application can simply call xr functions as if they are normal C functions.
/// Extension functions can also be used as C functions, e.g. xrCreateSpatialAnchorMSFT();
/// </summary>

namespace xr {

    struct DispatchTable {
#define XR_DISPATCH_TABLE_MEMBER(name) PFN_##name name{nullptr};
#define XR_DISPATCH_TABLE_MEMBER_VOID(name) PFN_xrVoidFunction name{nullptr};

        XR_LIST_FUNCTIONS_OPENXR_FUNCTIONS(XR_DISPATCH_TABLE_MEMBER);
        XR_LIST_FUNCTIONS_OPENXR_EXTENSIONS(XR_DISPATCH_TABLE_MEMBER, XR_DISPATCH_TABLE_MEMBER_VOID);
        XR_LIST_FUNCTIONS_MSFT_PREVIEW(XR_DISPATCH_TABLE_MEMBER);
#undef XR_DISPATCH_TABLE_DEFINE_MEMBER
#undef XR_DISPATCH_TABLE_MEMBER_VOID

        void Initialize(XrInstance instance, PFN_xrGetInstanceProcAddr getInstanceProcAddr) {
            auto getFunctionPointer = [&](const char* name) -> PFN_xrVoidFunction {
                PFN_xrVoidFunction function = nullptr;
                (void)getInstanceProcAddr(instance, name, &function);
                return function;
            };

#define XR_DISPATCH_TABLE_GET_PROC_ADDRESS(name) name = reinterpret_cast<PFN_##name>(getFunctionPointer(#name));
#define XR_DISPATCH_TABLE_SET_NO_OP(name)

            XR_LIST_FUNCTIONS_OPENXR_FUNCTIONS(XR_DISPATCH_TABLE_GET_PROC_ADDRESS);
            XR_LIST_FUNCTIONS_OPENXR_EXTENSIONS(XR_DISPATCH_TABLE_GET_PROC_ADDRESS, XR_DISPATCH_TABLE_SET_NO_OP);
            XR_LIST_FUNCTIONS_MSFT_PREVIEW(XR_DISPATCH_TABLE_GET_PROC_ADDRESS);
#undef XR_DISPATCH_TABLE_GET_PROC_ADDRESS
#undef XR_DISPATCH_TABLE_SET_NO_OP
        }
    };
} // namespace xr

// Add all xrFunctions, including extension functions, to the global namespace so they can be used as C functions.
#ifdef ENABLE_GLOBAL_XR_DISPATCH_TABLE
// clang-format off
    namespace xr {
        inline DispatchTable g_dispatchTable{};
    }

    #define XR_DISPATCH_TABLE_GLOBAL(name) inline const PFN_##name& name = xr::g_dispatchTable. name;
    #define XR_DISPATCH_TABLE_NO_OP(name)

    #if !defined(XR_NO_PROTOTYPES)
        // If openxr.h already defined function prototypes
        // avoid adding duplicated PFN aliases in global namespace
    #else
        XR_LIST_FUNCTIONS_OPENXR_FUNCTIONS(XR_DISPATCH_TABLE_GLOBAL);
    #endif

    #if !defined(XR_NO_PROTOTYPES) && defined(XR_EXTENSION_PROTOTYPES)
        // If openxr.h already defined extension function prototypes
        // avoid adding duplicated PFN aliases in global namespace
    #else
        XR_LIST_FUNCTIONS_OPENXR_EXTENSIONS(XR_DISPATCH_TABLE_GLOBAL, XR_DISPATCH_TABLE_NO_OP);
        XR_LIST_FUNCTIONS_MSFT_PREVIEW(XR_DISPATCH_TABLE_GLOBAL);
    #endif

    #undef XR_DISPATCH_TABLE_GLOBAL
    #undef XR_DISPATCH_TABLE_NO_OP
// clang-format on
#endif // ENABLE_GLOBAL_XR_DISPATCH_TABLE

