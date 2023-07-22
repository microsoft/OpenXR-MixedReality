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
#ifdef _WIN32
#define NOMINMAX
#include <Windows.h>
#define DIR_SEP "\\"
#else
#define HMODULE void *
#define _stricmp strcasecmp
#define DIR_SEP "/"
#endif
#include <memory>

class NvOF;
class NvOFBuffer;

/**
* @brief A managed pointer wrapper for NvOF class objects
*/
using NvOFObj = std::unique_ptr<NvOF>;

/**
* @brief A managed pointer wrapper for NvOFBuffer class objects
*/
using NvOFBufferObj = std::unique_ptr<NvOFBuffer>;
