/*
* Copyright 2018-2022 NVIDIA Corporation.  All rights reserved.
*
* Please refer to the NVIDIA end user license agreement (EULA) associated
* with this source code for terms and conditions that govern your use of
* this software. Any use, reproduction, disclosure, or distribution of
* this software and related documentation outside the terms of the EULA
* is strictly prohibited.
*
*/


#include "NvOFD3D12.h"
#include "NvOFD3DCommon.h"

void CreateBufferParamsD3D12(D3D12_HEAP_PROPERTIES* pHeapProps, D3D12_RESOURCE_DESC* pDesc, DXGI_FORMAT format,
    NV_OF_BUFFER_DESCRIPTOR nvOfDesc, uint32_t outputGridSize, uint32_t hintGridSize)
{
    uint32_t widthAligned = 0;
    uint32_t heightAligned = 0;
    uint32_t width = nvOfDesc.width;
    uint32_t height = nvOfDesc.height;

    switch (nvOfDesc.bufferUsage)
    {
    case NV_OF_BUFFER_USAGE_INPUT:
        widthAligned = nvOfDesc.width;
        heightAligned = nvOfDesc.height;
        break;
    case NV_OF_BUFFER_USAGE_OUTPUT:
        widthAligned = ((width + outputGridSize - 1) & (~(outputGridSize - 1))) / outputGridSize;
        heightAligned = ((height + outputGridSize - 1) & (~(outputGridSize - 1))) / outputGridSize;
        break;
    case NV_OF_BUFFER_USAGE_COST:
        widthAligned = ((width + outputGridSize - 1) & (~(outputGridSize - 1))) / outputGridSize;
        heightAligned = ((height + outputGridSize - 1) & (~(outputGridSize - 1))) / outputGridSize;
        break;
    case NV_OF_BUFFER_USAGE_HINT:
        widthAligned = ((width + hintGridSize - 1) & (~(hintGridSize - 1))) / hintGridSize;
        heightAligned = ((height + hintGridSize - 1) & (~(hintGridSize - 1))) / hintGridSize;
        break;
    default:
        NVOF_THROW_ERROR("Invalid buffer format", NV_OF_ERR_INVALID_PARAM);
    }

    if (pHeapProps && pDesc)
    {
        ZeroMemory(pHeapProps, sizeof(D3D12_HEAP_PROPERTIES));
        pHeapProps->Type = D3D12_HEAP_TYPE_DEFAULT;

        ZeroMemory(pDesc, sizeof(D3D12_RESOURCE_DESC));
        pDesc->Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        pDesc->Width = widthAligned;
        pDesc->Height = heightAligned;
        pDesc->DepthOrArraySize = 1;
        pDesc->MipLevels = 1;
        pDesc->Format = format;
        pDesc->SampleDesc.Count = 1;
    }
}

NvOFD3D12API::NvOFD3D12API(ID3D12Device* device)
    : m_device(device)
    , m_hOF(nullptr)
{
    typedef NV_OF_STATUS(NVOFAPI *PFNNvOFAPICreateInstanceD3D12)(uint32_t apiVer, NV_OF_D3D12_API_FUNCTION_LIST  *d3d11OF);
    PFNNvOFAPICreateInstanceD3D12 NvOFAPICreateInstanceD3D12 = (PFNNvOFAPICreateInstanceD3D12)GetProcAddress(m_hModule, "NvOFAPICreateInstanceD3D12");
    if (!NvOFAPICreateInstanceD3D12)
    {
        NVOF_THROW_ERROR("Cannot find NvOFAPICreateInstanceDX12() entry in NVOF library", NV_OF_ERR_OF_NOT_AVAILABLE);
    }

    m_ofAPI.reset(new NV_OF_D3D12_API_FUNCTION_LIST());
    NV_OF_STATUS status = NvOFAPICreateInstanceD3D12(NV_OF_API_VERSION, m_ofAPI.get());
    if (status != NV_OF_SUCCESS)
    {
        NVOF_THROW_ERROR("Cannot fetch function list", status);
    }
    status = m_ofAPI->nvCreateOpticalFlowD3D12(device, &m_hOF);
    if (status != NV_OF_SUCCESS || m_hOF == nullptr)
    {
        NVOF_THROW_ERROR("Cannot create D3D12 optical flow device", status);
    }
}

NvOFD3D12API::~NvOFD3D12API()
{
    if (m_ofAPI)
    {
        m_ofAPI->nvOFDestroy(m_hOF);
    }
}

NvOFObj NvOFD3D12::Create(ID3D12Device* d3dDevice, uint32_t nWidth, uint32_t nHeight, NV_OF_BUFFER_FORMAT eInBufFmt,
    NV_OF_MODE eMode, NV_OF_PERF_LEVEL preset)
{
    std::unique_ptr<NvOF> ofObj(new NvOFD3D12(d3dDevice,
        nWidth,
        nHeight,
        eInBufFmt,
        eMode,
        preset));
    return ofObj;
}

