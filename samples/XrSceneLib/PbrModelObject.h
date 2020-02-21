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

#include "Scene.h"
#include "SceneContext.h"
#include <pbr/PbrModel.h>
#include <pbr/PbrMaterial.h>

class PbrModelObject : public SceneObject {
public:
    PbrModelObject(std::shared_ptr<Pbr::Model> pbrModel = nullptr,
                   Pbr::ShadingMode shadingMode = Pbr::ShadingMode::Regular,
                   Pbr::FillMode fillMode = Pbr::FillMode::Solid);

    void SetModel(std::shared_ptr<Pbr::Model> model);
    std::shared_ptr<Pbr::Model> GetModel() const;

    void SetShadingMode(const Pbr::ShadingMode& shadingMode);
    void SetFillMode(const Pbr::FillMode& fillMode);
    void SetBaseColorFactor(Pbr::RGBAColor color);

    void Render(SceneContext& sceneContext) const override;

private:
    std::shared_ptr<Pbr::Model> m_pbrModel;
    Pbr::ShadingMode m_shadingMode;
    Pbr::FillMode m_fillMode;
};

std::shared_ptr<PbrModelObject> MakeCube(const Pbr::Resources& pbrResources,
                                         DirectX::XMFLOAT3 sideLengths,
                                         Pbr::RGBAColor color,
                                         float roughness = 1.0f,
                                         float metallic = 0.0f);

std::shared_ptr<PbrModelObject> MakeQuad(const Pbr::Resources& pbrResources,
                                         DirectX::XMFLOAT2 sideLengths,
                                         std::shared_ptr<Pbr::Material> material);

std::shared_ptr<PbrModelObject> MakeSphere(const Pbr::Resources& pbrResources,
                                           float size,
                                           uint32_t tesselation,
                                           Pbr::RGBAColor color,
                                           float roughness = 1.0f,
                                           float metallic = 0.0f);

std::shared_ptr<PbrModelObject> MakeAxis(const Pbr::Resources& pbrResources,
                                         float axisLength = 1.0f,
                                         float axisThickness = 0.01f,
                                         float roughness = 0.85f,
                                         float metallic = 0.01f);
