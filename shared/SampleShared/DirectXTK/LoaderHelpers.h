//--------------------------------------------------------------------------------------
// File: LoaderHelpers.h
//
// Helper functions for texture loaders and screen grabber
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
// http://go.microsoft.com/fwlink/?LinkId=248929
//--------------------------------------------------------------------------------------

#pragma once

#include "DDS.h"
#include "DDSTextureLoader.h"

// clang-format off

namespace DirectX
{

    namespace LoaderHelpers
    {
        //--------------------------------------------------------------------------------------
        // Return the BPP for a particular format
        //--------------------------------------------------------------------------------------
        inline size_t BitsPerPixel(_In_ DXGI_FORMAT fmt)
        {
            switch (fmt)
            {
            case DXGI_FORMAT_R32G32B32A32_TYPELESS:
            case DXGI_FORMAT_R32G32B32A32_FLOAT:
            case DXGI_FORMAT_R32G32B32A32_UINT:
            case DXGI_FORMAT_R32G32B32A32_SINT:
                return 128;

            case DXGI_FORMAT_R32G32B32_TYPELESS:
            case DXGI_FORMAT_R32G32B32_FLOAT:
            case DXGI_FORMAT_R32G32B32_UINT:
            case DXGI_FORMAT_R32G32B32_SINT:
                return 96;

            case DXGI_FORMAT_R16G16B16A16_TYPELESS:
            case DXGI_FORMAT_R16G16B16A16_FLOAT:
            case DXGI_FORMAT_R16G16B16A16_UNORM:
            case DXGI_FORMAT_R16G16B16A16_UINT:
            case DXGI_FORMAT_R16G16B16A16_SNORM:
            case DXGI_FORMAT_R16G16B16A16_SINT:
            case DXGI_FORMAT_R32G32_TYPELESS:
            case DXGI_FORMAT_R32G32_FLOAT:
            case DXGI_FORMAT_R32G32_UINT:
            case DXGI_FORMAT_R32G32_SINT:
            case DXGI_FORMAT_R32G8X24_TYPELESS:
            case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
            case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
            case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
            case DXGI_FORMAT_Y416:
            case DXGI_FORMAT_Y210:
            case DXGI_FORMAT_Y216:
                return 64;

            case DXGI_FORMAT_R10G10B10A2_TYPELESS:
            case DXGI_FORMAT_R10G10B10A2_UNORM:
            case DXGI_FORMAT_R10G10B10A2_UINT:
            case DXGI_FORMAT_R11G11B10_FLOAT:
            case DXGI_FORMAT_R8G8B8A8_TYPELESS:
            case DXGI_FORMAT_R8G8B8A8_UNORM:
            case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
            case DXGI_FORMAT_R8G8B8A8_UINT:
            case DXGI_FORMAT_R8G8B8A8_SNORM:
            case DXGI_FORMAT_R8G8B8A8_SINT:
            case DXGI_FORMAT_R16G16_TYPELESS:
            case DXGI_FORMAT_R16G16_FLOAT:
            case DXGI_FORMAT_R16G16_UNORM:
            case DXGI_FORMAT_R16G16_UINT:
            case DXGI_FORMAT_R16G16_SNORM:
            case DXGI_FORMAT_R16G16_SINT:
            case DXGI_FORMAT_R32_TYPELESS:
            case DXGI_FORMAT_D32_FLOAT:
            case DXGI_FORMAT_R32_FLOAT:
            case DXGI_FORMAT_R32_UINT:
            case DXGI_FORMAT_R32_SINT:
            case DXGI_FORMAT_R24G8_TYPELESS:
            case DXGI_FORMAT_D24_UNORM_S8_UINT:
            case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
            case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
            case DXGI_FORMAT_R9G9B9E5_SHAREDEXP:
            case DXGI_FORMAT_R8G8_B8G8_UNORM:
            case DXGI_FORMAT_G8R8_G8B8_UNORM:
            case DXGI_FORMAT_B8G8R8A8_UNORM:
            case DXGI_FORMAT_B8G8R8X8_UNORM:
            case DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM:
            case DXGI_FORMAT_B8G8R8A8_TYPELESS:
            case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
            case DXGI_FORMAT_B8G8R8X8_TYPELESS:
            case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
            case DXGI_FORMAT_AYUV:
            case DXGI_FORMAT_Y410:
            case DXGI_FORMAT_YUY2:
                return 32;

            case DXGI_FORMAT_P010:
            case DXGI_FORMAT_P016:
                return 24;

            case DXGI_FORMAT_R8G8_TYPELESS:
            case DXGI_FORMAT_R8G8_UNORM:
            case DXGI_FORMAT_R8G8_UINT:
            case DXGI_FORMAT_R8G8_SNORM:
            case DXGI_FORMAT_R8G8_SINT:
            case DXGI_FORMAT_R16_TYPELESS:
            case DXGI_FORMAT_R16_FLOAT:
            case DXGI_FORMAT_D16_UNORM:
            case DXGI_FORMAT_R16_UNORM:
            case DXGI_FORMAT_R16_UINT:
            case DXGI_FORMAT_R16_SNORM:
            case DXGI_FORMAT_R16_SINT:
            case DXGI_FORMAT_B5G6R5_UNORM:
            case DXGI_FORMAT_B5G5R5A1_UNORM:
            case DXGI_FORMAT_A8P8:
            case DXGI_FORMAT_B4G4R4A4_UNORM:
                return 16;

            case DXGI_FORMAT_NV12:
            case DXGI_FORMAT_420_OPAQUE:
            case DXGI_FORMAT_NV11:
                return 12;

            case DXGI_FORMAT_R8_TYPELESS:
            case DXGI_FORMAT_R8_UNORM:
            case DXGI_FORMAT_R8_UINT:
            case DXGI_FORMAT_R8_SNORM:
            case DXGI_FORMAT_R8_SINT:
            case DXGI_FORMAT_A8_UNORM:
            case DXGI_FORMAT_AI44:
            case DXGI_FORMAT_IA44:
            case DXGI_FORMAT_P8:
                return 8;

            case DXGI_FORMAT_R1_UNORM:
                return 1;

            case DXGI_FORMAT_BC1_TYPELESS:
            case DXGI_FORMAT_BC1_UNORM:
            case DXGI_FORMAT_BC1_UNORM_SRGB:
            case DXGI_FORMAT_BC4_TYPELESS:
            case DXGI_FORMAT_BC4_UNORM:
            case DXGI_FORMAT_BC4_SNORM:
                return 4;

            case DXGI_FORMAT_BC2_TYPELESS:
            case DXGI_FORMAT_BC2_UNORM:
            case DXGI_FORMAT_BC2_UNORM_SRGB:
            case DXGI_FORMAT_BC3_TYPELESS:
            case DXGI_FORMAT_BC3_UNORM:
            case DXGI_FORMAT_BC3_UNORM_SRGB:
            case DXGI_FORMAT_BC5_TYPELESS:
            case DXGI_FORMAT_BC5_UNORM:
            case DXGI_FORMAT_BC5_SNORM:
            case DXGI_FORMAT_BC6H_TYPELESS:
            case DXGI_FORMAT_BC6H_UF16:
            case DXGI_FORMAT_BC6H_SF16:
            case DXGI_FORMAT_BC7_TYPELESS:
            case DXGI_FORMAT_BC7_UNORM:
            case DXGI_FORMAT_BC7_UNORM_SRGB:
                return 8;

#if defined(_XBOX_ONE) && defined(_TITLE)

            case DXGI_FORMAT_R10G10B10_7E3_A2_FLOAT:
            case DXGI_FORMAT_R10G10B10_6E4_A2_FLOAT:
            case DXGI_FORMAT_R10G10B10_SNORM_A2_UNORM:
                return 32;

            case DXGI_FORMAT_D16_UNORM_S8_UINT:
            case DXGI_FORMAT_R16_UNORM_X8_TYPELESS:
            case DXGI_FORMAT_X16_TYPELESS_G8_UINT:
                return 24;

            case DXGI_FORMAT_R4G4_UNORM:
                return 8;

#endif // _XBOX_ONE && _TITLE

            default:
                return 0;
            }
        }

