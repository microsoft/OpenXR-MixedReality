// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <functional>
#include <openxr/openxr.h>

inline bool operator==(const XrUuidMSFT& lh, const XrUuidMSFT& rh) noexcept {
    return memcmp(&rh, &lh, sizeof(XrUuidMSFT)) == 0;
}

inline bool operator!=(const XrUuidMSFT& lh, const XrUuidMSFT& rh) noexcept {
    return !(lh == rh);
}

namespace xr {
    // Type safe UUID for ensuring that two different types of UUIDs do not get mixed.
    // Example: Mesh::Id and Plane::Id.
    template <typename Type>
    struct TypedUuid {
        TypedUuid() noexcept = default;
        explicit TypedUuid(const XrUuidMSFT& uuid) noexcept
            : m_uuid(uuid) {
        }

        TypedUuid& operator=(const XrUuidMSFT& uuid) noexcept {
            m_uuid = uuid;
            return *this;
        }

        explicit operator XrUuidMSFT() const noexcept {
            return m_uuid;
        }

        bool operator==(const TypedUuid& other) const noexcept {
            return m_uuid == other.m_uuid;
        }

        bool operator!=(const TypedUuid& other) const noexcept {
            return m_uuid != other.m_uuid;
        }

    private:
        XrUuidMSFT m_uuid;
    };
} // namespace xr

namespace std {
    // This template specialization allows XrUuidMSFT to be used as the key in a std::unordered_map.
    template <>
    struct hash<XrUuidMSFT> {
        std::size_t operator()(const XrUuidMSFT& uuid) const noexcept {
            static_assert(sizeof(XrUuidMSFT) == sizeof(uint64_t) * 2);
            const uint64_t* v = reinterpret_cast<const uint64_t*>(uuid.bytes);
            return std::hash<uint64_t>{}(v[0]) ^ (std::hash<uint64_t>{}(v[1]) << 1);
        }
    };

    // This template specialization allows TypedUuid to be used as the key in a std::unordered_map.
    template <typename T>
    struct hash<xr::TypedUuid<T>> {
        std::size_t operator()(const xr::TypedUuid<T>& uuid) const noexcept {
            return std::hash<XrUuidMSFT>{}(static_cast<XrUuidMSFT>(uuid));
        }
    };
} // namespace std
