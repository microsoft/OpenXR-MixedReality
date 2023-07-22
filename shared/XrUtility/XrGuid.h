// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <functional>
#include <string.h>

namespace xr {

// GUID_DEFINED is defined in guiddef.h
#ifdef GUID_DEFINED

    inline constexpr GUID ToGuid(const XrUuidMSFT& src) noexcept {
        GUID dest{};
        const auto& u = src.bytes;
        // Data1, Data2, and Data3 are encoded as big-endian for Variant-1
        dest.Data1 = u[0] * 0x01000000u + u[1] * 0x00010000u + u[2] * 0x00000100u + u[3];
        dest.Data2 = uint16_t(u[4] * 0x0100u + u[5]);
        dest.Data3 = uint16_t(u[6] * 0x0100u + u[7]);
        dest.Data4[0] = u[8];
        dest.Data4[1] = u[9];
        dest.Data4[2] = u[10];
        dest.Data4[3] = u[11];
        dest.Data4[4] = u[12];
        dest.Data4[5] = u[13];
        dest.Data4[6] = u[14];
        dest.Data4[7] = u[15];
        return dest;
    }

    inline constexpr XrUuidMSFT ToXrUuidMSFT(const GUID& src) noexcept {
        XrUuidMSFT dest{};
        auto& u = dest.bytes;
        // Data1, Data2, and Data3 are encoded as big-endian for Variant-1
        u[0] = uint8_t(src.Data1 >> 24);
        u[1] = uint8_t(src.Data1 >> 16);
        u[2] = uint8_t(src.Data1 >> 8);
        u[3] = uint8_t(src.Data1);

        u[4] = uint8_t(src.Data2 >> 8);
        u[5] = uint8_t(src.Data2);

        u[6] = uint8_t(src.Data3 >> 8);
        u[7] = uint8_t(src.Data3);

        u[8] = src.Data4[0];
        u[9] = src.Data4[1];
        u[10] = src.Data4[2];
        u[11] = src.Data4[3];
        u[12] = src.Data4[4];
        u[13] = src.Data4[5];
        u[14] = src.Data4[6];
        u[15] = src.Data4[7];
        return dest;
    }

#endif // GUID_DEFINED
} // namespace xr
