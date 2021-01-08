// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.


#include "pch.h"
#include <Pbr/GltfLoader.h>
#include <SampleShared/Trace.h>
#include "PbrModelObject.h"
#include "ControllerObject.h"
#include "Context.h"

using namespace DirectX;
using namespace std::literals::chrono_literals;

namespace {

    struct ControllerModel {
        XrControllerModelKeyMSFT Key = 0;
        std::shared_ptr<Pbr::Model> PbrModel;
        std::vector<Pbr::NodeIndex_t> NodeIndices;
        std::vector<XrControllerModelNodePropertiesMSFT> NodeProperties;
        std::vector<XrControllerModelNodeStateMSFT> NodeStates;
    };

    std::unique_ptr<ControllerModel> LoadControllerModel(engine::Context& context, XrControllerModelKeyMSFT modelKey) {
        std::unique_ptr<ControllerModel> model = std::make_unique<ControllerModel>();
        model->Key = modelKey;

        // Load the controller model as GLTF binary stream using two call idiom
        uint32_t bufferSize = 0;
        CHECK_XRCMD(context.Extensions.xrLoadControllerModelMSFT(context.Session.Handle, modelKey, 0, &bufferSize, nullptr));
        auto modelBuffer = std::make_unique<byte[]>(bufferSize);
        CHECK_XRCMD(
            context.Extensions.xrLoadControllerModelMSFT(context.Session.Handle, modelKey, bufferSize, &bufferSize, modelBuffer.get()));
        model->PbrModel = Gltf::FromGltfBinary(context.PbrResources, modelBuffer.get(), bufferSize);

        // Read the controller model properties with two call idiom
        XrControllerModelPropertiesMSFT properties{XR_TYPE_CONTROLLER_MODEL_PROPERTIES_MSFT};
        properties.nodeCapacityInput = 0;
        CHECK_XRCMD(context.Extensions.xrGetControllerModelPropertiesMSFT(context.Session.Handle, modelKey, &properties));
        model->NodeProperties.resize(properties.nodeCountOutput, {XR_TYPE_CONTROLLER_MODEL_NODE_PROPERTIES_MSFT});
        properties.nodeProperties = model->NodeProperties.data();
        properties.nodeCapacityInput = static_cast<uint32_t>(model->NodeProperties.size());
        CHECK_XRCMD(context.Extensions.xrGetControllerModelPropertiesMSFT(context.Session.Handle, modelKey, &properties));

        // Compute the index of each node reported by runtime to be animated.
        // The order of m_nodeIndices exactly matches the order of the nodes properties and states.
        model->NodeIndices.resize(model->NodeProperties.size(), Pbr::NodeIndex_npos);
        for (size_t i = 0; i < model->NodeProperties.size(); ++i) {
            const auto& nodeProperty = model->NodeProperties[i];
            const std::string_view parentNodeName = nodeProperty.parentNodeName;
            if (const auto parentNodeIndex = model->PbrModel->FindFirstNode(parentNodeName)) {
                if (const auto targetNodeIndex = model->PbrModel->FindFirstNode(nodeProperty.nodeName, *parentNodeIndex)) {
                    model->NodeIndices[i] = *targetNodeIndex;
                }
            }
        }

        return model;
    }

    // Update transforms of nodes for the animatable parts in the controller model
    void UpdateControllerParts(engine::Context& context, ControllerModel& model) {
        XrControllerModelStateMSFT modelState{XR_TYPE_CONTROLLER_MODEL_STATE_MSFT};
        modelState.nodeCapacityInput = 0;
        CHECK_XRCMD(context.Extensions.xrGetControllerModelStateMSFT(context.Session.Handle, model.Key, &modelState));

        model.NodeStates.resize(modelState.nodeCountOutput, {XR_TYPE_CONTROLLER_MODEL_STATE_MSFT});
        modelState.nodeCapacityInput = static_cast<uint32_t>(model.NodeStates.size());
        modelState.nodeStates = model.NodeStates.data();
        CHECK_XRCMD(context.Extensions.xrGetControllerModelStateMSFT(context.Session.Handle, model.Key, &modelState));

        assert(model.NodeStates.size() == model.NodeIndices.size());
        const size_t end = std::min(model.NodeStates.size(), model.NodeIndices.size());
        for (size_t i = 0; i < end; i++) {
            const Pbr::NodeIndex_t nodeIndex = model.NodeIndices[i];
            if (nodeIndex != Pbr::NodeIndex_npos) {
                Pbr::Node& node = model.PbrModel->GetNode(nodeIndex);
                node.SetTransform(xr::math::LoadXrPose(model.NodeStates[i].nodePose));
            }
        }
    }

    struct ControllerObject : engine::PbrModelObject {
        ControllerObject(engine::Context& context, XrPath controllerUserPath);
        ~ControllerObject();

        void Update(engine::Context& context, const engine::FrameTime& frameTime) override;

    private:
        const bool m_extensionSupported;
        const XrPath m_controllerUserPath;

        std::unique_ptr<ControllerModel> m_model;
        std::future<std::unique_ptr<ControllerModel>> m_modelLoadingTask;
    };

    ControllerObject::ControllerObject(engine::Context& context, XrPath controllerUserPath)
        : m_extensionSupported(context.Extensions.SupportsControllerModel)
        , m_controllerUserPath(controllerUserPath) {
    }

    ControllerObject::~ControllerObject() {
        // Wait for model loading task to complete before dtor complete because it captures the scene context.
        if (m_modelLoadingTask.valid()) {
            m_modelLoadingTask.wait();
        }
    }

    void ControllerObject::Update(engine::Context& context, const engine::FrameTime& frameTime) {
        if (!m_extensionSupported) {
            return; // The current runtime doesn't support controller model extension.
        }

        XrControllerModelKeyStateMSFT controllerModelKeyState{XR_TYPE_CONTROLLER_MODEL_KEY_STATE_MSFT};
        CHECK_XRCMD(context.Extensions.xrGetControllerModelKeyMSFT(context.Session.Handle, m_controllerUserPath, &controllerModelKeyState));

        // If a new valid model key is returned, reload the model into cache asynchronizely
        const bool modelKeyValid = controllerModelKeyState.modelKey != XR_NULL_CONTROLLER_MODEL_KEY_MSFT;
        if (modelKeyValid && (m_model == nullptr || m_model->Key != controllerModelKeyState.modelKey)) {
            // Avoid two background tasks running together. The new one will start in future update after the old one is finished.
            if (!m_modelLoadingTask.valid()) {
                m_modelLoadingTask = std::async(std::launch::async, [&, modelKey = controllerModelKeyState.modelKey]() {
                    return LoadControllerModel(context, modelKey);
                });
            }
        }

        // If controller model loading task is completed, get the result model and apply it to rendering.
        if (m_modelLoadingTask.valid() && m_modelLoadingTask.wait_for(0s) == std::future_status::ready) {
            try {
                m_model = m_modelLoadingTask.get(); // future.valid() is reset to false after get()
            } catch (...) {
                sample::Trace("Unexpected failure loading controller model");
            }
            if (m_model) {
                SetModel(m_model->PbrModel);
            }
        }

        // If controller model is already loaded, update all node transforms
        if (m_model != nullptr) {
            UpdateControllerParts(context, *m_model);
        }
    }
} // namespace

namespace engine {
    std::shared_ptr<engine::PbrModelObject> CreateControllerObject(Context& context, XrPath controllerUserPath) {
        return std::make_shared<ControllerObject>(context, controllerUserPath);
    }

} // namespace engine
