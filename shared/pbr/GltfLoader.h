////////////////////////////////////////////////////////////////////////////////
// Copyright (C) Microsoft Corporation.  All Rights Reserved
// Licensed under the MIT License. See License.txt in the project root for license information.
//
// Functions to load glTF 2.0 content into a renderable Pbr::Model.
//

#pragma once

#include <memory>
#include "PbrResources.h"
#include "PbrModel.h"

namespace tinygltf { class Model; }

namespace Gltf
{
    // Creates a Pbr Model from tinygltf model.
    std::shared_ptr<Pbr::Model> FromGltfObject(
        const Pbr::Resources& pbrResources,
        const tinygltf::Model& gltfModel);


    // Creates a Pbr Model from glTF 2.0 GLB file content.
    std::shared_ptr<Pbr::Model> FromGltfBinary(
        const Pbr::Resources& pbrResources,
        _In_reads_bytes_(bufferBytes) const uint8_t* buffer,
        uint32_t bufferBytes);

    template<typename Container>
    std::shared_ptr<Pbr::Model> FromGltfBinary(const Pbr::Resources& pbrResources, const Container& buffer) {
        return FromGltfBinary(pbrResources, buffer.data(), static_cast<uint32_t>(buffer.size()));
    }
}
