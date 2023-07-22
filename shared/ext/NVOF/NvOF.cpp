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


#ifdef _WIN32
#include <Windows.h>
#else
#include <dlfcn.h>
#endif

#include "NvOF.h"

NvOF::NvOF(uint32_t nWidth, uint32_t nHeight, NV_OF_BUFFER_FORMAT eInBufFmt, NV_OF_MODE eMode, 
    NV_OF_PERF_LEVEL preset) :
    m_ofMode(eMode),
    m_nOutGridSize(NV_OF_OUTPUT_VECTOR_GRID_SIZE_MAX),
    m_nHintGridSize(NV_OF_HINT_VECTOR_GRID_SIZE_UNDEFINED),
    m_ePreset(preset)
{
    m_bEnableRoi = NV_OF_FALSE;
    m_inputElementSize = 1;
    if (eInBufFmt == NV_OF_BUFFER_FORMAT_ABGR8)
        m_inputElementSize = 4;


    memset(&m_inputBufferDesc, 0, sizeof(m_inputBufferDesc));
    m_inputBufferDesc.width = nWidth;
    m_inputBufferDesc.height = nHeight;
    m_inputBufferDesc.bufferFormat = eInBufFmt;
    m_inputBufferDesc.bufferUsage = NV_OF_BUFFER_USAGE_INPUT;

    memset(&m_hintBufferDesc, 0, sizeof(m_hintBufferDesc));

}

bool NvOF::CheckGridSize(uint32_t nOutGridSize)
{
    uint32_t size;
    DoGetOutputGridSizes(nullptr, &size);

    std::unique_ptr<uint32_t[]> val(new uint32_t[size]);
    DoGetOutputGridSizes(val.get(), &size);

    for (uint32_t i = 0; i < size; i++)
    {
        if (nOutGridSize == val[i])
        {
            return true;
        }
    }
    return false;
}

bool NvOF::IsROISupported()
{
    uint32_t size;
    DoGetROISupport(nullptr, &size);

    std::unique_ptr<uint32_t[]> val(new uint32_t[size]);
    DoGetROISupport(val.get(), &size);
    return (val[0] == NV_OF_TRUE)? true : false;
}
bool NvOF::GetNextMinGridSize(uint32_t nOutGridSize, uint32_t& nextMinOutGridSize)
{
    uint32_t size;
    DoGetOutputGridSizes(nullptr, &size);

    std::unique_ptr<uint32_t[]> val(new uint32_t[size]);
    DoGetOutputGridSizes(val.get(), &size);

    nextMinOutGridSize = NV_OF_OUTPUT_VECTOR_GRID_SIZE_MAX;
    for (uint32_t i = 0; i < size; i++)
    {
        if (nOutGridSize == val[i])
        {
            nextMinOutGridSize = nOutGridSize;
            return true;
        }
        if (nOutGridSize < val[i] && val[i] < nextMinOutGridSize)
        {
            nextMinOutGridSize = val[i];
        }
    }
    return (nextMinOutGridSize >= NV_OF_OUTPUT_VECTOR_GRID_SIZE_MAX) ? false : true;
}

