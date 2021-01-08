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
