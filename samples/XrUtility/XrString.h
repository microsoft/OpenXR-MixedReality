#pragma once

#include <openxr/openxr.h>
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

    inline std::vector<XrPath> StringsToPaths(XrInstance instance, const std::vector<std::string>& strings) {
        std::vector<XrPath> paths;

        for (auto string : strings) {
            paths.push_back(StringToPath(instance, string.c_str()));
        }

        return paths;
    }
} // namespace xr
