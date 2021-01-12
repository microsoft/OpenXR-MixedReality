// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "pch.h"
#include "TextureUtility.h"

namespace sample {
    std::vector<uint32_t> CreateTileTextureBytes(size_t sideLength) {
        constexpr uint32_t White = 0xFFFFFFFF;
        constexpr uint32_t Black = 0x000000FF;
        std::vector<uint32_t> rgba(sideLength * sideLength, White);
        if (sideLength == 0) {
            return rgba;
        }
        const size_t Last = sideLength - 1;
        // make the border black
        for (size_t col = 0; col < sideLength; ++col) {
            rgba[col] = Black;
            rgba[Last + col] = Black;
        }
        for (size_t row = 0; row < sideLength * sideLength; row += sideLength) {
            rgba[row] = Black;
            rgba[row + Last] = Black;
        }
        return rgba;
    }
} // namespace sample
