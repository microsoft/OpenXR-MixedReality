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
#include <pbr/PbrModel.h>
#include "PbrModelObject.h"

using namespace DirectX;

PbrModelObject::PbrModelObject(std::shared_ptr<Pbr::Model> pbrModel, Pbr::ShadingMode shadingMode, Pbr::FillMode fillMode)
    : m_pbrModel(std::move(pbrModel))
    , m_shadingMode(shadingMode)
    , m_fillMode(fillMode) {
}

void PbrModelObject::SetModel(std::shared_ptr<Pbr::Model> model) {
    m_pbrModel = std::move(model);
}

std::shared_ptr<Pbr::Model> PbrModelObject::GetModel() const {
    return m_pbrModel;
}

void PbrModelObject::Render(SceneContext& sceneContext) const {
    if (!IsVisible() || !m_pbrModel) {
        return;
    }

    sceneContext.PbrResources.SetShadingMode(m_shadingMode);
    sceneContext.PbrResources.SetFillMode(m_fillMode);
    sceneContext.PbrResources.SetModelToWorld(WorldTransform(), sceneContext.DeviceContext.get());
    sceneContext.PbrResources.Bind(sceneContext.DeviceContext.get());
    m_pbrModel->Render(sceneContext.PbrResources, sceneContext.DeviceContext.get());
}

void PbrModelObject::SetShadingMode(const Pbr::ShadingMode& shadingMode) {
    m_shadingMode = shadingMode;
}

void PbrModelObject::SetFillMode(const Pbr::FillMode& fillMode) {
    m_fillMode = fillMode;
}

void PbrModelObject::SetBaseColorFactor(const Pbr::RGBAColor color) {
    for (uint32_t k = 0; k < GetModel()->GetPrimitiveCount(); k++) {
        auto& material = GetModel()->GetPrimitive(k).GetMaterial();
        material->Parameters().BaseColorFactor = color;
    }
}

std::shared_ptr<PbrModelObject> MakeCube(const Pbr::Resources& pbrResources,
                                         XMFLOAT3 sideLengths,
                                         const Pbr::RGBAColor color,
                                         float roughness /*= 1.0f*/,
                                         float metallic /*= 0.0f*/) {
    auto material = Pbr::Material::CreateFlat(pbrResources, color, roughness, metallic);
    auto cubeModel = std::make_shared<Pbr::Model>();
    cubeModel->AddPrimitive(Pbr::Primitive(pbrResources, Pbr::PrimitiveBuilder().AddCube(sideLengths), std::move(material)));
    return std::make_shared<PbrModelObject>(std::move(cubeModel));
}

std::shared_ptr<PbrModelObject> MakeQuad(const Pbr::Resources& pbrResources,
                                         XMFLOAT2 sideLengths,
                                         std::shared_ptr<Pbr::Material> material) {
    auto quadModel = std::make_shared<Pbr::Model>();
    quadModel->AddPrimitive(Pbr::Primitive(pbrResources, Pbr::PrimitiveBuilder().AddQuad(sideLengths), std::move(material)));
    return std::make_shared<PbrModelObject>(std::move(quadModel));
}

std::shared_ptr<PbrModelObject> MakeSphere(const Pbr::Resources& pbrResources,
                                           float size,
                                           uint32_t tesselation,
                                           Pbr::RGBAColor color,
                                           float roughness /*= 1.0f*/,
                                           float metallic /*= 0.0f*/) {
    auto material = Pbr::Material::CreateFlat(pbrResources, color, roughness, metallic);
    auto sphereModel = std::make_shared<Pbr::Model>();
    sphereModel->AddPrimitive(Pbr::Primitive(pbrResources, Pbr::PrimitiveBuilder().AddSphere(size, tesselation), std::move(material)));
    return std::make_shared<PbrModelObject>(std::move(sphereModel));
}

std::shared_ptr<PbrModelObject> MakeAxis(const Pbr::Resources& pbrResources,
                                         float axisLength /*= 1.0f*/,
                                         float axisThickness /*= 0.01f*/,
                                         float roughness /*= 0.85f*/,
                                         float metallic /*= 0.01f*/) {
    auto material = Pbr::Material::CreateFlat(pbrResources, Pbr::RGBA::White, roughness, metallic);

    auto axisModel = std::make_shared<Pbr::Model>();
    axisModel->AddPrimitive(Pbr::Primitive(pbrResources, Pbr::PrimitiveBuilder().AddAxis(axisLength, axisThickness), material));

    return std::make_shared<PbrModelObject>(std::move(axisModel));
}
