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


#include "NvOFD3DCommon.h"

NV_OF_BUFFER_FORMAT DXGIFormatToNvOFBufferFormat(DXGI_FORMAT dxgiFormat)
{
    NV_OF_BUFFER_FORMAT ofBufFormat;
    switch (dxgiFormat)
    {
    case DXGI_FORMAT_B8G8R8A8_UNORM:
        ofBufFormat = NV_OF_BUFFER_FORMAT_ABGR8;
        break;
    case DXGI_FORMAT_R16G16_SINT:
        ofBufFormat = NV_OF_BUFFER_FORMAT_SHORT2;
        break;
    case DXGI_FORMAT_R16_UINT:
        ofBufFormat = NV_OF_BUFFER_FORMAT_SHORT;
        break;
    case DXGI_FORMAT_NV12:
        ofBufFormat = NV_OF_BUFFER_FORMAT_NV12;
        break;
    case DXGI_FORMAT_R32_UINT:
        ofBufFormat = NV_OF_BUFFER_FORMAT_UINT;
        break;
    case DXGI_FORMAT_R8_UNORM:
        ofBufFormat = NV_OF_BUFFER_FORMAT_GRAYSCALE8;
        break;
    default:
        ofBufFormat = NV_OF_BUFFER_FORMAT_UNDEFINED;
    }
    return ofBufFormat;
}


DXGI_FORMAT NvOFBufferFormatToDxgiFormat(NV_OF_BUFFER_FORMAT  ofBufFormat)
{
    DXGI_FORMAT dxgiFormat;
    switch (ofBufFormat)
    {
    case NV_OF_BUFFER_FORMAT_ABGR8 :
        dxgiFormat = DXGI_FORMAT_B8G8R8A8_UNORM;
        break;
    case NV_OF_BUFFER_FORMAT_SHORT2:
        dxgiFormat =  DXGI_FORMAT_R16G16_SINT;
        break;
    case NV_OF_BUFFER_FORMAT_SHORT:
        dxgiFormat = DXGI_FORMAT_R16_UINT;
        break;
    case NV_OF_BUFFER_FORMAT_NV12 :
        dxgiFormat = DXGI_FORMAT_NV12;
        break;
    case NV_OF_BUFFER_FORMAT_UINT:
        dxgiFormat = DXGI_FORMAT_R32_UINT;
        break;
    case NV_OF_BUFFER_FORMAT_GRAYSCALE8:
        dxgiFormat = DXGI_FORMAT_R8_UNORM;
        break;
    default:
        dxgiFormat = DXGI_FORMAT_UNKNOWN;
    }
    return dxgiFormat;
}

uint32_t GetNumberOfPlanes(DXGI_FORMAT dxgiFormat)
{
    switch (dxgiFormat)
    {
    case DXGI_FORMAT_NV12:
        return 2;
    case DXGI_FORMAT_B8G8R8A8_UNORM:
    case DXGI_FORMAT_R16G16_UINT:
    case DXGI_FORMAT_R16G16_SINT:
    case DXGI_FORMAT_R32_UINT:
    case DXGI_FORMAT_R16_UINT:
    case DXGI_FORMAT_R8_UINT:
    case DXGI_FORMAT_R8_UNORM:
        return 1;
    default:
        NVOF_THROW_ERROR("Invalid buffer format", NV_OF_ERR_UNSUPPORTED_FEATURE);
    }

    return 0;
}

