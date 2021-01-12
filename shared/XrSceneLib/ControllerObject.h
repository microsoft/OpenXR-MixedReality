// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "PbrModelObject.h"

namespace engine {
    std::shared_ptr<engine::PbrModelObject> CreateControllerObject(Context& context, XrPath controllerUserPath);
}
