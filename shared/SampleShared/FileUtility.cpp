// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "pch.h"
#include "FileUtility.h"
#include "DirectXTK/DDSTextureLoader.h"
#include <pbr/GltfLoader.h>
#include <pbr/PbrModel.h>
#include <fstream>
#include <XrUtility/XrString.h>
#include "Trace.h"

#define FMT_HEADER_ONLY
#include <fmt/format.h>

using namespace DirectX;

namespace sample {
    std::vector<uint8_t> ReadFileBytes(const std::filesystem::path& path) {
        bool fileExists = false;
        try {
            std::ifstream file;
            file.exceptions(std::ios::failbit | std::ios::badbit);
            file.open(path, std::ios::binary | std::ios::ate);
            fileExists = true;
            // If tellg fails then it will throw an exception instead of returning -1.
            std::vector<uint8_t> data(static_cast<size_t>(file.tellg()));
            file.seekg(0, std::ios::beg);
            file.read(reinterpret_cast<char*>(data.data()), data.size());
            return data;
        } catch (const std::ios::failure&) {
            // The exception only knows that the failbit was set so it doesn't contain anything useful.
            throw std::runtime_error(fmt::format("Failed to {} file: {}", fileExists ? "read" : "open", path.string()));
        }
    }

    std::filesystem::path GetAppFolder() {
        HMODULE thisModule;
#ifdef UWP
        thisModule = nullptr;
#else
        if (!::GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, reinterpret_cast<LPCWSTR>(GetAppFolder), &thisModule)) {
            throw std::runtime_error("Unable to get the module handle.");
        }
#endif

        wchar_t moduleFilename[MAX_PATH];
        ::GetModuleFileName(thisModule, moduleFilename, (DWORD)std::size(moduleFilename));
        std::filesystem::path fullPath(moduleFilename);
        return fullPath.remove_filename();
    }

    std::filesystem::path GetPathInAppFolder(const std::filesystem::path& filename) {
        return GetAppFolder() / filename;
    }

    std::filesystem::path FindFileInAppFolder(const std::filesystem::path& filename,
                                              const std::vector<std::filesystem::path>& searchFolders) {
        auto appFolder = GetAppFolder();
        for (auto folder : searchFolders) {
            auto path = appFolder / folder / filename;
            if (std::filesystem::exists(path)) {
                return path;
            }
        }

        sample::Trace(fmt::format("File \"{}\" is not found in app folder \"{}\" and search folders{}",
                                    xr::wide_to_utf8(filename.c_str()),
                                    xr::wide_to_utf8(appFolder.c_str()),
                                    [&searchFolders]() -> std::string {
                                        fmt::memory_buffer buffer;
                                        for (auto& folder : searchFolders) {
                                            fmt::format_to(buffer, " \"{}\"", xr::wide_to_utf8(folder.c_str()));
                                        }
                                        return buffer.data();
                                    }())
                            .c_str());

        assert(false && "The file should be embeded in app folder in debug build.");
        return "";
    }

    Pbr::Resources InitializePbrResources(ID3D11Device* device, bool environmentIBL) {
        Pbr::Resources pbrResources(device);

        // Set up a light source (an image-based lighting environment map will also be loaded and contribute to the scene lighting).
        pbrResources.SetLight({0.0f, 0.7071067811865475f, 0.7071067811865475f}, Pbr::RGB::White);

        // Read the BRDF Lookup Table used by the PBR system into a DirectX texture.
        std::vector<byte> brdfLutFileData = ReadFileBytes(FindFileInAppFolder(L"brdf_lut.png", {"", L"Pbr_uwp"}));
        winrt::com_ptr<ID3D11ShaderResourceView> brdLutResourceView =
            Pbr::Texture::LoadTextureImage(device, brdfLutFileData.data(), (uint32_t)brdfLutFileData.size());
        pbrResources.SetBrdfLut(brdLutResourceView.get());

        winrt::com_ptr<ID3D11ShaderResourceView> diffuseTextureView;
        winrt::com_ptr<ID3D11ShaderResourceView> specularTextureView;

        if (environmentIBL) {
            CHECK_HRCMD(DirectX::CreateDDSTextureFromFile(device,
                                                          FindFileInAppFolder(L"Sample_DiffuseHDR.DDS", {"", "SampleShared_uwp"}).c_str(),
                                                          nullptr,
                                                          diffuseTextureView.put()));
            CHECK_HRCMD(DirectX::CreateDDSTextureFromFile(device,
                                                          FindFileInAppFolder(L"Sample_SpecularHDR.DDS", {"", "SampleShared_uwp"}).c_str(),
                                                          nullptr,
                                                          specularTextureView.put()));
        } else {
            diffuseTextureView = Pbr::Texture::CreateFlatCubeTexture(device, Pbr::RGBA::White);
            specularTextureView = Pbr::Texture::CreateFlatCubeTexture(device, Pbr::RGBA::White);
        }

        pbrResources.SetEnvironmentMap(specularTextureView.get(), diffuseTextureView.get());

        return pbrResources;
    }

} // namespace sample
