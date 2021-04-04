// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

namespace xr::Side {
    struct side_t {
        constexpr side_t() = default;

        constexpr side_t(side_t&&) = default;
        constexpr side_t& operator=(side_t&&) = default;

        constexpr side_t(const side_t&) = default;
        constexpr side_t& operator=(const side_t&) = default;

        explicit constexpr side_t(uint32_t i) noexcept
            : m_value(i) {
        }
        constexpr operator uint32_t() const noexcept {
            return m_value;
        }
    private:
        uint32_t m_value{};
    };

    constexpr side_t Left{0};
    constexpr side_t Right{1};
    constexpr side_t Count{2};

    constexpr const char* Name[Count] = {"Left", "Right"};
    constexpr const char* UserPath[Count] = {"/user/hand/left", "/user/hand/right"};

    template<typename T>
    const T& Select(side_t side, const T& left, const T& right) {
        assert(side == Left || side == Right);
        return side == Left ? left : right;
    }
} // namespace xr::Side
