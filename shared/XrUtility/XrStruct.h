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

#include <string_view>
#include <openxr/openxr.h>

namespace xr {
    struct NameVersion {
        NameVersion(std::string name, uint32_t version)
            : Name(std::move(name))
            , Version(version) {
        }
        std::string Name;
        uint32_t Version;
    };

    inline void SetApplicationInfo(XrApplicationInfo& appInfo,
                                   const xr::NameVersion& appNameVersion,
                                   const xr::NameVersion& engineNameVersion,
                                   XrVersion apiVersion = XR_CURRENT_API_VERSION) {
        strncpy_s(appInfo.applicationName, XR_MAX_APPLICATION_NAME_SIZE, appNameVersion.Name.data(), appNameVersion.Name.size());
        appInfo.applicationVersion = appNameVersion.Version;
        strncpy_s(appInfo.engineName, XR_MAX_ENGINE_NAME_SIZE, engineNameVersion.Name.data(), engineNameVersion.Name.size());
        appInfo.engineVersion = engineNameVersion.Version;
        appInfo.apiVersion = apiVersion;
    }

    // extensions can be const or non-const.
    template <typename T>
    void SetEnabledExtensions(XrInstanceCreateInfo& info, T& extensions) {
        info.enabledExtensionCount = static_cast<uint32_t>(std::size(extensions));
        info.enabledExtensionNames = std::data(extensions);
    }

    // Don't allow rvalues to be passed because the memory that enabledExtensionNames points to
    // needs to live longer than this function.
    template <typename T>
    void SetEnabledExtensions(XrInstanceCreateInfo& info, T&& extensions) = delete;

} // namespace xr