        //--------------------------------------------------------------------------------------
        inline DXGI_FORMAT MakeSRGB(_In_ DXGI_FORMAT format)
        {
            switch (format)
            {
            case DXGI_FORMAT_R8G8B8A8_UNORM:
                return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;

            case DXGI_FORMAT_BC1_UNORM:
                return DXGI_FORMAT_BC1_UNORM_SRGB;

            case DXGI_FORMAT_BC2_UNORM:
                return DXGI_FORMAT_BC2_UNORM_SRGB;

            case DXGI_FORMAT_BC3_UNORM:
                return DXGI_FORMAT_BC3_UNORM_SRGB;

            case DXGI_FORMAT_B8G8R8A8_UNORM:
                return DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;

            case DXGI_FORMAT_B8G8R8X8_UNORM:
                return DXGI_FORMAT_B8G8R8X8_UNORM_SRGB;

            case DXGI_FORMAT_BC7_UNORM:
                return DXGI_FORMAT_BC7_UNORM_SRGB;

            default:
                return format;
            }
        }

        //--------------------------------------------------------------------------------------
        inline bool IsCompressed(_In_ DXGI_FORMAT fmt)
        {
            switch (fmt)
            {
            case DXGI_FORMAT_BC1_TYPELESS:
            case DXGI_FORMAT_BC1_UNORM:
            case DXGI_FORMAT_BC1_UNORM_SRGB:
            case DXGI_FORMAT_BC2_TYPELESS:
            case DXGI_FORMAT_BC2_UNORM:
            case DXGI_FORMAT_BC2_UNORM_SRGB:
            case DXGI_FORMAT_BC3_TYPELESS:
            case DXGI_FORMAT_BC3_UNORM:
            case DXGI_FORMAT_BC3_UNORM_SRGB:
            case DXGI_FORMAT_BC4_TYPELESS:
            case DXGI_FORMAT_BC4_UNORM:
            case DXGI_FORMAT_BC4_SNORM:
            case DXGI_FORMAT_BC5_TYPELESS:
            case DXGI_FORMAT_BC5_UNORM:
            case DXGI_FORMAT_BC5_SNORM:
            case DXGI_FORMAT_BC6H_TYPELESS:
            case DXGI_FORMAT_BC6H_UF16:
            case DXGI_FORMAT_BC6H_SF16:
            case DXGI_FORMAT_BC7_TYPELESS:
            case DXGI_FORMAT_BC7_UNORM:
            case DXGI_FORMAT_BC7_UNORM_SRGB:
                return true;

            default:
                return false;
            }
        }

