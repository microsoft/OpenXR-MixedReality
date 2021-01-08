// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

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
