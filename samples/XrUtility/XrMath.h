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
#include <DirectXMath.h>

namespace xr::math {
    constexpr float QuaternionEpsilon = 0.01f;
    constexpr DirectX::XMVECTORF32 XMQuaternionEpsilon = {{{QuaternionEpsilon, QuaternionEpsilon, QuaternionEpsilon, QuaternionEpsilon}}};

    namespace Pose {
        constexpr XrPosef Identity();
        constexpr XrPosef Translation(const XrVector3f& translation);

        constexpr bool IsPoseValid(const XrSpaceLocation& location);
        constexpr bool IsPoseTracked(const XrSpaceLocation& location);
        constexpr bool IsPoseValid(const XrViewState& viewState);
        constexpr bool IsPoseTracked(const XrViewState& viewState);
    } // namespace Pose

    namespace Quaternion {
        constexpr XrQuaternionf Identity();
        bool IsNormalized(const XrQuaternionf& quaternion);
    } // namespace Quaternion

    struct NearFar {
        float Near;
        float Far;
    };

    struct ViewProjection {
        XrPosef Pose;
        XrFovf Fov;
        NearFar NearFar;
    };

    // Type conversion between math types
    template <typename X, typename Y>
    const X& cast(const Y& value);

    // Convert XR types to DX
    DirectX::XMVECTOR XM_CALLCONV LoadXrVector2(const XrVector2f& vector);
    DirectX::XMVECTOR XM_CALLCONV LoadXrVector3(const XrVector3f& vector);
    DirectX::XMVECTOR XM_CALLCONV LoadXrVector4(const XrVector4f& vector);
    DirectX::XMVECTOR XM_CALLCONV LoadXrQuaternion(const XrQuaternionf& quaternion);
    DirectX::XMMATRIX XM_CALLCONV LoadXrPose(const XrPosef& rigidTransform);
    DirectX::XMMATRIX XM_CALLCONV LoadInvertedXrPose(const XrPosef& rigidTransform);
    DirectX::XMVECTOR XM_CALLCONV LoadXrExtent(const XrExtent2Df& extend);

    // Convert DX types to XR
    void XM_CALLCONV StoreXrVector2(XrVector2f* outVec, DirectX::FXMVECTOR inVec);
    void XM_CALLCONV StoreXrVector3(XrVector3f* outVec, DirectX::FXMVECTOR inVec);
    void XM_CALLCONV StoreXrVector4(XrVector4f* outVec, DirectX::FXMVECTOR inVec);
    void XM_CALLCONV StoreXrQuaternion(XrQuaternionf* outQuat, DirectX::FXMVECTOR inQuat);
    bool XM_CALLCONV StoreXrPose(XrPosef* out, DirectX::FXMMATRIX matrix);
    void XM_CALLCONV StoreXrExtent(XrExtent2Df* extend, DirectX::FXMVECTOR inVec);

    // Projection matrix math
    DirectX::XMMATRIX ComposeProjectionMatrix(const XrFovf& fov, const NearFar& nearFar);
    NearFar GetProjectionNearFar(const DirectX::XMFLOAT4X4& projectionMatrix);
    XrFovf DecomposeProjectionMatrix(const DirectX::XMFLOAT4X4& projectionMatrix);
} // namespace xr::math

#pragma region Implementation

namespace xr::math {
    namespace detail {
        template <typename X, typename Y>
        const X& implement_math_cast(const Y& value) {
            static_assert(std::is_pod<X>::value, "Unsafe to cast between non-POD types.");
            static_assert(std::is_pod<Y>::value, "Unsafe to cast between non-POD types.");
            static_assert(!std::is_pointer<X>::value, "Incorrect cast between pointer types.");
            static_assert(!std::is_pointer<Y>::value, "Incorrect cast between pointer types.");
            static_assert(sizeof(X) == sizeof(Y), "Incorrect cast between types with different sizes.");
            return reinterpret_cast<const X&>(value);
        }

        template <typename X, typename Y>
        X& implement_math_cast(Y& value) {
            static_assert(std::is_pod<X>::value, "Unsafe to cast between non-POD types.");
            static_assert(std::is_pod<Y>::value, "Unsafe to cast between non-POD types.");
            static_assert(!std::is_pointer<X>::value, "Incorrect cast between pointer types.");
            static_assert(!std::is_pointer<Y>::value, "Incorrect cast between pointer types.");
            static_assert(sizeof(X) == sizeof(Y), "Incorrect cast between types with different sizes.");
            return reinterpret_cast<X&>(value);
        }
    } // namespace detail

