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

    // A dense array of joint indices to perform fast mapping
    constexpr XrHandJointMSFT HandJoints[] = {XR_HAND_JOINT_PALM_MSFT,
                                              XR_HAND_JOINT_WRIST_MSFT,
                                              XR_HAND_JOINT_THUMB_METACARPAL_MSFT,
                                              XR_HAND_JOINT_THUMB_PROXIMAL_MSFT,
                                              XR_HAND_JOINT_THUMB_DISTAL_MSFT,
                                              XR_HAND_JOINT_THUMB_TIP_MSFT,
                                              XR_HAND_JOINT_INDEX_METACARPAL_MSFT,
                                              XR_HAND_JOINT_INDEX_PROXIMAL_MSFT,
                                              XR_HAND_JOINT_INDEX_INTERMEDIATE_MSFT,
                                              XR_HAND_JOINT_INDEX_DISTAL_MSFT,
                                              XR_HAND_JOINT_INDEX_TIP_MSFT,
                                              XR_HAND_JOINT_MIDDLE_METACARPAL_MSFT,
                                              XR_HAND_JOINT_MIDDLE_PROXIMAL_MSFT,
                                              XR_HAND_JOINT_MIDDLE_INTERMEDIATE_MSFT,
                                              XR_HAND_JOINT_MIDDLE_DISTAL_MSFT,
                                              XR_HAND_JOINT_MIDDLE_TIP_MSFT,
                                              XR_HAND_JOINT_RING_METACARPAL_MSFT,
                                              XR_HAND_JOINT_RING_PROXIMAL_MSFT,
                                              XR_HAND_JOINT_RING_INTERMEDIATE_MSFT,
                                              XR_HAND_JOINT_RING_DISTAL_MSFT,
                                              XR_HAND_JOINT_RING_TIP_MSFT,
                                              XR_HAND_JOINT_LITTLE_METACARPAL_MSFT,
                                              XR_HAND_JOINT_LITTLE_PROXIMAL_MSFT,
                                              XR_HAND_JOINT_LITTLE_INTERMEDIATE_MSFT,
                                              XR_HAND_JOINT_LITTLE_DISTAL_MSFT,
                                              XR_HAND_JOINT_LITTLE_TIP_MSFT};

    constexpr size_t HandJointCount = std::size(HandJoints);

    constexpr uint32_t JointToIndex(XrHandJointMSFT jointType) {
        return static_cast<uint32_t>(jointType);
    }

    template <size_t Index>
    struct CheckJointIndex {
        constexpr static size_t JointIndex = JointToIndex(HandJoints[Index]);
        static_assert(Index - 1 == CheckJointIndex<Index - 1>::JointIndex);
    };

    template <>
    struct CheckJointIndex<0> {
        constexpr static size_t JointIndex = JointToIndex(HandJoints[0]);
    };

    static_assert(XR_HAND_JOINT_LITTLE_TIP_MSFT == CheckJointIndex<XR_HAND_JOINT_LITTLE_TIP_MSFT>::JointIndex);
} // namespace xr
