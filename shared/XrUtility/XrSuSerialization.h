//*********************************************************
//    Copyright (c) Microsoft. All rights reserved.
//
//    Apache 2.0 License
//
//    You may obtain a copy of the License at
//    http://www.apache.org/licenses/LICENSE-2.0
//
//    Unless required by applicable law or agreed to in writing, software
//    distributed under the License is distributed on an "AS IS" BASIS,
//    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
//    implied. See the License for the specific language governing
//    permissions and limitations under the License.
//
//*********************************************************
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
