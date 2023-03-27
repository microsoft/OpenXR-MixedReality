// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <future>
#include <pbr/PbrModel.h>
#include <pbr/PbrMaterial.h>
#include "Scene.h"
#include "Context.h"

namespace engine {

    class PbrModelObject : public Object {
    public:
        PbrModelObject(std::shared_ptr<Pbr::Model> pbrModel = nullptr,
                       Pbr::ShadingMode shadingMode = Pbr::ShadingMode::Regular,
                       Pbr::FillMode fillMode = Pbr::FillMode::Solid);

        void SetModel(std::shared_ptr<Pbr::Model> model);
        std::shared_ptr<Pbr::Model> GetModel() const;

        void SetShadingMode(const Pbr::ShadingMode& shadingMode);
        void SetFillMode(const Pbr::FillMode& fillMode);
        void SetBaseColorFactor(Pbr::RGBAColor color);

        void Render(Context& context) const override;

    private:
        std::shared_ptr<Pbr::Model> m_pbrModel;
        Pbr::ShadingMode m_shadingMode;
        Pbr::FillMode m_fillMode;
    };

    // Helper for loading GLB files in the background. 
    struct PbrModelLoadOperation {
        PbrModelLoadOperation() = default;
        PbrModelLoadOperation(PbrModelLoadOperation&&) = default;
        PbrModelLoadOperation& operator=(PbrModelLoadOperation&&) = default;

        static PbrModelLoadOperation LoadGltfBinaryAsync(Pbr::Resources& pbrResources, std::wstring filename);

        // Take the model (can only be done once) once it has been loaded.
        std::shared_ptr<Pbr::Model> TakeModelWhenReady();

        // Dtor ensures outstanding operation is complete before returning.
        ~PbrModelLoadOperation();

    private:
        explicit PbrModelLoadOperation(std::future<std::shared_ptr<Pbr::Model>> loadModelTask);

        std::future<std::shared_ptr<Pbr::Model>> m_loadModelTask;
    };

    std::shared_ptr<PbrModelObject> CreateCube(const Pbr::Resources& pbrResources,
                                               DirectX::XMFLOAT3 sideLengths,
                                               Pbr::RGBAColor color,
                                               float roughness = 1.0f,
                                               float metallic = 0.0f);

    std::shared_ptr<PbrModelObject> CreateQuad(const Pbr::Resources& pbrResources,
                                               DirectX::XMFLOAT2 sideLengths,
                                               std::shared_ptr<Pbr::Material> material);

    std::shared_ptr<PbrModelObject> CreateSphere(const Pbr::Resources& pbrResources,
                                                 float size,
                                                 uint32_t tesselation,
                                                 Pbr::RGBAColor color,
                                                 float roughness = 1.0f,
                                                 float metallic = 0.0f);

    std::shared_ptr<PbrModelObject> CreateAxis(const Pbr::Resources& pbrResources,
                                               float axisLength = 1.0f,
                                               float axisThickness = 0.01f,
                                               float originAdditionalThickness = 0.01f,
                                               float roughness = 0.85f,
                                               float metallic = 0.01f);

} // namespace engine
