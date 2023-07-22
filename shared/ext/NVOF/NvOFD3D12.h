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
#include <d3d12.h>
#include <wrl.h>
#include "NvOF.h"
#include "nvOpticalFlowD3D12.h"

#define MAX_SUBRESOURCES 3
/**
*  @brief Optical flow class for D3D12 resources.
*/
class NvOFD3D12API : public NvOFAPI
{
public:
    NvOFD3D12API(ID3D12Device* device);
    ~NvOFD3D12API();

    NV_OF_D3D12_API_FUNCTION_LIST* GetAPI()
    {
        std::lock_guard<std::mutex> lock(m_lock);
        return  m_ofAPI.get();
    }

    NvOFHandle GetHandle() { return m_hOF; }
    Microsoft::WRL::ComPtr<ID3D12Device> GetDevice() { return m_device.Get(); }
private:
    NvOFHandle m_hOF;
    std::unique_ptr<NV_OF_D3D12_API_FUNCTION_LIST>              m_ofAPI;
    Microsoft::WRL::ComPtr<ID3D12Device>                        m_device;
};

class NvOFBufferD3D12;
/*
DX12 Interface is different from other device interfaces. Note the private inheritance of NvOF.
*/
class NvOFD3D12 : private NvOF
{
public:
    /*
    * @brief  This is a static function to create NvOFD3D12 interface.
    * Returns a managed pointer to base class NvOF object.
    */
    static NvOFObj Create(ID3D12Device* device, uint32_t nWidth, uint32_t nHeight, NV_OF_BUFFER_FORMAT eInBufFmt, NV_OF_MODE eMode, NV_OF_PERF_LEVEL preset);

    virtual ~NvOFD3D12() {};
private:
    NvOFD3D12(ID3D12Device* d3dDevice, uint32_t nWidth, uint32_t nHeight, NV_OF_BUFFER_FORMAT eInBufFmt,
        NV_OF_MODE eMode, NV_OF_PERF_LEVEL preset);

private:
    /**
    *  @brief This function is used to retrieve supported grid size for output.
    *  This function is an override of pure virtual function NvOF::DoGetOutputGridSizes().
    */
    virtual void DoGetOutputGridSizes(uint32_t* vals, uint32_t* size) override;

    /**
    *  @brief This function is used to retrieve if Region of Interest is supported or not.
    *  This function is an override of pure virtual function NvOF::IsROISupported().
    */
    virtual void DoGetROISupport(uint32_t* vals, uint32_t* size) override;

    /**
    *  @brief This function is used to initialize the OF engine.
    *  This function is an override of pure virtual function NvOF::DoInit().
    */
    virtual void DoInit(const NV_OF_INIT_PARAMS& initParams) override;

    /**
    *  @brief This function is used to estimate the optical flow between 2 images.
    *  This function is an override of pure virtual function NvOF::DoExecute().
    */
    virtual void DoExecute(const NV_OF_EXECUTE_INPUT_PARAMS& executeInParams,
        NV_OF_EXECUTE_OUTPUT_PARAMS& executeOutParams, void* arrInputFencePoint, uint32_t numInputFencePoint, void* outputFence) override;

    /**
    *  @brief This function is used to allocate buffers used for optical flow estimation.
    *  This function is an override of pure virtual function NvOF::DoAllocBuffers().
    */
    virtual  std::vector<NvOFBufferObj> DoAllocBuffers(NV_OF_BUFFER_DESCRIPTOR ofBufferDesc,
        uint32_t elementSize, uint32_t numBuffers, void* arrOutputFencePoint, uint32_t numOutputFencePoint) override;


    /**
    * @brief This function is used to register preallocated buffers.
    * This function is an override of pure virtual function NvOF::DoRegisterBuffers().
    */
    virtual  NvOFBufferObj DoRegisterBuffers(NV_OF_BUFFER_DESCRIPTOR ofBufferDesc,
        uint32_t elementSize, const void* pResource, void* inputFencePoint, void* outputFencePoint) override;

private:
    Microsoft::WRL::ComPtr<ID3D12Device> m_d3dDevice;
    std::shared_ptr<NvOFD3D12API>        m_NvOFAPI;
};

class NvOFBufferD3D12 : public NvOFBuffer
{
public:
    ~NvOFBufferD3D12();
    ID3D12Resource* getD3D12ResourceHandle() { return m_resource.Get(); }
    DXGI_FORMAT getFormat() { return m_format; }
    void* getAPIResourceHandle() override { return reinterpret_cast<void*>(getD3D12ResourceHandle()); }
private:
    NvOFBufferD3D12(std::shared_ptr<NvOFD3D12API> nvOFD3D12, const NV_OF_BUFFER_DESCRIPTOR& desc,
        uint32_t elementSize, ID3D12Resource* pResource, NV_OF_FENCE_POINT* inputFence, NV_OF_FENCE_POINT*  outputFence);
    Microsoft::WRL::ComPtr<ID3D12Resource> m_resource;
    DXGI_FORMAT m_format;
    std::shared_ptr<NvOFD3D12API> m_nvOFD3D12;
    friend NvOFD3D12;
};

struct NV_OF_CONSTANTBUFFER
{
    uint32_t srcWidth;
    uint32_t srcHeight;
    uint32_t dstWidth;
    uint32_t dstHeight;
    uint32_t nScaleFactor;
    uint32_t reserved1;
    uint32_t reserved2;
    uint32_t reserved3;
};
