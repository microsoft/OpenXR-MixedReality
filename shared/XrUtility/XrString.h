// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <XrUtility/XrPlatformDependencies.h>
#include <openxr/openxr.h>
#include <openxr/openxr_platform.h>
#include <openxr/openxr_reflection.h>

#include <string>
#include <vector>

#include "XrToString.h"
#include "XrError.h"

namespace xr {

    inline XrPath StringToPath(XrInstance instance, const char* str) {
        XrPath path;
        CHECK_XRCMD(xrStringToPath(instance, str, &path));
        return path;
    }

    inline std::string PathToString(XrInstance instance, XrPath path) {
        uint32_t count;
        CHECK_XRCMD(xrPathToString(instance, path, 0, &count, nullptr));
        std::string string;
        string.resize(count);
        CHECK_XRCMD(xrPathToString(instance, path, count, &count, string.data()));
        return string;
    }

    inline std::vector<XrPath> StringsToPaths(XrInstance instance, const std::vector<std::string>& strings) {
        std::vector<XrPath> paths;

        for (auto string : strings) {
            paths.push_back(xr::StringToPath(instance, string.c_str()));
        }

        return paths;
    }

#ifdef _WIN32
    inline std::wstring utf8_to_wide(std::string_view utf8Text) {
        if (utf8Text.empty()) {
            return {};
        }

        std::wstring wideText;
        const int wideLength = ::MultiByteToWideChar(CP_UTF8, 0, utf8Text.data(), (int)utf8Text.size(), nullptr, 0);
        if (wideLength == 0) {
            DEBUG_PRINT("utf8_to_wide get size error.");
            return {};
        }

        // MultiByteToWideChar returns number of chars of the input buffer, regardless of null terminitor
        wideText.resize(wideLength, 0);
        const int length = ::MultiByteToWideChar(CP_UTF8, 0, utf8Text.data(), (int)utf8Text.size(), wideText.data(), wideLength);
        if (length != wideLength) {
            DEBUG_PRINT("utf8_to_wide convert string error.");
            return {};
        }

        return wideText;
    }

    inline std::string wide_to_utf8(std::wstring_view wideText) {
        if (wideText.empty()) {
            return {};
        }

        std::string narrowText;
        int narrowLength = ::WideCharToMultiByte(CP_UTF8, 0, wideText.data(), (int)wideText.size(), nullptr, 0, nullptr, nullptr);
        if (narrowLength == 0) {
            DEBUG_PRINT("wide_to_utf8 get size error.");
            return {};
        }

        // WideCharToMultiByte returns number of chars of the input buffer, regardless of null terminitor
        narrowText.resize(narrowLength, 0);
        const int length =
            ::WideCharToMultiByte(CP_UTF8, 0, wideText.data(), (int)wideText.size(), narrowText.data(), narrowLength, nullptr, nullptr);
        if (length != narrowLength) {
            DEBUG_PRINT("wide_to_utf8 convert string error.");
            return {};
        }

        return narrowText;
    }
#endif

} // namespace xr
