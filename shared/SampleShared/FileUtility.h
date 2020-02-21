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
#include <filesystem>

namespace Pbr {
    struct Model;
    struct Resources;
}

namespace sample {
    std::vector<uint8_t> ReadFileBytes(const std::filesystem::path& path);
    std::filesystem::path GetPathInAppFolder(const std::filesystem::path& filename);

    Pbr::Resources InitializePbrResources(ID3D11Device* device, bool environmentIBL = true);
    std::shared_ptr<Pbr::Model> LoadPbrModelBinary(const std::filesystem::path& filePath, const Pbr::Resources& resources);
} // namespace sample
