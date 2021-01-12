// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "XrSceneUnderstanding.h"
#include <ostream>

namespace xr {
    inline void ReadSerializedScene(const xr::ExtensionDispatchTable& extensions, XrSceneMSFT scene, std::basic_ostream<uint8_t>& stream) {
        constexpr uint32_t BufferSize = 8192;
        std::array<uint8_t, BufferSize> buffer;
        uint32_t readOutput = 0;
        do {
            // xrGetSceneSerializedDataMSFT does not use 2-call idiom. It behaves more like fread where readOutput will output
            // how many bytes were read. The function should be called until readOutput outputs zero.
            CHECK_XRCMD(extensions.xrGetSceneSerializedDataMSFT(scene, BufferSize, &readOutput, buffer.data()));
            stream.write(buffer.data(), readOutput);
        } while (readOutput > 0);
    }
} // namespace xr
