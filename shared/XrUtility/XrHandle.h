// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

namespace xr {

    template <typename HandleType>
    class UniqueXrHandle {
        using PFN_DestroyFunction = XrResult(XRAPI_PTR*)(HandleType);

    public:
        UniqueXrHandle() = default;
        UniqueXrHandle(const UniqueXrHandle&) = delete;
        UniqueXrHandle(UniqueXrHandle&& other) noexcept {
            *this = std::move(other);
        }

        ~UniqueXrHandle() noexcept {
            Reset();
        }

        UniqueXrHandle& operator=(const UniqueXrHandle&) = delete;
        UniqueXrHandle& operator=(UniqueXrHandle&& other) noexcept {
            if (m_handle != other.m_handle || m_destroyer != other.m_destroyer) {
                Reset();

                m_handle = other.m_handle;
                m_destroyer = other.m_destroyer;

                other.m_handle = XR_NULL_HANDLE;
                other.m_destroyer = nullptr;
            }
            return *this;
        }

        bool operator==(const UniqueXrHandle& other) noexcept {
            return m_handle == other.m_handle;
        }

        bool operator!=(const UniqueXrHandle& other) noexcept {
            return m_handle != other.m_handle;
        }

        explicit operator bool() const noexcept {
            return m_handle != XR_NULL_HANDLE;
        }

        HandleType Get() const noexcept {
            return m_handle;
        }

        // Extension functions cannot be statically linked, so the creator must pass in the destroy function.
        HandleType* Put(PFN_DestroyFunction destroyFunction) noexcept {
            assert(destroyFunction != nullptr);
            Reset();
            m_destroyer = destroyFunction;
            return &m_handle;
        }

        void Reset() noexcept {
            if (m_handle != XR_NULL_HANDLE) {
                m_destroyer(m_handle);
                m_handle = XR_NULL_HANDLE;
            }

            m_destroyer = nullptr;
        }

    private:
        HandleType m_handle{XR_NULL_HANDLE};
        PFN_DestroyFunction m_destroyer{nullptr};
    };

    class ActionHandle : public UniqueXrHandle<XrAction> {};
    class ActionSetHandle : public UniqueXrHandle<XrActionSet> {};
    class InstanceHandle : public UniqueXrHandle<XrInstance> {};
    class SessionHandle : public UniqueXrHandle<XrSession> {};
    class SpaceHandle : public UniqueXrHandle<XrSpace> {};
    class SwapchainHandle : public UniqueXrHandle<XrSwapchain> {};
    class SpatialAnchorHandle : public UniqueXrHandle<XrSpatialAnchorMSFT> {};
    class HandTrackerHandle : public UniqueXrHandle<XrHandTrackerEXT> {};

} // namespace xr
