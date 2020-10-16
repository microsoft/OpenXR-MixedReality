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

#include <memory.h>
#include <functional>
#include <string.h>

namespace xr {
    template <typename GUID1, typename GUID2>
    void CopyGuid(GUID1& dest, const GUID2& src) {
        static_assert(sizeof(GUID1) == sizeof(GUID2), "GUID sizes must be equal");
#ifdef _MSC_VER
        memcpy_s(&dest, sizeof(GUID1), &src, sizeof(GUID2));
#else
        memcpy(&dest, &src, sizeof(GUID2));
#endif
    }

    template <typename GUID1, typename GUID2>
    const GUID1& CastGuid(const GUID2& src) {
        static_assert(sizeof(GUID1) == sizeof(GUID2), "GUID sizes must be equal");
        return *reinterpret_cast<const GUID1*>(&src);
    }

    struct XrGuid {
        const uint8_t* Data() const {
            return m_data.data();
        }

        bool operator==(const XrGuid& other) const {
            return m_data == other.m_data;
        }

    private:
        std::array<uint8_t, 16> m_data{};
    };

} // namespace xr

namespace std {
    // This template specialization allows XrGuid to be used as the key in a std::unordered_map.
    template <>
    struct hash<xr::XrGuid> {
        std::size_t operator()(const xr::XrGuid& guid) const {
            static_assert(sizeof(guid) == sizeof(uint64_t) * 2);
            const uint64_t* v = reinterpret_cast<const uint64_t*>(guid.Data());
            if constexpr (sizeof(std::size_t) == 4) {
                return std::hash<uint64_t>{}(v[0] ^ v[1]);
            }
            return static_cast<std::size_t>(v[0] ^ v[1]);
        }
    };
} // namespace std
