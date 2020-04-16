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
} // namespace Pbr

namespace sample {
    std::vector<uint8_t> ReadFileBytes(const std::filesystem::path& path);

    // Get a path in app folder, the path might not exist
    std::filesystem::path GetPathInAppFolder(const std::filesystem::path& filename);

    // Find a file in given search folders relative to the app folder.
    // Returns the path to file if exist, or throw error if file is not found.
    std::filesystem::path FindFileInAppFolder(const std::filesystem::path& filename,
                                              const std::vector<std::filesystem::path>& searchFolders = {""});

    Pbr::Resources InitializePbrResources(ID3D11Device* device, bool environmentIBL = true);
} // namespace sample
