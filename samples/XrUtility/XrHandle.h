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

namespace xr {
    template <typename HandleType, XrResult(XRAPI_PTR* DestroyFunction)(HandleType)>
    class UniqueHandle {
    public:
        UniqueHandle() = default;
        UniqueHandle(const UniqueHandle&) = delete;
        UniqueHandle(UniqueHandle&& other) noexcept {
            *this = std::move(other);
        }

        ~UniqueHandle() noexcept {
            Reset();
        }

        UniqueHandle& operator=(const UniqueHandle&) = delete;
        UniqueHandle& operator=(UniqueHandle&& other) noexcept {
            if (m_handle != other.m_handle) {
                Reset();

                m_handle = other.m_handle;
                other.m_handle = XR_NULL_HANDLE;
            }
            return *this;
        }

        HandleType Get() const noexcept {
            return m_handle;
        }

        HandleType* Put() noexcept {
            Reset();
            return &m_handle;
        }

        void Reset(HandleType newHandle = XR_NULL_HANDLE) noexcept {
            if (m_handle != XR_NULL_HANDLE) {
                DestroyFunction(m_handle);
            }
            m_handle = newHandle;
        }

    private:
        HandleType m_handle{XR_NULL_HANDLE};
    };

    using ActionHandle = UniqueHandle<XrAction, xrDestroyAction>;
    using ActionSetHandle = UniqueHandle<XrActionSet, xrDestroyActionSet>;
    using InstanceHandle = UniqueHandle<XrInstance, xrDestroyInstance>;
    using SessionHandle = UniqueHandle<XrSession, xrDestroySession>;
    using SpaceHandle = UniqueHandle<XrSpace, xrDestroySpace>;
    using SwapchainHandle = UniqueHandle<XrSwapchain, xrDestroySwapchain>;
    using SpatialAnchorHandle = UniqueHandle<XrSpatialAnchorMSFT, xrDestroySpatialAnchorMSFT>;
} // namespace xr