        //--------------------------------------------------------------------------------------
        inline DXGI_FORMAT EnsureNotTypeless(DXGI_FORMAT fmt)
        {
            // Assumes UNORM or FLOAT; doesn't use UINT or SINT
            switch (fmt)
            {
            case DXGI_FORMAT_R32G32B32A32_TYPELESS: return DXGI_FORMAT_R32G32B32A32_FLOAT;
            case DXGI_FORMAT_R32G32B32_TYPELESS:    return DXGI_FORMAT_R32G32B32_FLOAT;
            case DXGI_FORMAT_R16G16B16A16_TYPELESS: return DXGI_FORMAT_R16G16B16A16_UNORM;
            case DXGI_FORMAT_R32G32_TYPELESS:       return DXGI_FORMAT_R32G32_FLOAT;
            case DXGI_FORMAT_R10G10B10A2_TYPELESS:  return DXGI_FORMAT_R10G10B10A2_UNORM;
            case DXGI_FORMAT_R8G8B8A8_TYPELESS:     return DXGI_FORMAT_R8G8B8A8_UNORM;
            case DXGI_FORMAT_R16G16_TYPELESS:       return DXGI_FORMAT_R16G16_UNORM;
            case DXGI_FORMAT_R32_TYPELESS:          return DXGI_FORMAT_R32_FLOAT;
            case DXGI_FORMAT_R8G8_TYPELESS:         return DXGI_FORMAT_R8G8_UNORM;
            case DXGI_FORMAT_R16_TYPELESS:          return DXGI_FORMAT_R16_UNORM;
            case DXGI_FORMAT_R8_TYPELESS:           return DXGI_FORMAT_R8_UNORM;
            case DXGI_FORMAT_BC1_TYPELESS:          return DXGI_FORMAT_BC1_UNORM;
            case DXGI_FORMAT_BC2_TYPELESS:          return DXGI_FORMAT_BC2_UNORM;
            case DXGI_FORMAT_BC3_TYPELESS:          return DXGI_FORMAT_BC3_UNORM;
            case DXGI_FORMAT_BC4_TYPELESS:          return DXGI_FORMAT_BC4_UNORM;
            case DXGI_FORMAT_BC5_TYPELESS:          return DXGI_FORMAT_BC5_UNORM;
            case DXGI_FORMAT_B8G8R8A8_TYPELESS:     return DXGI_FORMAT_B8G8R8A8_UNORM;
            case DXGI_FORMAT_B8G8R8X8_TYPELESS:     return DXGI_FORMAT_B8G8R8X8_UNORM;
            case DXGI_FORMAT_BC7_TYPELESS:          return DXGI_FORMAT_BC7_UNORM;
            default:                                return fmt;
            }
        }

