// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <memory.h>
#include <functional>
#include <string.h>

namespace xr {
    template <typename GUID1, typename GUID2>
    void CopyGuid(GUID1& dest, const GUID2& src) noexcept {
        static_assert(sizeof(GUID1) == sizeof(GUID2), "GUID sizes must be equal");
#ifdef _MSC_VER
        memcpy_s(&dest, sizeof(GUID1), &src, sizeof(GUID2));
#else
        memcpy(&dest, &src, sizeof(GUID2));
#endif
    }
} // namespace xr
