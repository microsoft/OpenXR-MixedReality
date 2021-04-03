// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

namespace xr {

    namespace Side {
        using side_t = uint32_t;

        constexpr side_t Left = 0;
        constexpr side_t Right = 1;
        constexpr side_t Count = 2;

        constexpr const char* Name[Count] = { "Left", "Right"};
        constexpr const char* UserPath[Count] = {"/user/hand/left", "/user/hand/right"};
    } // namespace Side

} // namespace xr