        //--------------------------------------------------------------------------------------
        inline HRESULT LoadTextureDataFromFile(_In_z_ const wchar_t* fileName,
            std::unique_ptr<uint8_t[]>& ddsData,
            const DDS_HEADER** header,
            const uint8_t** bitData,
            size_t* bitSize
        )
        {
            if (!header || !bitData || !bitSize)
            {
                return E_POINTER;
            }

            // open the file
#if (_WIN32_WINNT >= _WIN32_WINNT_WIN8)
            ScopedHandle hFile(safe_handle(CreateFile2(fileName,
                GENERIC_READ,
                FILE_SHARE_READ,
                OPEN_EXISTING,
                nullptr)));
#else
            ScopedHandle hFile(safe_handle(CreateFileW(fileName,
                GENERIC_READ,
                FILE_SHARE_READ,
                nullptr,
                OPEN_EXISTING,
                FILE_ATTRIBUTE_NORMAL,
                nullptr)));
#endif

            if (!hFile)
            {
                return HRESULT_FROM_WIN32(GetLastError());
            }

            // Get the file size
            FILE_STANDARD_INFO fileInfo;
            if (!GetFileInformationByHandleEx(hFile.get(), FileStandardInfo, &fileInfo, sizeof(fileInfo)))
            {
                return HRESULT_FROM_WIN32(GetLastError());
            }

            // File is too big for 32-bit allocation, so reject read
            if (fileInfo.EndOfFile.HighPart > 0)
            {
                return E_FAIL;
            }

            // Need at least enough data to fill the header and magic number to be a valid DDS
            if (fileInfo.EndOfFile.LowPart < (sizeof(DDS_HEADER) + sizeof(uint32_t)))
            {
                return E_FAIL;
            }

            // create enough space for the file data
            ddsData.reset(new (std::nothrow) uint8_t[fileInfo.EndOfFile.LowPart]);
            if (!ddsData)
            {
                return E_OUTOFMEMORY;
            }

            // read the data in
            DWORD BytesRead = 0;
            if (!ReadFile(hFile.get(),
                ddsData.get(),
                fileInfo.EndOfFile.LowPart,
                &BytesRead,
                nullptr
            ))
            {
                return HRESULT_FROM_WIN32(GetLastError());
            }

            if (BytesRead < fileInfo.EndOfFile.LowPart)
            {
                return E_FAIL;
            }

            // DDS files always start with the same magic number ("DDS ")
            uint32_t dwMagicNumber = *reinterpret_cast<const uint32_t*>(ddsData.get());
            if (dwMagicNumber != DDS_MAGIC)
            {
                return E_FAIL;
            }

            auto hdr = reinterpret_cast<const DDS_HEADER*>(ddsData.get() + sizeof(uint32_t));

            // Verify header to validate DDS file
            if (hdr->size != sizeof(DDS_HEADER) ||
                hdr->ddspf.size != sizeof(DDS_PIXELFORMAT))
            {
                return E_FAIL;
            }

            // Check for DX10 extension
            bool bDXT10Header = false;
            if ((hdr->ddspf.flags & DDS_FOURCC) &&
                (MAKEFOURCC('D', 'X', '1', '0') == hdr->ddspf.fourCC))
            {
                // Must be long enough for both headers and magic value
                if (fileInfo.EndOfFile.LowPart < (sizeof(DDS_HEADER) + sizeof(uint32_t) + sizeof(DDS_HEADER_DXT10)))
                {
                    return E_FAIL;
                }

                bDXT10Header = true;
            }

            // setup the pointers in the process request
            *header = hdr;
            ptrdiff_t offset = sizeof(uint32_t) + sizeof(DDS_HEADER)
                + (bDXT10Header ? sizeof(DDS_HEADER_DXT10) : 0);
            *bitData = ddsData.get() + offset;
            *bitSize = fileInfo.EndOfFile.LowPart - offset;

            return S_OK;
        }

