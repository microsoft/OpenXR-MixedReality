#pragma once

#include <openxr/openxr_reflection.h>
#include <openxr_preview/openxr_msft_preview.h>

namespace xr {

    // Struct of enabled flags is generated using openxr_reflection. Any non-public extensions will need to be added manually.
    struct EnabledExtensions {
        EnabledExtensions() = default;
        EnabledExtensions(const std::vector<const char*>& enabledExtensions, const std::vector<XrExtensionProperties>& extensionProperties)
            : EnabledExtensions(enabledExtensions.data(), (uint32_t)enabledExtensions.size(), extensionProperties) {
        }
        EnabledExtensions(const char* const* enabledExtensionNames,
                          uint32_t enabledExtensionNamesCount,
                          const std::vector<XrExtensionProperties>& extensionProperties) {
            auto getExtensionVersion = [&](const char* extensionName) -> uint32_t {
                for (const auto& extension : extensionProperties) {
                    if (strcmp(extension.extensionName, extensionName) == 0) {
                        return extension.extensionVersion;
                    }
                }
                return 0;
            };

            // "_enabled" or "_version" is token concat to the name because the extension name is also a C MACRO that can conflict.
#define SET_EXTENSION_IF_MATCH(name, _)                 \
    if (strcmp(enabledExtensionNames[i], #name) == 0) { \
        name##_enabled = true;                          \
        name##_version = getExtensionVersion(#name);    \
        continue;                                       \
    }

            for (uint32_t i = 0; i < enabledExtensionNamesCount; i++) {
                XR_LIST_EXTENSIONS(SET_EXTENSION_IF_MATCH)
                XR_LIST_EXTENSIONS_MSFT_PREVIEW(SET_EXTENSION_IF_MATCH)
            }

#undef SET_EXTENSION_IF_MATCH
        }

        // Example: for extension name `XR_KHR_extension_name`, there will be two fields define for it.
        //  bool   XR_KHR_extension_name_enabled; // it will be true if the extension is enabled by the current xrInstance
        //  uint32 XR_KHR_extension_name_version; // it will be > 0 if the extension is enabled by the current xrInstance.
#define DEFINE_EXTENSION_FIELD(name, _) \
    bool name##_enabled = false;        \
    uint32_t name##_version = 0;

        XR_LIST_EXTENSIONS(DEFINE_EXTENSION_FIELD)
        XR_LIST_EXTENSIONS_MSFT_PREVIEW(DEFINE_EXTENSION_FIELD)

#undef MSXR_ADD_EXTENSION_FIELD
    };

} // namespace xr