void NvOF::Init(uint32_t nOutGridSize, uint32_t nHintGridSize, bool bEnableHints, bool bEnableRoi)
{
    m_nOutGridSize = nOutGridSize;
    m_bEnableRoi = (NV_OF_BOOL)bEnableRoi;

    auto nOutWidth = (m_inputBufferDesc.width + m_nOutGridSize - 1) / m_nOutGridSize;
    auto nOutHeight = (m_inputBufferDesc.height + m_nOutGridSize - 1) / m_nOutGridSize;

    auto outBufFmt = NV_OF_BUFFER_FORMAT_SHORT2;
    if (m_ofMode == NV_OF_MODE_OPTICALFLOW)
    {
        outBufFmt = NV_OF_BUFFER_FORMAT_SHORT2;
        m_outputElementSize = sizeof(NV_OF_FLOW_VECTOR);
    }
    else if (m_ofMode == NV_OF_MODE_STEREODISPARITY)
    {
        outBufFmt = NV_OF_BUFFER_FORMAT_SHORT;
        m_outputElementSize = sizeof(NV_OF_STEREO_DISPARITY);
    }
    else
    {
        NVOF_THROW_ERROR("Unsupported OF mode", NV_OF_ERR_INVALID_PARAM);
    }

    memset(&m_outputBufferDesc, 0, sizeof(m_outputBufferDesc));
    m_outputBufferDesc.width = nOutWidth;
    m_outputBufferDesc.height = nOutHeight;
    m_outputBufferDesc.bufferFormat = outBufFmt;
    m_outputBufferDesc.bufferUsage = NV_OF_BUFFER_USAGE_OUTPUT;

    memset(&m_costBufferDesc, 0, sizeof(m_costBufferDesc));
    m_costBufferDesc.width = nOutWidth;
    m_costBufferDesc.height = nOutHeight;
    m_costBufferDesc.bufferFormat = NV_OF_BUFFER_FORMAT_UINT8;
    m_costBufferDesc.bufferUsage = NV_OF_BUFFER_USAGE_COST;
    m_costBufElementSize = sizeof(uint32_t);

    if (bEnableHints)
    {
        m_nHintGridSize = nHintGridSize;
        auto nHintWidth = (m_inputBufferDesc.width + m_nHintGridSize - 1) / m_nHintGridSize;
        auto nHintHeight = (m_inputBufferDesc.height + m_nHintGridSize - 1) / m_nHintGridSize;
        m_hintBufferDesc.width = nHintWidth;
        m_hintBufferDesc.height = nHintHeight;
        m_hintBufferDesc.bufferFormat = outBufFmt;
        m_hintBufferDesc.bufferUsage = NV_OF_BUFFER_USAGE_HINT;
        m_hintBufElementSize = m_outputElementSize;
    }

    memset(&m_initParams, 0, sizeof(m_initParams));
    m_initParams.width = m_inputBufferDesc.width;
    m_initParams.height = m_inputBufferDesc.height;
    m_initParams.enableExternalHints = bEnableHints ? NV_OF_TRUE : NV_OF_FALSE;
    m_initParams.enableOutputCost = NV_OF_FALSE;
    m_initParams.hintGridSize = (NV_OF_HINT_VECTOR_GRID_SIZE) m_nHintGridSize;
    m_initParams.outGridSize = (NV_OF_OUTPUT_VECTOR_GRID_SIZE)m_nOutGridSize;
    m_initParams.mode = m_ofMode;
    m_initParams.perfLevel = m_ePreset;
    m_initParams.enableRoi = m_bEnableRoi;
    DoInit(m_initParams);
}

void NvOF::Execute(NvOFBuffer* image1,
    NvOFBuffer* image2,
    NvOFBuffer* outputBuffer,
    NvOFBuffer* hintBuffer,
    NvOFBuffer* costBuffer,
    uint32_t numROIs,
    NV_OF_ROI_RECT *ROIData,
    void*    arrInputFencePoint,
    uint32_t numInputFencePoint,
    void*    pOutputFencePoint,
    NV_OF_BOOL disableTemporalHints)
{
    NV_OF_EXECUTE_INPUT_PARAMS exeInParams;
    NV_OF_EXECUTE_OUTPUT_PARAMS exeOutParams;

    memset(&exeInParams, 0, sizeof(exeInParams));
    exeInParams.inputFrame = image1->getOFBufferHandle();
    exeInParams.referenceFrame = image2->getOFBufferHandle();
    exeInParams.disableTemporalHints = disableTemporalHints;
    exeInParams.externalHints = m_initParams.enableExternalHints == NV_OF_TRUE ? hintBuffer->getOFBufferHandle() : nullptr;
    exeInParams.numRois = numROIs;
    exeInParams.roiData = numROIs != 0 ? ROIData : NULL;

    memset(&exeOutParams, 0, sizeof(exeOutParams));
    exeOutParams.outputBuffer = outputBuffer->getOFBufferHandle();
    exeOutParams.outputCostBuffer = m_initParams.enableOutputCost == NV_OF_TRUE ? costBuffer->getOFBufferHandle() : nullptr;
    DoExecute(exeInParams, exeOutParams, arrInputFencePoint, numInputFencePoint, pOutputFencePoint);
}