        //--------------------------------------------------------------------------------------
        // Get surface information for a particular format
        //--------------------------------------------------------------------------------------
        inline void GetSurfaceInfo(_In_ size_t width,
            _In_ size_t height,
            _In_ DXGI_FORMAT fmt,
            _Out_opt_ size_t* outNumBytes,
            _Out_opt_ size_t* outRowBytes,
            _Out_opt_ size_t* outNumRows)
        {
            size_t numBytes = 0;
            size_t rowBytes = 0;
            size_t numRows = 0;

            bool bc = false;
            bool packed = false;
            bool planar = false;
            size_t bpe = 0;
            switch (fmt)
            {
            case DXGI_FORMAT_BC1_TYPELESS:
            case DXGI_FORMAT_BC1_UNORM:
            case DXGI_FORMAT_BC1_UNORM_SRGB:
            case DXGI_FORMAT_BC4_TYPELESS:
            case DXGI_FORMAT_BC4_UNORM:
            case DXGI_FORMAT_BC4_SNORM:
                bc = true;
                bpe = 8;
                break;

            case DXGI_FORMAT_BC2_TYPELESS:
            case DXGI_FORMAT_BC2_UNORM:
            case DXGI_FORMAT_BC2_UNORM_SRGB:
            case DXGI_FORMAT_BC3_TYPELESS:
            case DXGI_FORMAT_BC3_UNORM:
            case DXGI_FORMAT_BC3_UNORM_SRGB:
            case DXGI_FORMAT_BC5_TYPELESS:
            case DXGI_FORMAT_BC5_UNORM:
            case DXGI_FORMAT_BC5_SNORM:
            case DXGI_FORMAT_BC6H_TYPELESS:
            case DXGI_FORMAT_BC6H_UF16:
            case DXGI_FORMAT_BC6H_SF16:
            case DXGI_FORMAT_BC7_TYPELESS:
            case DXGI_FORMAT_BC7_UNORM:
            case DXGI_FORMAT_BC7_UNORM_SRGB:
                bc = true;
                bpe = 16;
                break;

            case DXGI_FORMAT_R8G8_B8G8_UNORM:
            case DXGI_FORMAT_G8R8_G8B8_UNORM:
            case DXGI_FORMAT_YUY2:
                packed = true;
                bpe = 4;
                break;

            case DXGI_FORMAT_Y210:
            case DXGI_FORMAT_Y216:
                packed = true;
                bpe = 8;
                break;

            case DXGI_FORMAT_NV12:
            case DXGI_FORMAT_420_OPAQUE:
                planar = true;
                bpe = 2;
                break;

            case DXGI_FORMAT_P010:
            case DXGI_FORMAT_P016:
                planar = true;
                bpe = 4;
                break;

#if defined(_XBOX_ONE) && defined(_TITLE)

            case DXGI_FORMAT_D16_UNORM_S8_UINT:
            case DXGI_FORMAT_R16_UNORM_X8_TYPELESS:
            case DXGI_FORMAT_X16_TYPELESS_G8_UINT:
                planar = true;
                bpe = 4;
                break;

#endif

            default:
                break;
            }

            if (bc)
            {
                size_t numBlocksWide = 0;
                if (width > 0)
                {
                    numBlocksWide = std::max<size_t>(1, (width + 3) / 4);
                }
                size_t numBlocksHigh = 0;
                if (height > 0)
                {
                    numBlocksHigh = std::max<size_t>(1, (height + 3) / 4);
                }
                rowBytes = numBlocksWide * bpe;
                numRows = numBlocksHigh;
                numBytes = rowBytes * numBlocksHigh;
            }
            else if (packed)
            {
                rowBytes = ((width + 1) >> 1) * bpe;
                numRows = height;
                numBytes = rowBytes * height;
            }
            else if (fmt == DXGI_FORMAT_NV11)
            {
                rowBytes = ((width + 3) >> 2) * 4;
                numRows = height * 2; // Direct3D makes this simplifying assumption, although it is larger than the 4:1:1 data
                numBytes = rowBytes * numRows;
            }
            else if (planar)
            {
                rowBytes = ((width + 1) >> 1) * bpe;
                numBytes = (rowBytes * height) + ((rowBytes * height + 1) >> 1);
                numRows = height + ((height + 1) >> 1);
            }
            else
            {
                size_t bpp = BitsPerPixel(fmt);
                rowBytes = (width * bpp + 7) / 8; // round up to nearest byte
                numRows = height;
                numBytes = rowBytes * height;
            }

            if (outNumBytes)
            {
                *outNumBytes = numBytes;
            }
            if (outRowBytes)
            {
                *outRowBytes = rowBytes;
            }
            if (outNumRows)
            {
                *outNumRows = numRows;
            }
        }