    template <typename X, typename Y>
    const X& cast(const Y& value) {
        static_assert(false, "Undefined cast from Y to type X");
    }

#define DEFINE_CAST(X, Y)                             \
    template <>                                       \
    inline const X& cast<X, Y>(const Y& value) {      \
        return detail::implement_math_cast<X>(value); \
    }

    static_assert(offsetof(DirectX::XMFLOAT2, x) == offsetof(XrVector2f, x));
    static_assert(offsetof(DirectX::XMFLOAT2, y) == offsetof(XrVector2f, y));
    DEFINE_CAST(XrVector2f, DirectX::XMFLOAT2);
    DEFINE_CAST(DirectX::XMFLOAT2, XrVector2f);

    static_assert(offsetof(DirectX::XMFLOAT3, x) == offsetof(XrVector3f, x));
    static_assert(offsetof(DirectX::XMFLOAT3, y) == offsetof(XrVector3f, y));
    static_assert(offsetof(DirectX::XMFLOAT3, z) == offsetof(XrVector3f, z));
    DEFINE_CAST(XrVector3f, DirectX::XMFLOAT3);
    DEFINE_CAST(DirectX::XMFLOAT3, XrVector3f);

    static_assert(offsetof(DirectX::XMFLOAT4, x) == offsetof(XrVector4f, x));
    static_assert(offsetof(DirectX::XMFLOAT4, y) == offsetof(XrVector4f, y));
    static_assert(offsetof(DirectX::XMFLOAT4, z) == offsetof(XrVector4f, z));
    static_assert(offsetof(DirectX::XMFLOAT4, w) == offsetof(XrVector4f, w));
    DEFINE_CAST(XrVector4f, DirectX::XMFLOAT4);
    DEFINE_CAST(DirectX::XMFLOAT4, XrVector4f);

    static_assert(offsetof(DirectX::XMFLOAT4, x) == offsetof(XrQuaternionf, x));
    static_assert(offsetof(DirectX::XMFLOAT4, y) == offsetof(XrQuaternionf, y));
    static_assert(offsetof(DirectX::XMFLOAT4, z) == offsetof(XrQuaternionf, z));
    static_assert(offsetof(DirectX::XMFLOAT4, w) == offsetof(XrQuaternionf, w));
    DEFINE_CAST(XrQuaternionf, DirectX::XMFLOAT4);
    DEFINE_CAST(DirectX::XMFLOAT4, XrQuaternionf);

    static_assert(offsetof(DirectX::XMINT2, x) == offsetof(XrExtent2Di, width));
    static_assert(offsetof(DirectX::XMINT2, y) == offsetof(XrExtent2Di, height));
    DEFINE_CAST(XrExtent2Di, DirectX::XMINT2);
    DEFINE_CAST(DirectX::XMINT2, XrExtent2Di);

    static_assert(offsetof(DirectX::XMFLOAT2, x) == offsetof(XrExtent2Df, width));
    static_assert(offsetof(DirectX::XMFLOAT2, y) == offsetof(XrExtent2Df, height));
    DEFINE_CAST(XrExtent2Df, DirectX::XMFLOAT2);
    DEFINE_CAST(DirectX::XMFLOAT2, XrExtent2Df);

    static_assert(offsetof(DirectX::XMFLOAT4, x) == offsetof(XrColor4f, r));
    static_assert(offsetof(DirectX::XMFLOAT4, y) == offsetof(XrColor4f, g));
    static_assert(offsetof(DirectX::XMFLOAT4, z) == offsetof(XrColor4f, b));
    static_assert(offsetof(DirectX::XMFLOAT4, w) == offsetof(XrColor4f, a));
    DEFINE_CAST(XrColor4f, DirectX::XMFLOAT4);
    DEFINE_CAST(DirectX::XMFLOAT4, XrColor4f);

#undef DEFINE_CAST

    // Shortcut non-templated overload of cast() function
#define DEFINE_CAST(X, Y)                             \
    inline const X& cast(const Y& value) {            \
        return detail::implement_math_cast<X>(value); \
    }

