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


#pragma once
#include <vector>
#include <stdint.h>
#include <mutex>
#include <string>
#include <iostream>
#include <sstream>
#include <string.h>
#include <memory>
#include <mutex>
#include "nvOpticalFlowCommon.h"
#include "NvOFDefines.h"

/**
 * @brief Exception class for error reporting from NvOFAPI calls
 */
class NvOFException : public std::exception
{
public:
    NvOFException(const std::string& errorStr, const NV_OF_STATUS errorCode)
        : m_errorString(errorStr), m_errorCode(errorCode) {}
    virtual ~NvOFException() throw() {}
    virtual const char* what() const throw() { return m_errorString.c_str(); }
    NV_OF_STATUS getErrorCode() const { return m_errorCode; }
    const std::string& getErrorString() const { return m_errorString; }
    static NvOFException makeNvOFException(const std::string& errorStr, const NV_OF_STATUS errorCode,
        const std::string& functionName, const std::string& fileName, int lineNo);

private:
    std::string m_errorString;
    NV_OF_STATUS m_errorCode;
};

inline NvOFException NvOFException::makeNvOFException(const std::string& errorStr, const NV_OF_STATUS errorCode, const std::string& functionName,
                                        const std::string& fileName, int lineNo)
{
    std::ostringstream errorLog;
    errorLog << functionName << " : " << errorStr << " at " << fileName << ";" << lineNo << std::endl;
    NvOFException exception(errorLog.str(), errorCode);
    return exception;
}

#define NVOF_THROW_ERROR(errorStr, errorCode)                                                           \
    do                                                                                                  \
    {                                                                                                   \
        throw NvOFException::makeNvOFException(errorStr, errorCode, __FUNCTION__, __FILE__, __LINE__);  \
    } while (0)

#define NVOF_API_CALL(nvOFAPI)                                                                          \
    do                                                                                                  \
    {                                                                                                   \
        NV_OF_STATUS errorCode = nvOFAPI;                                                               \
        if (errorCode != NV_OF_SUCCESS)                                                                 \
        {                                                                                               \
            std::ostringstream errorLog;                                                                \
            errorLog << #nvOFAPI << "returned error " << errorCode;                                    \
            throw NvOFException::makeNvOFException(errorLog.str(), errorCode, __FUNCTION__, __FILE__, __LINE__);    \
        }                                                                                               \
    } while (0)


struct NV_OF_FENCE_POINT_COMMON
{
    
};

/*
 * NvOFBuffer is a wrapper over the NvOFGPUBufferHandle object defined in
 * NVOF API and provides methods for various operations associated with the
 * GPU buffer.
 */
class NvOFBuffer
{
public:
    virtual ~NvOFBuffer() {}
    uint32_t getWidth() { return m_width; }
    uint32_t getHeight() { return m_height; }
    uint32_t getElementSize() { return m_elementSize; }
    NV_OF_BUFFER_FORMAT getBufferFormat() { return m_eBufFmt; }
    NV_OF_BUFFER_USAGE getBufferUsage() { return m_eBufUsage; }

    bool IsRegistered(void* pResource) { return m_hGPUBuffer != nullptr && pResource == getAPIResourceHandle(); }

protected:
    NvOFBuffer(const NV_OF_BUFFER_DESCRIPTOR& desc, uint32_t elementSize) :
        m_width(desc.width),
        m_height(desc.height),
        m_elementSize(elementSize),
        m_eBufFmt(desc.bufferFormat),
        m_eBufUsage(desc.bufferUsage),
        m_hGPUBuffer(nullptr)
    {
    }
    virtual void* getAPIResourceHandle() = 0;
    NvOFGPUBufferHandle getOFBufferHandle() { return m_hGPUBuffer; }

    NvOFGPUBufferHandle m_hGPUBuffer;
private:
    uint32_t m_width;
    uint32_t m_elementSize;
    uint32_t m_height;
    NV_OF_BUFFER_USAGE m_eBufUsage;
    NV_OF_BUFFER_FORMAT m_eBufFmt;
    friend class NvOF;
};


/*
 * NvOFAPI is a helper class for loading the library which implements the
 * NVOF API. Classes derived from this provide access to the common and
 * interface-specific API calls from NVOF API.
 */
class NvOFAPI
{
public:
    NvOFAPI()
    {
        LoadNvOFAPI();
    }
    virtual ~NvOFAPI();
protected:
    HMODULE m_hModule;
    std::mutex m_lock;
private:
    void LoadNvOFAPI();
};

/**
 * @brief Base class for different optical flow interfaces
 */
class NvOF
{
public:
    /**
     * @brief NvOF class virtual destructor
     */
    virtual ~NvOF() {};

    /**
    * @brief Create one or more GPU buffers for the specified usage mode
    */
    std::vector<NvOFBufferObj> CreateBuffers(NV_OF_BUFFER_USAGE usage, uint32_t numBuffers, void* arrOutputFence = nullptr, uint32_t numOutputFence = 0);

