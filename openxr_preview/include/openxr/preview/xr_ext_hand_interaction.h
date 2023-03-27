////////////////////////////////////////////////////////////////////////////////
// Copyright (C) Microsoft Corporation.  All Rights Reserved

#ifndef XR_EXT_HAND_INTERACTION_H_
#define XR_EXT_HAND_INTERACTION_H 1

#define XR_LIST_EXTENSIONS_EXT_hand_interaction(_) _(XR_EXT_hand_interaction, 303)
#define XR_LIST_STRUCTURE_TYPES_EXT_hand_interaction(_)
#define XR_LIST_ENUM_XrStructureType_EXT_hand_interaction(_)

#define XR_EXT_hand_interaction 1
#define XR_EXT_hand_interaction_SPEC_VERSION 3
#define XR_EXT_HAND_INTERACTION_EXTENSION_NAME "XR_EXT_hand_interaction"
/// There are two goals of this XR_EXT_hand_interaction extension:
///
/// - Seeking the alignment of commonly used hand poses and corresponding actions for hand interactions across motion controller and hand
///   tracking devices.
///
/// - Providing a new interaction profile for hand tracking input device to provide actions through the OpenXR action system.
///
/// The design is focusing on the following 4 commonly used hand interaction scenarios:
///
/// - Interacting with far object out of user's reach, such as using a virtual laser pointer to click a button on the wall, using
///   .../aim/pose.
///
/// - Picking up a big object or holding a grip, for example, pulling a door handle or holding a sword, using .../grip/pose.
///
/// - Pinching or interacting with a small object within reach, such as turning a key to open a lock or moving the knob on a slider bar,
///   using .../pinch_ext/pose.
///
/// - For finger tip touch interaction, such as pressing a push button, swiping to slide a browser view, or typing on a virtual keyboard,
///   using .../poke_ext/pose.
///
/// The above 4 hand poses must be supported by all interaction profile supporting /user/hand/left or /user/hand/right
/// top-level paths, including motion controller interaction profiles, as well as the following new interaction profile for hand tracking.
///
/// The 4 hand poses are typically used together with appropriate buttons on controller or corresponding hand gesture actions in following
/// new interaction profile.
///
/// The new interaction profile is added for optical hand tracking input devices, this interaction profile includes the 4 poses as above, as
/// well as following gesture actions:
///
/// - The .../aim_activate_ext/value action that's optimized for stabilizing .../aim/pose for interacting with objects out of user's direct
///   reach. Also added bool action .../aim_activate_ext/ready_ext to indicate if the aim gesture is ready to be used.
///
/// - The .../grasp/value action that's optimized to be used together with .../grip/pose for direct manipulation. Also added bool action
///   .../grasp/ready_ext to indicate if the grasp gesture is ready to be used.
///
/// - The .../pinch_ext/value that's optimized to be used together with the .../pinch_ext/pose for pinch gestures. Also added bool action
///   .../pinch_ext/ready_ext to indicate if the pinch gesture is ready to be used.
///
/// The new actions are only available with the new /interaction_profiles/ext/hand_interaction_ext profile for hand tracking devices, and they
/// will not affect existing controller interaction profiles.

#endif // XR_EXT_HAND_INTERACTION_H_
