// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "Object.h"

namespace engine {
    std::shared_ptr<engine::Object> CreateSpaceObject(xr::SpaceHandle space, bool hideWhenPoseInvalid = true);
} // namespace engine