        //--------------------------------------------------------------------------------------
#define ISBITMASK( r,g,b,a ) ( ddpf.RBitMask == r && ddpf.GBitMask == g && ddpf.BBitMask == b && ddpf.ABitMask == a )

        inline DXGI_FORMAT GetDXGIFormat(const DDS_PIXELFORMAT& ddpf)
        {
            if (ddpf.flags & DDS_RGB)
            {
                // Note that sRGB formats are written using the "DX10" extended header

                switch (ddpf.RGBBitCount)
                {
                case 32:
                    if (ISBITMASK(0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000))
                    {
                        return DXGI_FORMAT_R8G8B8A8_UNORM;
                    }

                    if (ISBITMASK(0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000))
                    {
                        return DXGI_FORMAT_B8G8R8A8_UNORM;
                    }

                    if (ISBITMASK(0x00ff0000, 0x0000ff00, 0x000000ff, 0x00000000))
                    {
                        return DXGI_FORMAT_B8G8R8X8_UNORM;
                    }

                    // No DXGI format maps to ISBITMASK(0x000000ff,0x0000ff00,0x00ff0000,0x00000000) aka D3DFMT_X8B8G8R8

                    // Note that many common DDS reader/writers (including D3DX) swap the
                    // the RED/BLUE masks for 10:10:10:2 formats. We assume
                    // below that the 'backwards' header mask is being used since it is most
                    // likely written by D3DX. The more robust solution is to use the 'DX10'
                    // header extension and specify the DXGI_FORMAT_R10G10B10A2_UNORM format directly

                    // For 'correct' writers, this should be 0x000003ff,0x000ffc00,0x3ff00000 for RGB data
                    if (ISBITMASK(0x3ff00000, 0x000ffc00, 0x000003ff, 0xc0000000))
                    {
                        return DXGI_FORMAT_R10G10B10A2_UNORM;
                    }

                    // No DXGI format maps to ISBITMASK(0x000003ff,0x000ffc00,0x3ff00000,0xc0000000) aka D3DFMT_A2R10G10B10

                    if (ISBITMASK(0x0000ffff, 0xffff0000, 0x00000000, 0x00000000))
                    {
                        return DXGI_FORMAT_R16G16_UNORM;
                    }

                    if (ISBITMASK(0xffffffff, 0x00000000, 0x00000000, 0x00000000))
                    {
                        // Only 32-bit color channel format in D3D9 was R32F
                        return DXGI_FORMAT_R32_FLOAT; // D3DX writes this out as a FourCC of 114
                    }
                    break;

                case 24:
                    // No 24bpp DXGI formats aka D3DFMT_R8G8B8
                    break;

                case 16:
                    if (ISBITMASK(0x7c00, 0x03e0, 0x001f, 0x8000))
                    {
                        return DXGI_FORMAT_B5G5R5A1_UNORM;
                    }
                    if (ISBITMASK(0xf800, 0x07e0, 0x001f, 0x0000))
                    {
                        return DXGI_FORMAT_B5G6R5_UNORM;
                    }

                    // No DXGI format maps to ISBITMASK(0x7c00,0x03e0,0x001f,0x0000) aka D3DFMT_X1R5G5B5

                    if (ISBITMASK(0x0f00, 0x00f0, 0x000f, 0xf000))
                    {
                        return DXGI_FORMAT_B4G4R4A4_UNORM;
                    }

                    // No DXGI format maps to ISBITMASK(0x0f00,0x00f0,0x000f,0x0000) aka D3DFMT_X4R4G4B4

                    // No 3:3:2, 3:3:2:8, or paletted DXGI formats aka D3DFMT_A8R3G3B2, D3DFMT_R3G3B2, D3DFMT_P8, D3DFMT_A8P8, etc.
                    break;
                }
            }
            else if (ddpf.flags & DDS_LUMINANCE)
            {
                if (8 == ddpf.RGBBitCount)
                {
                    if (ISBITMASK(0x000000ff, 0x00000000, 0x00000000, 0x00000000))
                    {
                        return DXGI_FORMAT_R8_UNORM; // D3DX10/11 writes this out as DX10 extension
                    }

                    // No DXGI format maps to ISBITMASK(0x0f,0x00,0x00,0xf0) aka D3DFMT_A4L4

                    if (ISBITMASK(0x000000ff, 0x00000000, 0x00000000, 0x0000ff00))
                    {
                        return DXGI_FORMAT_R8G8_UNORM; // Some DDS writers assume the bitcount should be 8 instead of 16
                    }
                }

                if (16 == ddpf.RGBBitCount)
                {
                    if (ISBITMASK(0x0000ffff, 0x00000000, 0x00000000, 0x00000000))
                    {
                        return DXGI_FORMAT_R16_UNORM; // D3DX10/11 writes this out as DX10 extension
                    }
                    if (ISBITMASK(0x000000ff, 0x00000000, 0x00000000, 0x0000ff00))
                    {
                        return DXGI_FORMAT_R8G8_UNORM; // D3DX10/11 writes this out as DX10 extension
                    }
                }
            }
            else if (ddpf.flags & DDS_ALPHA)
            {
                if (8 == ddpf.RGBBitCount)
                {
                    return DXGI_FORMAT_A8_UNORM;
                }
            }
            else if (ddpf.flags & DDS_BUMPDUDV)
            {
                if (16 == ddpf.RGBBitCount)
                {
                    if (ISBITMASK(0x00ff, 0xff00, 0x0000, 0x0000))
                    {
                        return DXGI_FORMAT_R8G8_SNORM; // D3DX10/11 writes this out as DX10 extension
                    }
                }

                if (32 == ddpf.RGBBitCount)
                {
                    if (ISBITMASK(0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000))
                    {
                        return DXGI_FORMAT_R8G8B8A8_SNORM; // D3DX10/11 writes this out as DX10 extension
                    }
                    if (ISBITMASK(0x0000ffff, 0xffff0000, 0x00000000, 0x00000000))
                    {
                        return DXGI_FORMAT_R16G16_SNORM; // D3DX10/11 writes this out as DX10 extension
                    }

                    // No DXGI format maps to ISBITMASK(0x3ff00000, 0x000ffc00, 0x000003ff, 0xc0000000) aka D3DFMT_A2W10V10U10
                }
            }
            else if (ddpf.flags & DDS_FOURCC)
            {
                if (MAKEFOURCC('D', 'X', 'T', '1') == ddpf.fourCC)
                {
                    return DXGI_FORMAT_BC1_UNORM;
                }
                if (MAKEFOURCC('D', 'X', 'T', '3') == ddpf.fourCC)
                {
                    return DXGI_FORMAT_BC2_UNORM;
                }
                if (MAKEFOURCC('D', 'X', 'T', '5') == ddpf.fourCC)
                {
                    return DXGI_FORMAT_BC3_UNORM;
                }

                // While pre-multiplied alpha isn't directly supported by the DXGI formats,
                // they are basically the same as these BC formats so they can be mapped
                if (MAKEFOURCC('D', 'X', 'T', '2') == ddpf.fourCC)
                {
                    return DXGI_FORMAT_BC2_UNORM;
                }
                if (MAKEFOURCC('D', 'X', 'T', '4') == ddpf.fourCC)
                {
                    return DXGI_FORMAT_BC3_UNORM;
                }

                if (MAKEFOURCC('A', 'T', 'I', '1') == ddpf.fourCC)
                {
                    return DXGI_FORMAT_BC4_UNORM;
                }
                if (MAKEFOURCC('B', 'C', '4', 'U') == ddpf.fourCC)
                {
                    return DXGI_FORMAT_BC4_UNORM;
                }
                if (MAKEFOURCC('B', 'C', '4', 'S') == ddpf.fourCC)
                {
                    return DXGI_FORMAT_BC4_SNORM;
                }

                if (MAKEFOURCC('A', 'T', 'I', '2') == ddpf.fourCC)
                {
                    return DXGI_FORMAT_BC5_UNORM;
                }
                if (MAKEFOURCC('B', 'C', '5', 'U') == ddpf.fourCC)
                {
                    return DXGI_FORMAT_BC5_UNORM;
                }
                if (MAKEFOURCC('B', 'C', '5', 'S') == ddpf.fourCC)
                {
                    return DXGI_FORMAT_BC5_SNORM;
                }

                // BC6H and BC7 are written using the "DX10" extended header

                if (MAKEFOURCC('R', 'G', 'B', 'G') == ddpf.fourCC)
                {
                    return DXGI_FORMAT_R8G8_B8G8_UNORM;
                }
                if (MAKEFOURCC('G', 'R', 'G', 'B') == ddpf.fourCC)
                {
                    return DXGI_FORMAT_G8R8_G8B8_UNORM;
                }

                if (MAKEFOURCC('Y', 'U', 'Y', '2') == ddpf.fourCC)
                {
                    return DXGI_FORMAT_YUY2;
                }

                // Check for D3DFORMAT enums being set here
                switch (ddpf.fourCC)
                {
                case 36: // D3DFMT_A16B16G16R16
                    return DXGI_FORMAT_R16G16B16A16_UNORM;

                case 110: // D3DFMT_Q16W16V16U16
                    return DXGI_FORMAT_R16G16B16A16_SNORM;

                case 111: // D3DFMT_R16F
                    return DXGI_FORMAT_R16_FLOAT;

                case 112: // D3DFMT_G16R16F
                    return DXGI_FORMAT_R16G16_FLOAT;

                case 113: // D3DFMT_A16B16G16R16F
                    return DXGI_FORMAT_R16G16B16A16_FLOAT;

                case 114: // D3DFMT_R32F
                    return DXGI_FORMAT_R32_FLOAT;

                case 115: // D3DFMT_G32R32F
                    return DXGI_FORMAT_R32G32_FLOAT;

                case 116: // D3DFMT_A32B32G32R32F
                    return DXGI_FORMAT_R32G32B32A32_FLOAT;
                }
            }

            return DXGI_FORMAT_UNKNOWN;
        }

#undef ISBITMASK