std::vector<NvOFBufferObj>
NvOF::CreateBuffers(NV_OF_BUFFER_USAGE usage, uint32_t numBuffers, void* arrOutputFencePoint, uint32_t numOutputFencePoint)
{
    std::vector<NvOFBufferObj> ofBuffers;

    if (usage == NV_OF_BUFFER_USAGE_INPUT)
    {
        ofBuffers = DoAllocBuffers(m_inputBufferDesc, m_inputElementSize, numBuffers, arrOutputFencePoint, numOutputFencePoint);
    }
    else if (usage == NV_OF_BUFFER_USAGE_OUTPUT)
    {
        ofBuffers = DoAllocBuffers(m_outputBufferDesc, m_outputElementSize, numBuffers, arrOutputFencePoint, numOutputFencePoint);
    }
    else if (usage == NV_OF_BUFFER_USAGE_COST)
    {
        ofBuffers = DoAllocBuffers(m_costBufferDesc, m_costBufElementSize, numBuffers,  arrOutputFencePoint, numOutputFencePoint);
    }
    else if (usage == NV_OF_BUFFER_USAGE_HINT)
    {
        ofBuffers = DoAllocBuffers(m_hintBufferDesc, m_hintBufElementSize, numBuffers,  arrOutputFencePoint, numOutputFencePoint);
    }
    else
    {
        throw std::runtime_error("Invalid parameter");
    }

    return ofBuffers;
}

std::vector<NvOFBufferObj>
NvOF::CreateBuffers(uint32_t nWidth, uint32_t nHeight, NV_OF_BUFFER_USAGE usage, uint32_t numBuffers, void* arrOutputFencePoint, uint32_t numOutputFencePoint)
{
    std::vector<NvOFBufferObj> ofBuffers;

    NV_OF_BUFFER_DESCRIPTOR bufferDesc;

    if (usage == NV_OF_BUFFER_USAGE_OUTPUT)
    {
        bufferDesc.width = nWidth;
        bufferDesc.height = nHeight;
        bufferDesc.bufferFormat = m_outputBufferDesc.bufferFormat;
        bufferDesc.bufferUsage = NV_OF_BUFFER_USAGE_OUTPUT;

        ofBuffers = DoAllocBuffers(bufferDesc, m_outputElementSize, numBuffers, arrOutputFencePoint, numOutputFencePoint);
    }
    else
    {
        throw std::runtime_error("Invalid parameter");
    }

    return ofBuffers;
}

NvOFBufferObj NvOF::RegisterPreAllocBuffers(NV_OF_BUFFER_DESCRIPTOR ofBufDesc,
    const void* pResource,void* inputFencePoint, void* outputFencePoint)
{
    NV_OF_BUFFER_USAGE usage = ofBufDesc.bufferUsage;
    NvOFBufferObj ofBuffers;
    uint32_t elementSize;
    if (usage == NV_OF_BUFFER_USAGE_INPUT)
    {
        elementSize = m_inputElementSize;
    }
    else if (usage == NV_OF_BUFFER_USAGE_OUTPUT)
    {
        elementSize = m_outputElementSize;
    }
    else if (usage == NV_OF_BUFFER_USAGE_COST)
    {
        elementSize = m_costBufElementSize;
    }
    else if (usage == NV_OF_BUFFER_USAGE_HINT)
    {
        elementSize = m_hintBufElementSize;
    }
    else
    {
        throw std::runtime_error("Invalid parameter");
    }
    ofBuffers = DoRegisterBuffers(ofBufDesc, elementSize, pResource, inputFencePoint, outputFencePoint);
    return ofBuffers;
}

void NvOFAPI::LoadNvOFAPI()
{
#if defined(_WIN32)
#if defined(_WIN64)
    HMODULE hModule = LoadLibrary(TEXT("nvofapi64.dll"));
#else
    HMODULE hModule = LoadLibrary(TEXT("nvofapi.dll"));
#endif
#else
    void *hModule = dlopen("libnvidia-opticalflow.so.1", RTLD_LAZY);
#endif
    if (hModule == NULL)
    {
        NVOF_THROW_ERROR("NVOF library file not found. Please ensure that the NVIDIA driver is installed", NV_OF_ERR_OF_NOT_AVAILABLE);
    }

    m_hModule = hModule;
}

NvOFAPI::~NvOFAPI()
{
    if (m_hModule)
    {
#if defined(_WIN32) || defined(_WIN64)
        FreeLibrary(m_hModule);
#else
        dlclose(m_hModule);
#endif
    }
}
