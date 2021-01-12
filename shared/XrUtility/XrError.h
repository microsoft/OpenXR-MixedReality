// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <memory>
#include <stdexcept>
#include "XrToString.h"

#define CHECK_XRCMD(cmd) xr::detail::_CheckXrResult(cmd, #cmd, FILE_AND_LINE)
#define CHECK_XRRESULT(res, cmdStr) xr::detail::_CheckXrResult(res, cmdStr, FILE_AND_LINE)

#define CHECK_HRCMD(cmd) xr::detail::_CheckHResult(cmd, #cmd, FILE_AND_LINE)
#define CHECK_HRESULT(res, cmdStr) xr::detail::_CheckHResult(res, cmdStr, FILE_AND_LINE)

#define DEBUG_PRINT(...) ::OutputDebugStringA((xr::detail::_Fmt(__VA_ARGS__) + "\n").c_str())

namespace xr::detail {
#define CHK_STRINGIFY(x) #x
#define TOSTRING(x) CHK_STRINGIFY(x)
#define FILE_AND_LINE __FILE__ ":" TOSTRING(__LINE__)

    inline std::string _Fmt(const char* fmt, ...) {
        va_list vl;
        va_start(vl, fmt);
        int size = std::vsnprintf(nullptr, 0, fmt, vl);
        va_end(vl);

        if (size != -1) {
            std::unique_ptr<char[]> buffer(new char[size + 1]);

            va_start(vl, fmt);
            size = std::vsnprintf(buffer.get(), size + 1, fmt, vl);
            va_end(vl);
            if (size != -1) {
                return std::string(buffer.get(), size);
            }
        }

        throw std::runtime_error("Unexpected vsnprintf failure");
    }

    [[noreturn]] inline void _Throw(std::string failureMessage, const char* originator = nullptr, const char* sourceLocation = nullptr) {
        if (originator != nullptr) {
            failureMessage += _Fmt("\n    Origin: %s", originator);
        }
        if (sourceLocation != nullptr) {
            failureMessage += _Fmt("\n    Source: %s", sourceLocation);
        }

        throw std::logic_error(failureMessage);
    }

#define THROW(msg) xr::detail::_Throw(msg, nullptr, FILE_AND_LINE)
#define CHECK(exp)                                                   \
    {                                                                \
        if (!(exp)) {                                                \
            xr::detail::_Throw("Check failed", #exp, FILE_AND_LINE); \
        }                                                            \
    }
#define CHECK_MSG(exp, msg)                               \
    {                                                     \
        if (!(exp)) {                                     \
            xr::detail::_Throw(msg, #exp, FILE_AND_LINE); \
        }                                                 \
    }

    [[noreturn]] inline void _ThrowXrResult(XrResult res, const char* originator = nullptr, const char* sourceLocation = nullptr) {
        xr::detail::_Throw(_Fmt("XrResult failure [%s]", xr::ToCString(res)), originator, sourceLocation);
    }

    inline XrResult _CheckXrResult(XrResult res, const char* originator = nullptr, const char* sourceLocation = nullptr) {
        if (XR_FAILED(res)) {
            xr::detail::_ThrowXrResult(res, originator, sourceLocation);
        }

        return res;
    }

#ifdef _WIN32
    [[noreturn]] inline void _ThrowHResult(HRESULT hr, const char* originator = nullptr, const char* sourceLocation = nullptr) {
        xr::detail::_Throw(xr::detail::_Fmt("HRESULT failure [%x]", hr), originator, sourceLocation);
    }

    inline HRESULT _CheckHResult(HRESULT hr, const char* originator = nullptr, const char* sourceLocation = nullptr) {
        if (FAILED(hr)) {
            xr::detail::_ThrowHResult(hr, originator, sourceLocation);
        }

        return hr;
    }
#endif
} // namespace xr::detail