NvOFD3D12::NvOFD3D12(ID3D12Device* d3dDevice, uint32_t nWidth, uint32_t nHeight, NV_OF_BUFFER_FORMAT eInBufFmt, NV_OF_MODE eMode,
    NV_OF_PERF_LEVEL preset)
    : NvOF(nWidth, nHeight, eInBufFmt, eMode, preset),
    m_d3dDevice(d3dDevice)
 {
    m_NvOFAPI = std::make_shared<NvOFD3D12API>(m_d3dDevice.Get());
    uint32_t formatCount = 0;
    bool bInputFormatSupported = false;
    bool bOutputFormatSupported = false;

    NVOF_API_CALL(m_NvOFAPI->GetAPI()->nvOFGetSurfaceFormatCountD3D12(m_NvOFAPI->GetHandle(), NV_OF_BUFFER_USAGE_INPUT, m_ofMode, &formatCount));
    std::unique_ptr<DXGI_FORMAT[]> pDxgiFormat(new DXGI_FORMAT[formatCount]);
    NVOF_API_CALL(m_NvOFAPI->GetAPI()->nvOFGetSurfaceFormatD3D12(m_NvOFAPI->GetHandle(), NV_OF_BUFFER_USAGE_INPUT, m_ofMode, pDxgiFormat.get()));

    for (uint32_t i = 0; i < formatCount; ++i)
    {
        if (m_inputBufferDesc.bufferFormat == DXGIFormatToNvOFBufferFormat(pDxgiFormat[i]))
        {
            bInputFormatSupported = true;
        }
    }
    auto outBufFmt = (m_ofMode == NV_OF_MODE_OPTICALFLOW) ? NV_OF_BUFFER_FORMAT_SHORT2 : NV_OF_BUFFER_FORMAT_SHORT;
    formatCount = 0;
    NVOF_API_CALL(m_NvOFAPI->GetAPI()->nvOFGetSurfaceFormatCountD3D12(m_NvOFAPI->GetHandle(), NV_OF_BUFFER_USAGE_OUTPUT, m_ofMode, &formatCount));
    pDxgiFormat.reset(new DXGI_FORMAT[formatCount]);
    NVOF_API_CALL(m_NvOFAPI->GetAPI()->nvOFGetSurfaceFormatD3D12(m_NvOFAPI->GetHandle(), NV_OF_BUFFER_USAGE_OUTPUT, m_ofMode, pDxgiFormat.get()));

    for (uint32_t i = 0; i < formatCount; ++i)
    {
        if (outBufFmt == DXGIFormatToNvOFBufferFormat(pDxgiFormat[i]))
        {
            bOutputFormatSupported = true;
        }
    }

    if (!bOutputFormatSupported || !bInputFormatSupported)
    {
        NVOF_THROW_ERROR("Invalid buffer format", NV_OF_ERR_INVALID_PARAM);
    }
}

void NvOFD3D12::DoGetOutputGridSizes(uint32_t* vals, uint32_t* size)
{
    NVOF_API_CALL(m_NvOFAPI->GetAPI()->nvOFGetCaps(m_NvOFAPI->GetHandle(), NV_OF_CAPS_SUPPORTED_OUTPUT_GRID_SIZES, vals, size));
}

void NvOFD3D12::DoGetROISupport(uint32_t* vals, uint32_t* size)
{
    NVOF_API_CALL(m_NvOFAPI->GetAPI()->nvOFGetCaps(m_NvOFAPI->GetHandle(), NV_OF_CAPS_SUPPORT_ROI, vals, size));
}

void NvOFD3D12::DoExecute(const NV_OF_EXECUTE_INPUT_PARAMS& executeInParams,  NV_OF_EXECUTE_OUTPUT_PARAMS& executeOutParams,  void* arrInputFencePoint, uint32_t numInputFencePoint, void* pOutputFencePoint)
{
    NV_OF_EXECUTE_INPUT_PARAMS_D3D12 executeInParamsD3D12;
    memcpy(&executeInParamsD3D12, &executeInParams, sizeof(executeInParams));

    if (arrInputFencePoint == nullptr)
    {
        NVOF_THROW_ERROR("arrInputFencePoint must be set to an array of NV_OF_FENCE_POINT. Execute() will wait for these fences to reach before execution", NV_OF_ERR_INVALID_PARAM);
    }
    if (numInputFencePoint == 0)
    {
        NVOF_THROW_ERROR("numInputFencePoint must be non-zero", NV_OF_ERR_INVALID_PARAM);
    }
    if (pOutputFencePoint == nullptr)
    {
        NVOF_THROW_ERROR("pOuputFencePoint must be set to a NV_OF_FENCE_POINT pointer", NV_OF_ERR_INVALID_PARAM);
    }

    executeInParamsD3D12.fencePoint = (NV_OF_FENCE_POINT*) arrInputFencePoint;
    executeInParamsD3D12.numFencePoints = numInputFencePoint;

    NV_OF_EXECUTE_OUTPUT_PARAMS_D3D12 executeOutParamsD3D12;
    memcpy(&executeOutParamsD3D12, &executeOutParams, sizeof(executeOutParams));
    executeOutParamsD3D12.fencePoint = (NV_OF_FENCE_POINT*)pOutputFencePoint;
   
    NVOF_API_CALL(m_NvOFAPI->GetAPI()->nvOFExecuteD3D12(m_NvOFAPI->GetHandle(), &executeInParamsD3D12, &executeOutParamsD3D12));
}

