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

    // Handles introduced by extensions cannot statically link to a destroy function
    // because the loader does not export extension functions.
    template <typename HandleType>
    class UniqueHandleExt {
        using XrDestroyerFuncPtr = XrResult(XRAPI_PTR*)(HandleType);

    public:
        UniqueHandleExt() = default;
        UniqueHandleExt(const UniqueHandleExt&) = delete;
        UniqueHandleExt(UniqueHandleExt&& other) noexcept {
            *this = std::move(other);
        }

        ~UniqueHandleExt() noexcept {
            Reset();
        }

        UniqueHandleExt& operator=(const UniqueHandleExt&) = delete;
        UniqueHandleExt& operator=(UniqueHandleExt&& other) noexcept {
            if (m_handle != other.m_handle) {
                Reset();

                m_handle = other.m_handle;
                m_destroyFunction = other.m_destroyFunction;
                other.m_handle = XR_NULL_HANDLE;
                other.m_destroyFunction = nullptr;
            }
            return *this;
        }

        HandleType Get() const noexcept {
            return m_handle;
        }

        HandleType* Put(XrDestroyerFuncPtr destroyFunction) noexcept {
            Reset();
            m_destroyFunction = destroyFunction;
            return &m_handle;
        }

        void Reset(HandleType newHandle = XR_NULL_HANDLE) noexcept {
            if (m_handle != XR_NULL_HANDLE) {
                m_destroyFunction(m_handle);
            }
            m_handle = newHandle;
            m_destroyFunction = nullptr;
        }

    private:
        HandleType m_handle{XR_NULL_HANDLE};
        XrDestroyerFuncPtr m_destroyFunction{nullptr};
    };

    using ActionHandle = UniqueHandle<XrAction, xrDestroyAction>;
    using ActionSetHandle = UniqueHandle<XrActionSet, xrDestroyActionSet>;
    using InstanceHandle = UniqueHandle<XrInstance, xrDestroyInstance>;
    using SessionHandle = UniqueHandle<XrSession, xrDestroySession>;
    using SpaceHandle = UniqueHandle<XrSpace, xrDestroySpace>;
    using SwapchainHandle = UniqueHandle<XrSwapchain, xrDestroySwapchain>;
    using SpatialAnchorHandle = UniqueHandleExt<XrSpatialAnchorMSFT>;
} // namespace xr
