#pragma once

#include <openxr/openxr_platform.h>
#include <openxr/openxr_reflection.h>
#include <openxr_preview/openxr_msft_preview.h>

#include <string_view>

namespace msxr {

    // Type trait which can be used for validation of extensible struct types. Does not validate position of member variables yet.
    template <typename T>
    using is_xr_struct =
        std::conjunction<std::is_same<XrStructureType, std::remove_const_t<decltype(T::type)>>,              // T::type is XrStructureType
                         std::is_same<void, std::remove_const_t<std::remove_pointer_t<decltype(T::next)>>>>; // T::next is void* or const
                                                                                                             // void*
    template <typename T>
    inline constexpr bool is_xr_struct_v = is_xr_struct<T>::value;

    using namespace std::literals::string_view_literals;

    template <typename T>
    struct XrStructTraits {
        static_assert(sizeof(T) == -1, "Must define traits for your struct");
    };

#define DECLARE_XR_STRUCT_TRAITS(Type, Enum)                                       \
    template <>                                                                    \
    struct XrStructTraits<Type> {                                                  \
        static constexpr std::string_view name{#Type##sv};                         \
        static constexpr XrStructureType type{static_cast<XrStructureType>(Enum)}; \
    };

    XR_LIST_STRUCTURE_TYPES(DECLARE_XR_STRUCT_TRAITS)
    XR_LIST_STRUCTURE_TYPES_MSFT_PREVIEW(DECLARE_XR_STRUCT_TRAITS)
} // namespace msxr