        //--------------------------------------------------------------------------------------
        inline DirectX::DDS_ALPHA_MODE GetAlphaMode(_In_ const DDS_HEADER* header)
        {
            if (header->ddspf.flags & DDS_FOURCC)
            {
                if (MAKEFOURCC('D', 'X', '1', '0') == header->ddspf.fourCC)
                {
                    auto d3d10ext = reinterpret_cast<const DDS_HEADER_DXT10*>((const char*)header + sizeof(DDS_HEADER));
                    auto mode = static_cast<DDS_ALPHA_MODE>(d3d10ext->miscFlags2 & DDS_MISC_FLAGS2_ALPHA_MODE_MASK);
                    switch (mode)
                    {
                    case DDS_ALPHA_MODE_STRAIGHT:
                    case DDS_ALPHA_MODE_PREMULTIPLIED:
                    case DDS_ALPHA_MODE_OPAQUE:
                    case DDS_ALPHA_MODE_CUSTOM:
                        return mode;

                    default:
                        break;
                    }
                }
                else if ((MAKEFOURCC('D', 'X', 'T', '2') == header->ddspf.fourCC)
                    || (MAKEFOURCC('D', 'X', 'T', '4') == header->ddspf.fourCC))
                {
                    return DDS_ALPHA_MODE_PREMULTIPLIED;
                }
            }

            return DDS_ALPHA_MODE_UNKNOWN;
        }

        //--------------------------------------------------------------------------------------
        class auto_delete_file
        {
        public:
            auto_delete_file(HANDLE hFile) : m_handle(hFile) {}

            auto_delete_file(const auto_delete_file&) = delete;
            auto_delete_file& operator=(const auto_delete_file&) = delete;

            ~auto_delete_file()
            {
                if (m_handle)
                {
                    FILE_DISPOSITION_INFO info = {};
                    info.DeleteFile = TRUE;
                    (void)SetFileInformationByHandle(m_handle, FileDispositionInfo, &info, sizeof(info));
                }
            }

            void clear() { m_handle = 0; }

        private:
            HANDLE m_handle;
        };
    }
}
