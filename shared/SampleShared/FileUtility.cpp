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
#include "FileUtility.h"
#include "DirectXTK/DDSTextureLoader.h"
#include <pbr/GltfLoader.h>
#include <pbr/PbrModel.h>
#include <DirectXMath.h>
#include <fstream>

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

    std::filesystem::path GetPathInAppFolder(const std::filesystem::path& filename) {
        HMODULE thisModule;
#ifdef UWP
        thisModule = nullptr;
#else
        if (!::GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, reinterpret_cast<LPCWSTR>(GetPathInAppFolder), &thisModule)) {
            throw std::runtime_error("Unable to get the module handle.");
        }
#endif

        wchar_t moduleFilename[MAX_PATH];
        ::GetModuleFileName(thisModule, moduleFilename, (DWORD)std::size(moduleFilename));
        std::filesystem::path fullPath(moduleFilename);
        return fullPath.replace_filename(filename);
    }

    Pbr::Resources InitializePbrResources(ID3D11Device* device, bool environmentIBL) {
        Pbr::Resources pbrResources(device);

        // Set up a light source (an image-based lighting environment map will also be loaded and contribute to the scene lighting).
        pbrResources.SetLight({0.0f, 0.7071067811865475f, 0.7071067811865475f}, Pbr::RGB::White);

        // Read the BRDF Lookup Table used by the PBR system into a DirectX texture.
        std::vector<byte> brdfLutFileData = ReadFileBytes(GetPathInAppFolder(L"brdf_lut.png"));
        winrt::com_ptr<ID3D11ShaderResourceView> brdLutResourceView =
            Pbr::Texture::LoadTextureImage(device, brdfLutFileData.data(), (uint32_t)brdfLutFileData.size());
        pbrResources.SetBrdfLut(brdLutResourceView.get());

        winrt::com_ptr<ID3D11ShaderResourceView> diffuseTextureView;
        winrt::com_ptr<ID3D11ShaderResourceView> specularTextureView;

        if (environmentIBL) {
            CHECK_HRCMD(DirectX::CreateDDSTextureFromFile(
                device, GetPathInAppFolder(L"Sample_DiffuseHDR.DDS").c_str(), nullptr, diffuseTextureView.put()));
            CHECK_HRCMD(DirectX::CreateDDSTextureFromFile(
                device, GetPathInAppFolder(L"Sample_SpecularHDR.DDS").c_str(), nullptr, specularTextureView.put()));
        } else {
            diffuseTextureView = Pbr::Texture::CreateFlatCubeTexture(device, Pbr::RGBA::White);
            specularTextureView = Pbr::Texture::CreateFlatCubeTexture(device, Pbr::RGBA::White);
        }

        pbrResources.SetEnvironmentMap(specularTextureView.get(), diffuseTextureView.get());

        return pbrResources;
    }

    std::shared_ptr<Pbr::Model> LoadPbrModelBinary(const std::filesystem::path& filePath, const Pbr::Resources& resources) {
        std::vector<byte> fileData = ReadFileBytes(GetPathInAppFolder(filePath));

        std::shared_ptr<Pbr::Model> pbrModel = Gltf::FromGltfBinary(resources, fileData.data(), (uint32_t)fileData.size());

        return pbrModel;
    }

} // namespace sample