    DEFINE_CAST(DirectX::XMFLOAT2, XrVector2f);
    DEFINE_CAST(DirectX::XMFLOAT3, XrVector3f);
    DEFINE_CAST(DirectX::XMFLOAT4, XrVector4f);
    DEFINE_CAST(DirectX::XMFLOAT4, XrQuaternionf);
    DEFINE_CAST(DirectX::XMFLOAT2, XrExtent2Df);
#undef DEFINE_CAST

    inline DirectX::XMVECTOR XM_CALLCONV LoadXrVector2(const XrVector2f& vector) {
        return DirectX::XMLoadFloat2(&xr::math::cast(vector));
    }

    inline DirectX::XMVECTOR XM_CALLCONV LoadXrVector3(const XrVector3f& vector) {
        return DirectX::XMLoadFloat3(&xr::math::cast(vector));
    }

    inline DirectX::XMVECTOR XM_CALLCONV LoadXrVector4(const XrVector4f& vector) {
        return DirectX::XMLoadFloat4(&xr::math::cast(vector));
    }

    inline DirectX::XMVECTOR XM_CALLCONV LoadXrQuaternion(const XrQuaternionf& quaternion) {
        return DirectX::XMLoadFloat4(&xr::math::cast(quaternion));
    }

    inline DirectX::XMVECTOR XM_CALLCONV LoadXrExtent(const XrExtent2Df& extend) {
        return DirectX::XMLoadFloat2(&xr::math::cast(extend));
    }

    inline DirectX::XMMATRIX XM_CALLCONV LoadXrPose(const XrPosef& pose) {
        return XMMatrixAffineTransformation(DirectX::g_XMOne,  // scale
                                            DirectX::g_XMZero, // rotation origin
                                            LoadXrQuaternion(pose.orientation),
                                            LoadXrVector3(pose.position));
    }

    inline DirectX::XMMATRIX XM_CALLCONV LoadInvertedXrPose(const XrPosef& pose) {
        DirectX::XMVECTOR position = LoadXrVector3(pose.position);
        DirectX::XMVECTOR orientation = LoadXrQuaternion(pose.orientation);
        DirectX::XMVECTOR invertOrientation = DirectX::XMQuaternionConjugate(orientation);
        DirectX::XMVECTOR invertPosition = DirectX::XMVector3Rotate(DirectX::XMVectorNegate(position), invertOrientation);
        return XMMatrixAffineTransformation(DirectX::g_XMOne,  // scale
                                            DirectX::g_XMZero, // rotation origin
                                            invertOrientation, // rotation
                                            invertPosition);   // translation
    }

    inline void XM_CALLCONV StoreXrVector2(XrVector2f* outVec, DirectX::FXMVECTOR inVec) {
        DirectX::XMStoreFloat2(&detail::implement_math_cast<DirectX::XMFLOAT2>(*outVec), inVec);
    }

    inline void XM_CALLCONV StoreXrVector3(XrVector3f* outVec, DirectX::FXMVECTOR inVec) {
        DirectX::XMStoreFloat3(&detail::implement_math_cast<DirectX::XMFLOAT3>(*outVec), inVec);
    }

    inline void XM_CALLCONV StoreXrVector4(XrVector4f* outVec, DirectX::FXMVECTOR inVec) {
        DirectX::XMStoreFloat4(&detail::implement_math_cast<DirectX::XMFLOAT4>(*outVec), inVec);
    }

    inline void XM_CALLCONV StoreXrQuaternion(XrQuaternionf* outQuat, DirectX::FXMVECTOR inQuat) {
        DirectX::XMStoreFloat4(&detail::implement_math_cast<DirectX::XMFLOAT4>(*outQuat), inQuat);
    }

    inline void XM_CALLCONV StoreXrExtent(XrExtent2Df* outVec, DirectX::FXMVECTOR inVec) {
        DirectX::XMStoreFloat2(&detail::implement_math_cast<DirectX::XMFLOAT2>(*outVec), inVec);
    }

    inline bool XM_CALLCONV StoreXrPose(XrPosef* out, DirectX::FXMMATRIX matrix) {
        DirectX::XMVECTOR position;
        DirectX::XMVECTOR orientation;
        DirectX::XMVECTOR scale;

        if (!DirectX::XMMatrixDecompose(&scale, &orientation, &position, matrix)) {
            return false; // Non-SRT matrix encountered
        }

        StoreXrQuaternion(&out->orientation, orientation);
        StoreXrVector3(&out->position, position);
        return true;
    }

    namespace Pose {
        constexpr inline XrPosef Identity() {
            return {{0, 0, 0, 1}, {0, 0, 0}};
        }