    /**
    * @brief Create one or more GPU buffers for the specified width, height and usage mode,
    */
    std::vector<NvOFBufferObj> CreateBuffers(uint32_t nWidth, uint32_t nHeight, NV_OF_BUFFER_USAGE usage,
        uint32_t numBuffers, void* arrOutputFence = nullptr, uint32_t numOutputFence = 0);

    /**
    * @brief This function is used to register preallocated resource with OF
    */
    virtual  NvOFBufferObj RegisterPreAllocBuffers(NV_OF_BUFFER_DESCRIPTOR ofBufferDesc,
        const void* pResource, void* pInputFencePoint=nullptr, void* pOutputFencePoint=nullptr);

    /**
    * @brief This function is used to estimate the optical flow from image1 to image2.
    */
    void Execute(NvOFBuffer* image1,
        NvOFBuffer*     image2,
        NvOFBuffer*     outputBuffer,
        NvOFBuffer*     hintBuffer = nullptr,
        NvOFBuffer*     costBuffer = nullptr,
        uint32_t        numROIs = 0,
        NV_OF_ROI_RECT* ROIData = nullptr,
        void*           arrInputFencePoint = nullptr,
        uint32_t        numInputFencePoint = 0,
        void*           pOutputFencePoint = nullptr,
        NV_OF_BOOL      disableTemporalHints = NV_OF_FALSE);

protected:

    /**
     * @brief NvOF class constructor.
     * NvOF class constructor cannot be called directly by the application.
     */
    NvOF(uint32_t nWidth, uint32_t nHeight, NV_OF_BUFFER_FORMAT eInBufFmt,
        NV_OF_MODE eMode = NV_OF_MODE_OPTICALFLOW,
        NV_OF_PERF_LEVEL preset = NV_OF_PERF_LEVEL_SLOW);
public:
    void Init(uint32_t nOutGridSize, uint32_t nHintGridSize = NV_OF_HINT_VECTOR_GRID_SIZE_UNDEFINED, bool bEnableExtHints = false, bool bEnableRoi = false);

    /*
     * Check for the grid size support by hw
     */
    bool CheckGridSize(uint32_t nOutGridSize);

    /*
    * Retrieves if the ROI supported or not
    */
    bool IsROISupported();

    /*
     * Retrieves the next minimum grid size supported for the specified grid size
     */
    bool GetNextMinGridSize(uint32_t nOutGridSize, uint32_t& nextMinOutGridSize);

private:
    /*
     * Retrieves the output grid sizes supported
     */
    virtual void DoGetOutputGridSizes(uint32_t* vals, uint32_t* size) = 0;
    
    /*
    * Retrieves the output grid sizes supported
    */
    virtual void DoGetROISupport(uint32_t* vals, uint32_t* size) = 0;

    /*
     * Initializes the NVOF API.
     */
    virtual void DoInit(const NV_OF_INIT_PARAMS& initParams ) = 0;

    /*
     * Executes the estimation of optical flow/stereo disparity between 2 images.
     */
    virtual void DoExecute(const NV_OF_EXECUTE_INPUT_PARAMS& executeInParams, 
        NV_OF_EXECUTE_OUTPUT_PARAMS& executeOutParams, void* arrInputFencePoint, uint32_t numInputFencePoint,  void* pOutputFencePoint = nullptr) = 0;

    /*
     * Allocates one or more GPU buffers.
     */
    virtual std::vector<NvOFBufferObj> DoAllocBuffers(NV_OF_BUFFER_DESCRIPTOR ofBufferDesc,
        uint32_t elementSize, uint32_t numBuffers,  void* arrOutputFencePoint, uint32_t numOutputFencePoint) = 0;

    /*
     * Registers given resources with OF 
     */

    virtual NvOFBufferObj DoRegisterBuffers(NV_OF_BUFFER_DESCRIPTOR ofBufferDesc,
        uint32_t elementSize, const void* pResource,void* inputFencePoint, void* outputFencePoint) = 0;

protected:
    uint32_t m_nHintGridSize;
    uint32_t m_nOutGridSize;
    NV_OF_PERF_LEVEL m_ePreset;
    NV_OF_MODE m_ofMode;
    NV_OF_BOOL m_bEnableRoi;
    NV_OF_BUFFER_DESCRIPTOR m_inputBufferDesc;
    NV_OF_BUFFER_DESCRIPTOR m_outputBufferDesc;
    NV_OF_BUFFER_DESCRIPTOR m_costBufferDesc;
    NV_OF_BUFFER_DESCRIPTOR m_hintBufferDesc;

    uint32_t m_outputElementSize;
    uint32_t m_inputElementSize;
    uint32_t m_costBufElementSize;
    uint32_t m_hintBufElementSize;

    NV_OF_INIT_PARAMS m_initParams;
};

