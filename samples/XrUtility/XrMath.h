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
    namespace Pose {
        constexpr XrPosef Identity();
        constexpr XrPosef Translation(const XrVector3f& translation);
    } // namespace Pose

    namespace Quaternion {
        constexpr XrQuaternionf Identity();
        bool IsNormalized(const XrQuaternionf& quaternion);
    } // namespace Quaternion

    struct NearFarDistance {
        float Near;
        float Far;
    };

    // Convert XR types to DX
    DirectX::XMVECTOR XM_CALLCONV LoadXrVector3(const XrVector3f& vector);
    DirectX::XMVECTOR XM_CALLCONV LoadXrVector4(const XrVector4f& vector);
    DirectX::XMVECTOR XM_CALLCONV LoadXrQuaternion(const XrQuaternionf& quaternion);
    DirectX::XMMATRIX XM_CALLCONV LoadXrPose(const XrPosef& rigidTransform);
    DirectX::XMMATRIX XM_CALLCONV LoadInvertedXrPose(const XrPosef& rigidTransform);

    // Convert DX types to XR
    void XM_CALLCONV StoreXrVector3(XrVector3f* outVec, DirectX::FXMVECTOR inVec);
    void XM_CALLCONV StoreXrVector4(XrVector4f* outVec, DirectX::FXMVECTOR inVec);
    void XM_CALLCONV StoreXrQuaternion(XrQuaternionf* outQuat, DirectX::FXMVECTOR inQuat);
    bool XM_CALLCONV StoreXrPose(XrPosef* out, DirectX::FXMMATRIX matrix);

    // Projection matrix math
    DirectX::XMMATRIX ComposeProjectionMatrix(const XrFovf& fov, const NearFarDistance& nearFar);
    NearFarDistance GetProjectionNearFarDistance(const DirectX::XMFLOAT4X4& projectionMatrix);
    XrFovf DecomposeProjectionMatrix(const DirectX::XMFLOAT4X4& projectionMatrix);
} // namespace xr::math

#pragma region Implementation

namespace xr::math {
    template <typename T, typename U>
    T* AsPtr(U* u) {
        static_assert(sizeof(*u) == sizeof(T), "Pointer content are not the same size.");
        return reinterpret_cast<T*>(u);
    }

    template <typename T, typename U>
    const T* AsPtr(const U* u) {
        static_assert(sizeof(*u) == sizeof(T), "Pointer content are not the same size.");
        return reinterpret_cast<const T*>(u);
    }

    using namespace DirectX;
    inline XMVECTOR XM_CALLCONV LoadXrVector3(const XrVector3f& vector) {
        static_assert(offsetof(XMFLOAT3, x) == offsetof(XrVector3f, x));
        static_assert(offsetof(XMFLOAT3, y) == offsetof(XrVector3f, y));
        static_assert(offsetof(XMFLOAT3, z) == offsetof(XrVector3f, z));

        return XMLoadFloat3(AsPtr<XMFLOAT3>(&vector));
    }

    inline XMVECTOR XM_CALLCONV LoadXrVector4(const XrVector4f& vector) {
        static_assert(offsetof(XMFLOAT4, x) == offsetof(XrVector4f, x));
        static_assert(offsetof(XMFLOAT4, y) == offsetof(XrVector4f, y));
        static_assert(offsetof(XMFLOAT4, z) == offsetof(XrVector4f, z));
        static_assert(offsetof(XMFLOAT4, w) == offsetof(XrVector4f, w));

        return XMLoadFloat4(AsPtr<XMFLOAT4>(&vector));
    }

    inline XMVECTOR XM_CALLCONV LoadXrQuaternion(const XrQuaternionf& quaternion) {
        static_assert(offsetof(XMFLOAT4, x) == offsetof(XrQuaternionf, x));
        static_assert(offsetof(XMFLOAT4, y) == offsetof(XrQuaternionf, y));
        static_assert(offsetof(XMFLOAT4, z) == offsetof(XrQuaternionf, z));
        static_assert(offsetof(XMFLOAT4, w) == offsetof(XrQuaternionf, w));

        return XMLoadFloat4(AsPtr<XMFLOAT4>(&quaternion));
    }

    inline XMMATRIX XM_CALLCONV LoadXrPose(const XrPosef& pose) {
        return XMMatrixAffineTransformation(g_XMOne,  // scale
                                            g_XMZero, // rotation origin
                                            LoadXrQuaternion(pose.orientation),
                                            LoadXrVector3(pose.position));
    }

    inline DirectX::XMMATRIX XM_CALLCONV LoadInvertedXrPose(const XrPosef& pose) {
        XMVECTOR position = LoadXrVector3(pose.position);
        XMVECTOR orientation = LoadXrQuaternion(pose.orientation);
        XMVECTOR invertOrientation = XMQuaternionConjugate(orientation);
        XMVECTOR invertPosition = XMVector3Rotate(-position, invertOrientation);
        return XMMatrixAffineTransformation(g_XMOne,           // scale
                                            g_XMZero,          // rotation origin
                                            invertOrientation, // rotation
                                            invertPosition);   // translation
    }

    inline void XM_CALLCONV StoreXrVector3(XrVector3f* outVec, FXMVECTOR inVec) {
        XMStoreFloat3(AsPtr<XMFLOAT3>(outVec), inVec);
    }

    inline void XM_CALLCONV StoreXrVector4(XrVector4f* outVec, FXMVECTOR inVec) {
        XMStoreFloat4(AsPtr<XMFLOAT4>(outVec), inVec);
    }

    inline void XM_CALLCONV StoreXrQuaternion(XrQuaternionf* outQuat, FXMVECTOR inQuat) {
        XMStoreFloat4(AsPtr<XMFLOAT4>(outQuat), inQuat);
    }

    inline bool XM_CALLCONV StoreXrPose(XrPosef* out, FXMMATRIX matrix) {
        XMVECTOR position;
        XMVECTOR orientation;
        XMVECTOR scale;

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
    } // namespace Pose

    namespace Quaternion {
        constexpr inline XrQuaternionf Identity() {
            return {0, 0, 0, 1};
        }

        inline bool IsNormalized(const XrQuaternionf& quaternion) {
            XMVECTOR vector = LoadXrQuaternion(quaternion);
            XMVECTOR lenth = XMVector4Length(vector);
            XMVECTOR equal = XMVectorNearEqual(lenth, g_XMOne, g_XMEpsilon);
            return XMVectorGetX(equal) != 0;
        }
    } // namespace Quaternion

    // 2 * n / (r - l)    0                  0                    0
    // 0                  2 * n / (t - b)    0                    0
    // (r + l) / (r - l)  (t + b) / (t - b)  f / (n - f)          -1
    // 0                  0                  n*f / (n - f)        0
    inline DirectX::XMMATRIX ComposeProjectionMatrix(const XrFovf& fov, const NearFarDistance& nearFar) {
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

        constexpr float epsilon = 0.00001f;
        if (nearPlane < 0.f || farPlane < 0.f || r < l || t < b || XMScalarNearEqual(r, l, epsilon) || XMScalarNearEqual(b, t, epsilon) ||
            XMScalarNearEqual(nearPlane, farPlane, epsilon)) {
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

            return XMLoadFloat4x4(&projectionMatrix);
        } else {
            return XMMatrixPerspectiveOffCenterRH(l, r, b, t, nearPlane, farPlane);
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

    inline NearFarDistance GetProjectionNearFarDistance(const DirectX::XMFLOAT4X4& p) {
        ValidateProjectionMatrix(p);

        NearFarDistance d;

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