        constexpr inline XrPosef Translation(const XrVector3f& translation) {
            XrPosef pose = Identity();
            pose.position = translation;
            return pose;
        }

        constexpr bool IsPoseValid(const XrSpaceLocation& spaceLocation) {
            constexpr XrSpaceLocationFlags PoseValidFlags = XR_SPACE_LOCATION_POSITION_VALID_BIT | XR_SPACE_LOCATION_ORIENTATION_VALID_BIT;
            return (spaceLocation.locationFlags & PoseValidFlags) == PoseValidFlags;
        }

        constexpr bool IsPoseTracked(const XrSpaceLocation& spaceLocation) {
            constexpr XrSpaceLocationFlags PoseTrackedFlags =
                XR_SPACE_LOCATION_POSITION_TRACKED_BIT | XR_SPACE_LOCATION_ORIENTATION_TRACKED_BIT;
            return (spaceLocation.locationFlags & PoseTrackedFlags) == PoseTrackedFlags;
        }

        constexpr bool IsPoseValid(const XrViewState& viewState) {
            constexpr XrViewStateFlags PoseValidFlags = XR_VIEW_STATE_POSITION_VALID_BIT | XR_VIEW_STATE_ORIENTATION_VALID_BIT;
            return (viewState.viewStateFlags & PoseValidFlags) == PoseValidFlags;
        }

        constexpr bool IsPoseTracked(const XrViewState& viewState) {
            constexpr XrViewStateFlags PoseTrackedFlags = XR_VIEW_STATE_POSITION_TRACKED_BIT | XR_VIEW_STATE_ORIENTATION_TRACKED_BIT;
            return (viewState.viewStateFlags & PoseTrackedFlags) == PoseTrackedFlags;
        }
    } // namespace Pose

    namespace Quaternion {
        constexpr inline XrQuaternionf Identity() {
            return {0, 0, 0, 1};
        }

        inline bool IsNormalized(const XrQuaternionf& quaternion) {
            DirectX::XMVECTOR vector = LoadXrQuaternion(quaternion);
            DirectX::XMVECTOR length = DirectX::XMVector4Length(vector);
            DirectX::XMVECTOR equal = DirectX::XMVectorNearEqual(length, DirectX::g_XMOne, XMQuaternionEpsilon);
            return DirectX::XMVectorGetX(equal) != 0;
        }
    } // namespace Quaternion

    inline bool IsValidFov(const XrFovf& fov) {
        if (fov.angleRight >= DirectX::XM_PIDIV2 || fov.angleLeft <= -DirectX::XM_PIDIV2) {
            return false;
        }

        if (fov.angleUp >= DirectX::XM_PIDIV2 || fov.angleDown <= -DirectX::XM_PIDIV2) {
            return false;
        }

        return true;
    }

    // 2 * n / (r - l)    0                  0                    0
    // 0                  2 * n / (t - b)    0                    0
    // (r + l) / (r - l)  (t + b) / (t - b)  f / (n - f)         -1
    // 0                  0                  n*f / (n - f)        0
    inline DirectX::XMMATRIX ComposeProjectionMatrix(const XrFovf& fov, const NearFar& nearFar) {
        if (!IsValidFov(fov)) {
            throw std::runtime_error("Invalid projection specification");
        }

        const float nearPlane = nearFar.Near;
        const float farPlane = nearFar.Far;
        const bool infNearPlane = isinf(nearPlane);
        const bool infFarPlane = isinf(farPlane);

        float l = tan(fov.angleLeft);
        float r = tan(fov.angleRight);
        float b = tan(fov.angleDown);
        float t = tan(fov.angleUp);
        if (!infNearPlane) {
            l *= nearPlane;
            r *= nearPlane;
            b *= nearPlane;
            t *= nearPlane;
        }

        if (nearPlane < 0.f || farPlane < 0.f) {
            throw std::runtime_error("Invalid projection specification");
        }

        if (infNearPlane || infFarPlane) {
            if (infNearPlane && infFarPlane) {
                throw std::runtime_error("Invalid projection specification");
            }

            const float reciprocalWidth = 1.0f / (r - l);
            const float reciprocalHeight = 1.0f / (t - b);

            DirectX::XMFLOAT4X4 projectionMatrix;

            float twoNearZ;
            if (infNearPlane) {
                twoNearZ = 2;

                projectionMatrix._33 = 0.0f;     // far / (near - far) = far / inf = 0
                projectionMatrix._43 = farPlane; // near * far / (near - far) = far * (near / (near - far)) = far * (inf / inf) = far
            } else {
                twoNearZ = nearPlane + nearPlane;

                projectionMatrix._33 = -1.0f;      // far / (near - far) = inf / -inf = -1
                projectionMatrix._43 = -nearPlane; // near * far / (near - far) = near * inf / -inf = -near
            }

            projectionMatrix._11 = twoNearZ * reciprocalWidth;
            projectionMatrix._12 = 0.0f;
            projectionMatrix._13 = 0.0f;
            projectionMatrix._14 = 0.0f;

            projectionMatrix._21 = 0.0f;
            projectionMatrix._22 = twoNearZ * reciprocalHeight;
            projectionMatrix._23 = 0.0f;
            projectionMatrix._24 = 0.0f;

            projectionMatrix._31 = (l + r) * reciprocalWidth;
            projectionMatrix._32 = (t + b) * reciprocalHeight;
            projectionMatrix._34 = -1.0f;

            projectionMatrix._41 = 0.0f;
            projectionMatrix._42 = 0.0f;
            projectionMatrix._44 = 0.0f;

            return DirectX::XMLoadFloat4x4(&projectionMatrix);
        } else {
            return DirectX::XMMatrixPerspectiveOffCenterRH(l, r, b, t, nearPlane, farPlane);
        }
    }

