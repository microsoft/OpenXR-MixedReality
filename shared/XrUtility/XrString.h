//*********************************************************
//    Copyright (c) Microsoft. All rights reserved.
//
//    Apache 2.0 License
//
//    You may obtain a copy of the License at
//    http://www.apache.org/licenses/LICENSE-2.0
//
//    Unless required by applicable law or agreed to in writing, software
//    distributed under the License is distributed on an "AS IS" BASIS,
//    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
//    implied. See the License for the specific language governing
//    permissions and limitations under the License.
//
//*********************************************************
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
