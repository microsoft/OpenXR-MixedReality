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

#include <wrl.h>
#include <dxgiformat.h>
#include "NvOF.h"
#include "nvOpticalFlowCommon.h"

class DXException : public std::exception
{
public:
    DXException(HRESULT hr) : result(hr) {}

    virtual const char* what() const override
    {
        static char s_str[64] = {};
        sprintf_s(s_str, "Failure with HRESULT of %08X", static_cast<unsigned int>(result));
        return s_str;
    }

private:
    HRESULT result;
};

#define D3D_API_CALL(dxAPI)                           \
    do                                                \
    {                                                 \
        HRESULT hr = dxAPI;                           \
        if (FAILED(hr))                               \
        {                                             \
            throw DXException(hr);                    \
        }                                             \
    } while (0)

DXGI_FORMAT NvOFBufferFormatToDxgiFormat(NV_OF_BUFFER_FORMAT  ofBufFormat);

NV_OF_BUFFER_FORMAT DXGIFormatToNvOFBufferFormat(DXGI_FORMAT dxgiFormat);

uint32_t GetNumberOfPlanes(DXGI_FORMAT dxgiFormat);