    inline bool IsInfiniteNearPlaneProjectionMatrix(const DirectX::XMFLOAT4X4& p) {
        return (p._33 == 0);
    }

    inline bool IsInfiniteFarPlaneProjectionMatrix(const DirectX::XMFLOAT4X4& p) {
        return (p._33 == -1);
    }

    inline void ValidateProjectionMatrix(const DirectX::XMFLOAT4X4& p) {
        // Reference equations on top of ComposeProjectionMatrix() above.
        if (p._12 != 0 || p._13 != 0 || p._14 != 0 ||
            // p._21 is not 0 on old MR devices, but small enough to be ignored. For future MR devices, it should be 0 (no shear)
            p._23 != 0 || p._24 != 0 ||
            // When near or far plane is infinite, p._33 is 0 or -1, respectively. They are valid cases.
            p._34 != -1 || p._41 != 0 || p._42 != 0 || p._44 != 0) {
            throw std::runtime_error("Invalid projection matrix");
        }
    }

    inline NearFar GetProjectionNearFar(const DirectX::XMFLOAT4X4& p) {
        ValidateProjectionMatrix(p);

        NearFar d;

        if (IsInfiniteNearPlaneProjectionMatrix(p)) {
            d.Near = std::numeric_limits<float>::infinity();
            d.Far = p._43;
        } else if (IsInfiniteFarPlaneProjectionMatrix(p)) {
            d.Near = -p._43;
            d.Far = std::numeric_limits<float>::infinity();
        } else {
            // Reference equations on top of ComposeProjectionMatrix() above.
            d.Near = p._43 / p._33;
            d.Far = p._43 / (1 + p._33);
        }

        return d;
    }

    inline XrFovf DecomposeProjectionMatrix(const DirectX::XMFLOAT4X4& p) {
        ValidateProjectionMatrix(p);

        // n = m43 / m33
        // f = m43 / (1 + m33)
        // l = n * (m31 - 1) / m11  => angle left = atan2(l, n) => atan2(m31 - 1, m11)
        // r = n * (m31 + 1) / m11  => so on
        // b = n * (m32 - 1) / m22  => and
        // t = n * (m32 + 1) / m22  => so forth
        XrFovf fov;
        fov.angleLeft = atan2(p._31 - 1, p._11);
        fov.angleRight = atan2(p._31 + 1, p._11);
        fov.angleDown = atan2(p._32 - 1, p._22);
        fov.angleUp = atan2(p._32 + 1, p._22);
        return fov;
    }

    template <uint32_t alignment>
    inline constexpr uint32_t AlignTo(uint32_t n) {
        static_assert((alignment & (alignment - 1)) == 0); // must be power-of-two
        return (n + alignment - 1) & ~(alignment - 1);
    }

    inline constexpr uint32_t DivideRoundingUp(uint32_t x, uint32_t y) {
        return (x + y - 1) / y;
    }
} // namespace xr::math

#pragma endregion