void NvOFD3D12::DoInit(const NV_OF_INIT_PARAMS& initParams)
{
    NVOF_API_CALL(m_NvOFAPI->GetAPI()->nvOFInit(m_NvOFAPI->GetHandle(), &initParams));
}

std::vector<NvOFBufferObj>
NvOFD3D12::DoAllocBuffers(NV_OF_BUFFER_DESCRIPTOR ofBufferDesc,
    uint32_t elementSize, uint32_t numBuffers, void* arrOutputFencePoint, uint32_t numOutputFencePoint)
{
    std::vector<std::unique_ptr<NvOFBuffer>> ofBuffers;
    NV_OF_FENCE_POINT inputFence;
    D3D_API_CALL(m_d3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&(inputFence.fence))));
    inputFence.value = 0;
    if (numBuffers != numOutputFencePoint)
    {
        NVOF_THROW_ERROR("numBuffers must be equal to numOutputFencePoint", NV_OF_ERR_INVALID_PARAM);
    }
    for (uint32_t i = 0; i < numBuffers; ++i)
    {
        D3D12_HEAP_PROPERTIES heapProps;
        D3D12_RESOURCE_DESC desc;
        ID3D12Resource* pResource = nullptr; 
        
        CreateBufferParamsD3D12(&heapProps, &desc, NvOFBufferFormatToDxgiFormat(ofBufferDesc.bufferFormat), ofBufferDesc, m_nOutGridSize, m_nHintGridSize);
        D3D_API_CALL(m_d3dDevice.Get()->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE,
            &desc, D3D12_RESOURCE_STATE_COMMON, nullptr,
            IID_PPV_ARGS(&pResource)));
        
        NV_OF_FENCE_POINT* outputFenceArray = (NV_OF_FENCE_POINT*)arrOutputFencePoint;
        NV_OF_FENCE_POINT outputFence = outputFenceArray[i];
        ofBuffers.emplace_back(new NvOFBufferD3D12(m_NvOFAPI, ofBufferDesc, elementSize, pResource, &inputFence, &outputFence));
    }
    return ofBuffers;
}

NvOFBufferObj NvOFD3D12::DoRegisterBuffers(NV_OF_BUFFER_DESCRIPTOR ofBufferDesc,
    uint32_t elementSize, const void* pResource, void* inputFencePoint, void* outputFencePoint)
{
    ID3D12Resource* pRes = reinterpret_cast<ID3D12Resource*>(const_cast<void*>(pResource));
    D3D12_RESOURCE_DESC inputDesc = pRes->GetDesc();
    bool bError = (inputDesc.Format != NvOFBufferFormatToDxgiFormat(ofBufferDesc.bufferFormat));
    bError &= (inputDesc.Width < ofBufferDesc.width);
    bError &= (inputDesc.Height < ofBufferDesc.height);
    if (bError)
    {
        NVOF_THROW_ERROR("Resource does not match with params provided during NvOF Init", NV_OF_ERR_INVALID_PARAM);
    }
    NvOFBufferObj ofBuffer(new NvOFBufferD3D12(m_NvOFAPI, ofBufferDesc, elementSize,
        pRes, (NV_OF_FENCE_POINT*)inputFencePoint, (NV_OF_FENCE_POINT*)outputFencePoint));
    return ofBuffer;
}

NvOFBufferD3D12::NvOFBufferD3D12(std::shared_ptr<NvOFD3D12API> ofD3D12, const NV_OF_BUFFER_DESCRIPTOR& nvBufDesc,
    uint32_t elementSize, ID3D12Resource* pResource, NV_OF_FENCE_POINT* inputFence, NV_OF_FENCE_POINT* outputFence ) :
    NvOFBuffer(nvBufDesc, elementSize), m_resource(pResource), m_nvOFD3D12(ofD3D12)
{
    Microsoft::WRL::ComPtr<ID3D12Device> dx12Device = ofD3D12->GetDevice();
    
    NV_OF_REGISTER_RESOURCE_PARAMS_D3D12 registerParams{};
    registerParams.resource = pResource;
    registerParams.inputFencePoint.fence = inputFence->fence;
    registerParams.inputFencePoint.value = inputFence->value; 
    registerParams.hOFGpuBuffer = &m_hGPUBuffer;
    registerParams.outputFencePoint.fence = outputFence->fence;
    registerParams.outputFencePoint.value = outputFence->value;
    NVOF_API_CALL(ofD3D12->GetAPI()->nvOFRegisterResourceD3D12(ofD3D12->GetHandle(), &registerParams));

    m_format = NvOFBufferFormatToDxgiFormat(nvBufDesc.bufferFormat);
}

NvOFBufferD3D12::~NvOFBufferD3D12()
{
    NV_OF_UNREGISTER_RESOURCE_PARAMS_D3D12 param;
    param.hOFGpuBuffer = getOFBufferHandle();
    m_nvOFD3D12.get()->GetAPI()->nvOFUnregisterResourceD3D12(&param);
}
