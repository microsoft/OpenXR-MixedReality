// *********** THIS FILE IS GENERATED - DO NOT EDIT ***********
//     See loader_source_generator.py for modifications
// ************************************************************

// Copyright (c) 2017-2019 The Khronos Group Inc.
// Copyright (c) 2017-2019 Valve Corporation
// Copyright (c) 2017-2019 LunarG, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// Author: Mark Young <marky@lunarg.com>
//

#include <ios>
#include <sstream>
#include <cstring>
#include <string>

#include <algorithm>

#include "xr_dependencies.h"
#include <openxr/openxr.h>
#include <openxr/openxr_platform.h>

#include "loader_logger.hpp"
#include "xr_generated_loader.hpp"
#include "xr_generated_dispatch_table.h"
#include "xr_generated_utilities.h"
#include "api_layer_interface.hpp"

// Unordered maps to lookup the instance for a given object type
std::unordered_map<XrInstance, class LoaderInstance*> g_instance_map;
std::mutex g_instance_mutex;
std::unordered_map<XrSession, class LoaderInstance*> g_session_map;
std::mutex g_session_mutex;
std::unordered_map<XrSpace, class LoaderInstance*> g_space_map;
std::mutex g_space_mutex;
std::unordered_map<XrAction, class LoaderInstance*> g_action_map;
std::mutex g_action_mutex;
std::unordered_map<XrSwapchain, class LoaderInstance*> g_swapchain_map;
std::mutex g_swapchain_mutex;
std::unordered_map<XrActionSet, class LoaderInstance*> g_actionset_map;
std::mutex g_actionset_mutex;
std::unordered_map<XrDebugUtilsMessengerEXT, class LoaderInstance*> g_debugutilsmessengerext_map;
std::mutex g_debugutilsmessengerext_mutex;
std::unordered_map<XrSpatialAnchorMSFT, class LoaderInstance*> g_spatialanchormsft_map;
std::mutex g_spatialanchormsft_mutex;

// Template function to reduce duplicating the map locking, searching, and deleting.`
template <typename MapType>
void EraseAllInstanceMapElements(MapType &search_map, std::mutex &mutex, LoaderInstance *search_value) {
    try {
        std::unique_lock<std::mutex> lock(mutex);
        for (auto it = search_map.begin(); it != search_map.end();) {
            if (it->second == search_value) {
                search_map.erase(it++);
            } else {
                ++it;
            }
        }
    } catch (...) {
        // Log a message, but don't throw an exception outside of this so we continue to erase the
        // remaining items in the remaining maps.
        LoaderLogger::LogErrorMessage("xrDestroyInstance", "EraseAllInstanceMapElements encountered an exception.  Ignoring it for now.");
    }
}

// Function used to clean up any residual map values that point to an instance prior to that
// instance being deleted.
void LoaderCleanUpMapsForInstance(class LoaderInstance *instance) {
    EraseAllInstanceMapElements<std::unordered_map<XrInstance, class LoaderInstance*>>(g_instance_map, g_instance_mutex, instance);
    EraseAllInstanceMapElements<std::unordered_map<XrSession, class LoaderInstance*>>(g_session_map, g_session_mutex, instance);
    EraseAllInstanceMapElements<std::unordered_map<XrSpace, class LoaderInstance*>>(g_space_map, g_space_mutex, instance);
    EraseAllInstanceMapElements<std::unordered_map<XrAction, class LoaderInstance*>>(g_action_map, g_action_mutex, instance);
    EraseAllInstanceMapElements<std::unordered_map<XrSwapchain, class LoaderInstance*>>(g_swapchain_map, g_swapchain_mutex, instance);
    EraseAllInstanceMapElements<std::unordered_map<XrActionSet, class LoaderInstance*>>(g_actionset_map, g_actionset_mutex, instance);
    EraseAllInstanceMapElements<std::unordered_map<XrDebugUtilsMessengerEXT, class LoaderInstance*>>(g_debugutilsmessengerext_map, g_debugutilsmessengerext_mutex, instance);
    EraseAllInstanceMapElements<std::unordered_map<XrSpatialAnchorMSFT, class LoaderInstance*>>(g_spatialanchormsft_map, g_spatialanchormsft_mutex, instance);
}

LoaderInstance* TryLookupLoaderInstance(XrInstance instance) {
    std::unique_lock<std::mutex> instance_lock(g_instance_mutex);
    auto instance_iter = g_instance_map.find(instance);
    if (instance_iter != g_instance_map.end()) {
        return instance_iter->second;
    }
    else {
        return nullptr;
    }
}
#ifdef __cplusplus
extern "C" { 
#endif

// Automatically generated instance trampolines and terminators

// ---- Core 0.90 commands
XRAPI_ATTR XrResult XRAPI_CALL xrGetInstanceProperties(
    XrInstance                                  instance,
    XrInstanceProperties*                       instanceProperties) {
    try {
        std::unique_lock<std::mutex> secondary_lock(g_instance_mutex);
        LoaderInstance *loader_instance = g_instance_map[instance];
        secondary_lock.unlock();
        if (nullptr == loader_instance) {
            XrLoaderLogObjectInfo bad_object = {};
            bad_object.type = XR_OBJECT_TYPE_INSTANCE;
            bad_object.handle = reinterpret_cast<uint64_t const&>(instance);
            std::vector<XrLoaderLogObjectInfo> loader_objects;
            loader_objects.push_back(bad_object);
            LoaderLogger::LogValidationErrorMessage("VUID-xrGetInstanceProperties-instance-parameter", "xrGetInstanceProperties",
                                                    "instance is not a valid XrInstance", loader_objects);
            return XR_ERROR_HANDLE_INVALID;
        }
        const std::unique_ptr<XrGeneratedDispatchTable>& dispatch_table = loader_instance->DispatchTable();
        return dispatch_table->GetInstanceProperties(instance, instanceProperties);
    } catch (...) {
        std::string error_message = "xrGetInstanceProperties trampoline encountered an unknown error.  Likely XrInstance 0x";
        std::ostringstream oss;
        oss << std::hex << reinterpret_cast<const void*>(instance);
        error_message += oss.str();
        error_message += " is invalid";
        LoaderLogger::LogErrorMessage("xrGetInstanceProperties", error_message);
        return XR_ERROR_HANDLE_INVALID;
    }
}


XRAPI_ATTR XrResult XRAPI_CALL xrPollEvent(
    XrInstance                                  instance,
    XrEventDataBuffer*                          eventData) {
    try {
        std::unique_lock<std::mutex> secondary_lock(g_instance_mutex);
        LoaderInstance *loader_instance = g_instance_map[instance];
        secondary_lock.unlock();
        if (nullptr == loader_instance) {
            XrLoaderLogObjectInfo bad_object = {};
            bad_object.type = XR_OBJECT_TYPE_INSTANCE;
            bad_object.handle = reinterpret_cast<uint64_t const&>(instance);
            std::vector<XrLoaderLogObjectInfo> loader_objects;
            loader_objects.push_back(bad_object);
            LoaderLogger::LogValidationErrorMessage("VUID-xrPollEvent-instance-parameter", "xrPollEvent",
                                                    "instance is not a valid XrInstance", loader_objects);
            return XR_ERROR_HANDLE_INVALID;
        }
        const std::unique_ptr<XrGeneratedDispatchTable>& dispatch_table = loader_instance->DispatchTable();
        return dispatch_table->PollEvent(instance, eventData);
    } catch (...) {
        std::string error_message = "xrPollEvent trampoline encountered an unknown error.  Likely XrInstance 0x";
        std::ostringstream oss;
        oss << std::hex << reinterpret_cast<const void*>(instance);
        error_message += oss.str();
        error_message += " is invalid";
        LoaderLogger::LogErrorMessage("xrPollEvent", error_message);
        return XR_ERROR_HANDLE_INVALID;
    }
}


XRAPI_ATTR XrResult XRAPI_CALL xrResultToString(
    XrInstance                                  instance,
    XrResult                                    value,
    char                                        buffer[XR_MAX_RESULT_STRING_SIZE]) {
    try {
        std::unique_lock<std::mutex> secondary_lock(g_instance_mutex);
        LoaderInstance *loader_instance = g_instance_map[instance];
        secondary_lock.unlock();
        if (nullptr == loader_instance) {
            XrLoaderLogObjectInfo bad_object = {};
            bad_object.type = XR_OBJECT_TYPE_INSTANCE;
            bad_object.handle = reinterpret_cast<uint64_t const&>(instance);
            std::vector<XrLoaderLogObjectInfo> loader_objects;
            loader_objects.push_back(bad_object);
            LoaderLogger::LogValidationErrorMessage("VUID-xrResultToString-instance-parameter", "xrResultToString",
                                                    "instance is not a valid XrInstance", loader_objects);
            return XR_ERROR_HANDLE_INVALID;
        }
        const std::unique_ptr<XrGeneratedDispatchTable>& dispatch_table = loader_instance->DispatchTable();
        return dispatch_table->ResultToString(instance, value, buffer);
    } catch (...) {
        std::string error_message = "xrResultToString trampoline encountered an unknown error.  Likely XrInstance 0x";
        std::ostringstream oss;
        oss << std::hex << reinterpret_cast<const void*>(instance);
        error_message += oss.str();
        error_message += " is invalid";
        LoaderLogger::LogErrorMessage("xrResultToString", error_message);
        return XR_ERROR_HANDLE_INVALID;
    }
}

XRAPI_ATTR XrResult XRAPI_CALL LoaderGenTermXrResultToString(
    XrInstance                                  instance,
    XrResult                                    value,
    char                                        buffer[XR_MAX_RESULT_STRING_SIZE]) {
    try {
        XrResult result = GeneratedXrUtilitiesResultToString(value, buffer);
        if (XR_SUCCESS != result) {
            return result;
        }
        // If we did not find it in the generated code, ask the runtime.
        const XrGeneratedDispatchTable* dispatch_table = RuntimeInterface::GetRuntime().GetDispatchTable(instance);
        if (nullptr != dispatch_table->ResultToString) {
            result = dispatch_table->ResultToString(instance, value, buffer);
        }
        return result;
    } catch (...) {
        LoaderLogger::LogErrorMessage("xrResultToString", "xrResultToString terminator encountered an unknown error");
        // NOTE: Most calls only allow XR_SUCCESS as a return code
        return XR_SUCCESS;
    }
}

XRAPI_ATTR XrResult XRAPI_CALL xrStructureTypeToString(
    XrInstance                                  instance,
    XrStructureType                             value,
    char                                        buffer[XR_MAX_STRUCTURE_NAME_SIZE]) {
    try {
        std::unique_lock<std::mutex> secondary_lock(g_instance_mutex);
        LoaderInstance *loader_instance = g_instance_map[instance];
        secondary_lock.unlock();
        if (nullptr == loader_instance) {
            XrLoaderLogObjectInfo bad_object = {};
            bad_object.type = XR_OBJECT_TYPE_INSTANCE;
            bad_object.handle = reinterpret_cast<uint64_t const&>(instance);
            std::vector<XrLoaderLogObjectInfo> loader_objects;
            loader_objects.push_back(bad_object);
            LoaderLogger::LogValidationErrorMessage("VUID-xrStructureTypeToString-instance-parameter", "xrStructureTypeToString",
                                                    "instance is not a valid XrInstance", loader_objects);
            return XR_ERROR_HANDLE_INVALID;
        }
        const std::unique_ptr<XrGeneratedDispatchTable>& dispatch_table = loader_instance->DispatchTable();
        return dispatch_table->StructureTypeToString(instance, value, buffer);
    } catch (...) {
        std::string error_message = "xrStructureTypeToString trampoline encountered an unknown error.  Likely XrInstance 0x";
        std::ostringstream oss;
        oss << std::hex << reinterpret_cast<const void*>(instance);
        error_message += oss.str();
        error_message += " is invalid";
        LoaderLogger::LogErrorMessage("xrStructureTypeToString", error_message);
        return XR_ERROR_HANDLE_INVALID;
    }
}

XRAPI_ATTR XrResult XRAPI_CALL LoaderGenTermXrStructureTypeToString(
    XrInstance                                  instance,
    XrStructureType                             value,
    char                                        buffer[XR_MAX_STRUCTURE_NAME_SIZE]) {
    try {
        XrResult result = GeneratedXrUtilitiesStructureTypeToString(value, buffer);
        if (XR_SUCCESS != result) {
            return result;
        }
        // If we did not find it in the generated code, ask the runtime.
        const XrGeneratedDispatchTable* dispatch_table = RuntimeInterface::GetRuntime().GetDispatchTable(instance);
        if (nullptr != dispatch_table->StructureTypeToString) {
            result = dispatch_table->StructureTypeToString(instance, value, buffer);
        }
        return result;
    } catch (...) {
        LoaderLogger::LogErrorMessage("xrStructureTypeToString", "xrStructureTypeToString terminator encountered an unknown error");
        // NOTE: Most calls only allow XR_SUCCESS as a return code
        return XR_SUCCESS;
    }
}

XRAPI_ATTR XrResult XRAPI_CALL xrGetSystem(
    XrInstance                                  instance,
    const XrSystemGetInfo*                      getInfo,
    XrSystemId*                                 systemId) {
    try {
        std::unique_lock<std::mutex> secondary_lock(g_instance_mutex);
        LoaderInstance *loader_instance = g_instance_map[instance];
        secondary_lock.unlock();
        if (nullptr == loader_instance) {
            XrLoaderLogObjectInfo bad_object = {};
            bad_object.type = XR_OBJECT_TYPE_INSTANCE;
            bad_object.handle = reinterpret_cast<uint64_t const&>(instance);
            std::vector<XrLoaderLogObjectInfo> loader_objects;
            loader_objects.push_back(bad_object);
            LoaderLogger::LogValidationErrorMessage("VUID-xrGetSystem-instance-parameter", "xrGetSystem",
                                                    "instance is not a valid XrInstance", loader_objects);
            return XR_ERROR_HANDLE_INVALID;
        }
        const std::unique_ptr<XrGeneratedDispatchTable>& dispatch_table = loader_instance->DispatchTable();
        return dispatch_table->GetSystem(instance, getInfo, systemId);
    } catch (...) {
        std::string error_message = "xrGetSystem trampoline encountered an unknown error.  Likely XrInstance 0x";
        std::ostringstream oss;
        oss << std::hex << reinterpret_cast<const void*>(instance);
        error_message += oss.str();
        error_message += " is invalid";
        LoaderLogger::LogErrorMessage("xrGetSystem", error_message);
        return XR_ERROR_HANDLE_INVALID;
    }
}


XRAPI_ATTR XrResult XRAPI_CALL xrGetSystemProperties(
    XrInstance                                  instance,
    XrSystemId                                  systemId,
    XrSystemProperties*                         properties) {
    try {
        std::unique_lock<std::mutex> secondary_lock(g_instance_mutex);
        LoaderInstance *loader_instance = g_instance_map[instance];
        secondary_lock.unlock();
        if (nullptr == loader_instance) {
            XrLoaderLogObjectInfo bad_object = {};
            bad_object.type = XR_OBJECT_TYPE_INSTANCE;
            bad_object.handle = reinterpret_cast<uint64_t const&>(instance);
            std::vector<XrLoaderLogObjectInfo> loader_objects;
            loader_objects.push_back(bad_object);
            LoaderLogger::LogValidationErrorMessage("VUID-xrGetSystemProperties-instance-parameter", "xrGetSystemProperties",
                                                    "instance is not a valid XrInstance", loader_objects);
            return XR_ERROR_HANDLE_INVALID;
        }
        const std::unique_ptr<XrGeneratedDispatchTable>& dispatch_table = loader_instance->DispatchTable();
        return dispatch_table->GetSystemProperties(instance, systemId, properties);
    } catch (...) {
        std::string error_message = "xrGetSystemProperties trampoline encountered an unknown error.  Likely XrInstance 0x";
        std::ostringstream oss;
        oss << std::hex << reinterpret_cast<const void*>(instance);
        error_message += oss.str();
        error_message += " is invalid";
        LoaderLogger::LogErrorMessage("xrGetSystemProperties", error_message);
        return XR_ERROR_HANDLE_INVALID;
    }
}


XRAPI_ATTR XrResult XRAPI_CALL xrEnumerateEnvironmentBlendModes(
    XrInstance                                  instance,
    XrSystemId                                  systemId,
    uint32_t                                    environmentBlendModeCapacityInput,
    uint32_t*                                   environmentBlendModeCountOutput,
    XrEnvironmentBlendMode*                     environmentBlendModes) {
    try {
        std::unique_lock<std::mutex> secondary_lock(g_instance_mutex);
        LoaderInstance *loader_instance = g_instance_map[instance];
        secondary_lock.unlock();
        if (nullptr == loader_instance) {
            XrLoaderLogObjectInfo bad_object = {};
            bad_object.type = XR_OBJECT_TYPE_INSTANCE;
            bad_object.handle = reinterpret_cast<uint64_t const&>(instance);
            std::vector<XrLoaderLogObjectInfo> loader_objects;
            loader_objects.push_back(bad_object);
            LoaderLogger::LogValidationErrorMessage("VUID-xrEnumerateEnvironmentBlendModes-instance-parameter", "xrEnumerateEnvironmentBlendModes",
                                                    "instance is not a valid XrInstance", loader_objects);
            return XR_ERROR_HANDLE_INVALID;
        }
        const std::unique_ptr<XrGeneratedDispatchTable>& dispatch_table = loader_instance->DispatchTable();
        return dispatch_table->EnumerateEnvironmentBlendModes(instance, systemId, environmentBlendModeCapacityInput, environmentBlendModeCountOutput, environmentBlendModes);
    } catch (...) {
        std::string error_message = "xrEnumerateEnvironmentBlendModes trampoline encountered an unknown error.  Likely XrInstance 0x";
        std::ostringstream oss;
        oss << std::hex << reinterpret_cast<const void*>(instance);
        error_message += oss.str();
        error_message += " is invalid";
        LoaderLogger::LogErrorMessage("xrEnumerateEnvironmentBlendModes", error_message);
        return XR_ERROR_HANDLE_INVALID;
    }
}


XRAPI_ATTR XrResult XRAPI_CALL xrCreateSession(
    XrInstance                                  instance,
    const XrSessionCreateInfo*                  createInfo,
    XrSession*                                  session) {
    try {
        std::unique_lock<std::mutex> secondary_lock(g_instance_mutex);
        LoaderInstance *loader_instance = g_instance_map[instance];
        secondary_lock.unlock();
        if (nullptr == loader_instance) {
            XrLoaderLogObjectInfo bad_object = {};
            bad_object.type = XR_OBJECT_TYPE_INSTANCE;
            bad_object.handle = reinterpret_cast<uint64_t const&>(instance);
            std::vector<XrLoaderLogObjectInfo> loader_objects;
            loader_objects.push_back(bad_object);
            LoaderLogger::LogValidationErrorMessage("VUID-xrCreateSession-instance-parameter", "xrCreateSession",
                                                    "instance is not a valid XrInstance", loader_objects);
            return XR_ERROR_HANDLE_INVALID;
        }
        const std::unique_ptr<XrGeneratedDispatchTable>& dispatch_table = loader_instance->DispatchTable();
        XrResult result = XR_SUCCESS;
        result = dispatch_table->CreateSession(instance, createInfo, session);
        if (XR_SUCCESS == result && nullptr != session) {
            std::unique_lock<std::mutex> session_lock(g_session_mutex);
            auto exists = g_session_map.find(*session);
            if (exists == g_session_map.end()) {
                g_session_map[*session] = loader_instance;
            }
        }
        return result;
    } catch (std::bad_alloc &) {
        LoaderLogger::LogErrorMessage("xrCreateSession", "xrCreateSession trampoline failed allocating memory");
        return XR_ERROR_OUT_OF_MEMORY;
    } catch (...) {
        LoaderLogger::LogErrorMessage("xrCreateSession", "xrCreateSession trampoline encountered an unknown error");
        return XR_ERROR_INITIALIZATION_FAILED;
    }
}


XRAPI_ATTR XrResult XRAPI_CALL xrDestroySession(
    XrSession                                   session) {
    try {
        std::unique_lock<std::mutex> secondary_lock(g_session_mutex);
        LoaderInstance *loader_instance = g_session_map[session];
        // Destroy the mapping entry for this item if it was valid.
        if (nullptr != loader_instance) {
                g_session_map.erase(session);
        }
        secondary_lock.unlock();
        if (nullptr == loader_instance) {
            XrLoaderLogObjectInfo bad_object = {};
            bad_object.type = XR_OBJECT_TYPE_SESSION;
            bad_object.handle = reinterpret_cast<uint64_t const&>(session);
            std::vector<XrLoaderLogObjectInfo> loader_objects;
            loader_objects.push_back(bad_object);
            LoaderLogger::LogValidationErrorMessage("VUID-xrDestroySession-session-parameter", "xrDestroySession",
                                                    "session is not a valid XrSession", loader_objects);
            return XR_ERROR_HANDLE_INVALID;
        }
        const std::unique_ptr<XrGeneratedDispatchTable>& dispatch_table = loader_instance->DispatchTable();
        XrResult result = XR_SUCCESS;
        result = dispatch_table->DestroySession(session);
        return result;
    } catch (...) {
        LoaderLogger::LogErrorMessage("xrDestroySession", "xrDestroySession trampoline encountered an unknown error");
        // NOTE: Most calls only allow XR_SUCCESS as a return code
        return XR_SUCCESS;
    }
}


XRAPI_ATTR XrResult XRAPI_CALL xrEnumerateReferenceSpaces(
    XrSession                                   session,
    uint32_t                                    spaceCapacityInput,
    uint32_t*                                   spaceCountOutput,
    XrReferenceSpaceType*                       spaces) {
    try {
        std::unique_lock<std::mutex> secondary_lock(g_session_mutex);
        LoaderInstance *loader_instance = g_session_map[session];
        secondary_lock.unlock();
        if (nullptr == loader_instance) {
            XrLoaderLogObjectInfo bad_object = {};
            bad_object.type = XR_OBJECT_TYPE_SESSION;
            bad_object.handle = reinterpret_cast<uint64_t const&>(session);
            std::vector<XrLoaderLogObjectInfo> loader_objects;
            loader_objects.push_back(bad_object);
            LoaderLogger::LogValidationErrorMessage("VUID-xrEnumerateReferenceSpaces-session-parameter", "xrEnumerateReferenceSpaces",
                                                    "session is not a valid XrSession", loader_objects);
            return XR_ERROR_HANDLE_INVALID;
        }
        const std::unique_ptr<XrGeneratedDispatchTable>& dispatch_table = loader_instance->DispatchTable();
        return dispatch_table->EnumerateReferenceSpaces(session, spaceCapacityInput, spaceCountOutput, spaces);
    } catch (...) {
        LoaderLogger::LogErrorMessage("xrEnumerateReferenceSpaces", "xrEnumerateReferenceSpaces trampoline encountered an unknown error");
        // NOTE: Most calls only allow XR_SUCCESS as a return code
        return XR_SUCCESS;
    }
}


XRAPI_ATTR XrResult XRAPI_CALL xrCreateReferenceSpace(
    XrSession                                   session,
    const XrReferenceSpaceCreateInfo*           createInfo,
    XrSpace*                                    space) {
    try {
        std::unique_lock<std::mutex> secondary_lock(g_session_mutex);
        LoaderInstance *loader_instance = g_session_map[session];
        secondary_lock.unlock();
        if (nullptr == loader_instance) {
            XrLoaderLogObjectInfo bad_object = {};
            bad_object.type = XR_OBJECT_TYPE_SESSION;
            bad_object.handle = reinterpret_cast<uint64_t const&>(session);
            std::vector<XrLoaderLogObjectInfo> loader_objects;
            loader_objects.push_back(bad_object);
            LoaderLogger::LogValidationErrorMessage("VUID-xrCreateReferenceSpace-session-parameter", "xrCreateReferenceSpace",
                                                    "session is not a valid XrSession", loader_objects);
            return XR_ERROR_HANDLE_INVALID;
        }
        const std::unique_ptr<XrGeneratedDispatchTable>& dispatch_table = loader_instance->DispatchTable();
        XrResult result = XR_SUCCESS;
        result = dispatch_table->CreateReferenceSpace(session, createInfo, space);
        if (XR_SUCCESS == result && nullptr != space) {
            std::unique_lock<std::mutex> space_lock(g_space_mutex);
            auto exists = g_space_map.find(*space);
            if (exists == g_space_map.end()) {
                g_space_map[*space] = loader_instance;
            }
        }
        return result;
    } catch (std::bad_alloc &) {
        LoaderLogger::LogErrorMessage("xrCreateReferenceSpace", "xrCreateReferenceSpace trampoline failed allocating memory");
        return XR_ERROR_OUT_OF_MEMORY;
    } catch (...) {
        LoaderLogger::LogErrorMessage("xrCreateReferenceSpace", "xrCreateReferenceSpace trampoline encountered an unknown error");
        return XR_ERROR_INITIALIZATION_FAILED;
    }
}


XRAPI_ATTR XrResult XRAPI_CALL xrGetReferenceSpaceBoundsRect(
    XrSession                                   session,
    XrReferenceSpaceType                        referenceSpaceType,
    XrExtent2Df*                                bounds) {
    try {
        std::unique_lock<std::mutex> secondary_lock(g_session_mutex);
        LoaderInstance *loader_instance = g_session_map[session];
        secondary_lock.unlock();
        if (nullptr == loader_instance) {
            XrLoaderLogObjectInfo bad_object = {};
            bad_object.type = XR_OBJECT_TYPE_SESSION;
            bad_object.handle = reinterpret_cast<uint64_t const&>(session);
            std::vector<XrLoaderLogObjectInfo> loader_objects;
            loader_objects.push_back(bad_object);
            LoaderLogger::LogValidationErrorMessage("VUID-xrGetReferenceSpaceBoundsRect-session-parameter", "xrGetReferenceSpaceBoundsRect",
                                                    "session is not a valid XrSession", loader_objects);
            return XR_ERROR_HANDLE_INVALID;
        }
        const std::unique_ptr<XrGeneratedDispatchTable>& dispatch_table = loader_instance->DispatchTable();
        return dispatch_table->GetReferenceSpaceBoundsRect(session, referenceSpaceType, bounds);
    } catch (...) {
        LoaderLogger::LogErrorMessage("xrGetReferenceSpaceBoundsRect", "xrGetReferenceSpaceBoundsRect trampoline encountered an unknown error");
        // NOTE: Most calls only allow XR_SUCCESS as a return code
        return XR_SUCCESS;
    }
}


XRAPI_ATTR XrResult XRAPI_CALL xrCreateActionSpace(
    XrAction                                    action,
    const XrActionSpaceCreateInfo*              createInfo,
    XrSpace*                                    space) {
    try {
        std::unique_lock<std::mutex> secondary_lock(g_action_mutex);
        LoaderInstance *loader_instance = g_action_map[action];
        secondary_lock.unlock();
        if (nullptr == loader_instance) {
            XrLoaderLogObjectInfo bad_object = {};
            bad_object.type = XR_OBJECT_TYPE_ACTION;
            bad_object.handle = reinterpret_cast<uint64_t const&>(action);
            std::vector<XrLoaderLogObjectInfo> loader_objects;
            loader_objects.push_back(bad_object);
            LoaderLogger::LogValidationErrorMessage("VUID-xrCreateActionSpace-action-parameter", "xrCreateActionSpace",
                                                    "action is not a valid XrAction", loader_objects);
            return XR_ERROR_HANDLE_INVALID;
        }
        const std::unique_ptr<XrGeneratedDispatchTable>& dispatch_table = loader_instance->DispatchTable();
        XrResult result = XR_SUCCESS;
        result = dispatch_table->CreateActionSpace(action, createInfo, space);
        if (XR_SUCCESS == result && nullptr != space) {
            std::unique_lock<std::mutex> space_lock(g_space_mutex);
            auto exists = g_space_map.find(*space);
            if (exists == g_space_map.end()) {
                g_space_map[*space] = loader_instance;
            }
        }
        return result;
    } catch (std::bad_alloc &) {
        LoaderLogger::LogErrorMessage("xrCreateActionSpace", "xrCreateActionSpace trampoline failed allocating memory");
        return XR_ERROR_OUT_OF_MEMORY;
    } catch (...) {
        LoaderLogger::LogErrorMessage("xrCreateActionSpace", "xrCreateActionSpace trampoline encountered an unknown error");
        return XR_ERROR_INITIALIZATION_FAILED;
    }
}


XRAPI_ATTR XrResult XRAPI_CALL xrLocateSpace(
    XrSpace                                     space,
    XrSpace                                     baseSpace,
    XrTime                                      time,
    XrSpaceRelation*                            relation) {
    try {
        std::unique_lock<std::mutex> secondary_lock(g_space_mutex);
        LoaderInstance *loader_instance = g_space_map[space];
        secondary_lock.unlock();
        if (nullptr == loader_instance) {
            XrLoaderLogObjectInfo bad_object = {};
            bad_object.type = XR_OBJECT_TYPE_SPACE;
            bad_object.handle = reinterpret_cast<uint64_t const&>(space);
            std::vector<XrLoaderLogObjectInfo> loader_objects;
            loader_objects.push_back(bad_object);
            LoaderLogger::LogValidationErrorMessage("VUID-xrLocateSpace-space-parameter", "xrLocateSpace",
                                                    "space is not a valid XrSpace", loader_objects);
            return XR_ERROR_HANDLE_INVALID;
        }
        const std::unique_ptr<XrGeneratedDispatchTable>& dispatch_table = loader_instance->DispatchTable();
        return dispatch_table->LocateSpace(space, baseSpace, time, relation);
    } catch (...) {
        LoaderLogger::LogErrorMessage("xrLocateSpace", "xrLocateSpace trampoline encountered an unknown error");
        // NOTE: Most calls only allow XR_SUCCESS as a return code
        return XR_SUCCESS;
    }
}


XRAPI_ATTR XrResult XRAPI_CALL xrDestroySpace(
    XrSpace                                     space) {
    try {
        std::unique_lock<std::mutex> secondary_lock(g_space_mutex);
        LoaderInstance *loader_instance = g_space_map[space];
        // Destroy the mapping entry for this item if it was valid.
        if (nullptr != loader_instance) {
                g_space_map.erase(space);
        }
        secondary_lock.unlock();
        if (nullptr == loader_instance) {
            XrLoaderLogObjectInfo bad_object = {};
            bad_object.type = XR_OBJECT_TYPE_SPACE;
            bad_object.handle = reinterpret_cast<uint64_t const&>(space);
            std::vector<XrLoaderLogObjectInfo> loader_objects;
            loader_objects.push_back(bad_object);
            LoaderLogger::LogValidationErrorMessage("VUID-xrDestroySpace-space-parameter", "xrDestroySpace",
                                                    "space is not a valid XrSpace", loader_objects);
            return XR_ERROR_HANDLE_INVALID;
        }
        const std::unique_ptr<XrGeneratedDispatchTable>& dispatch_table = loader_instance->DispatchTable();
        XrResult result = XR_SUCCESS;
        result = dispatch_table->DestroySpace(space);
        return result;
    } catch (...) {
        LoaderLogger::LogErrorMessage("xrDestroySpace", "xrDestroySpace trampoline encountered an unknown error");
        // NOTE: Most calls only allow XR_SUCCESS as a return code
        return XR_SUCCESS;
    }
}


XRAPI_ATTR XrResult XRAPI_CALL xrEnumerateViewConfigurations(
    XrInstance                                  instance,
    XrSystemId                                  systemId,
    uint32_t                                    viewConfigurationTypeCapacityInput,
    uint32_t*                                   viewConfigurationTypeCountOutput,
    XrViewConfigurationType*                    viewConfigurationTypes) {
    try {
        std::unique_lock<std::mutex> secondary_lock(g_instance_mutex);
        LoaderInstance *loader_instance = g_instance_map[instance];
        secondary_lock.unlock();
        if (nullptr == loader_instance) {
            XrLoaderLogObjectInfo bad_object = {};
            bad_object.type = XR_OBJECT_TYPE_INSTANCE;
            bad_object.handle = reinterpret_cast<uint64_t const&>(instance);
            std::vector<XrLoaderLogObjectInfo> loader_objects;
            loader_objects.push_back(bad_object);
            LoaderLogger::LogValidationErrorMessage("VUID-xrEnumerateViewConfigurations-instance-parameter", "xrEnumerateViewConfigurations",
                                                    "instance is not a valid XrInstance", loader_objects);
            return XR_ERROR_HANDLE_INVALID;
        }
        const std::unique_ptr<XrGeneratedDispatchTable>& dispatch_table = loader_instance->DispatchTable();
        return dispatch_table->EnumerateViewConfigurations(instance, systemId, viewConfigurationTypeCapacityInput, viewConfigurationTypeCountOutput, viewConfigurationTypes);
    } catch (...) {
        std::string error_message = "xrEnumerateViewConfigurations trampoline encountered an unknown error.  Likely XrInstance 0x";
        std::ostringstream oss;
        oss << std::hex << reinterpret_cast<const void*>(instance);
        error_message += oss.str();
        error_message += " is invalid";
        LoaderLogger::LogErrorMessage("xrEnumerateViewConfigurations", error_message);
        return XR_ERROR_HANDLE_INVALID;
    }
}


XRAPI_ATTR XrResult XRAPI_CALL xrGetViewConfigurationProperties(
    XrInstance                                  instance,
    XrSystemId                                  systemId,
    XrViewConfigurationType                     viewConfigurationType,
    XrViewConfigurationProperties*              configurationProperties) {
    try {
        std::unique_lock<std::mutex> secondary_lock(g_instance_mutex);
        LoaderInstance *loader_instance = g_instance_map[instance];
        secondary_lock.unlock();
        if (nullptr == loader_instance) {
            XrLoaderLogObjectInfo bad_object = {};
            bad_object.type = XR_OBJECT_TYPE_INSTANCE;
            bad_object.handle = reinterpret_cast<uint64_t const&>(instance);
            std::vector<XrLoaderLogObjectInfo> loader_objects;
            loader_objects.push_back(bad_object);
            LoaderLogger::LogValidationErrorMessage("VUID-xrGetViewConfigurationProperties-instance-parameter", "xrGetViewConfigurationProperties",
                                                    "instance is not a valid XrInstance", loader_objects);
            return XR_ERROR_HANDLE_INVALID;
        }
        const std::unique_ptr<XrGeneratedDispatchTable>& dispatch_table = loader_instance->DispatchTable();
        return dispatch_table->GetViewConfigurationProperties(instance, systemId, viewConfigurationType, configurationProperties);
    } catch (...) {
        std::string error_message = "xrGetViewConfigurationProperties trampoline encountered an unknown error.  Likely XrInstance 0x";
        std::ostringstream oss;
        oss << std::hex << reinterpret_cast<const void*>(instance);
        error_message += oss.str();
        error_message += " is invalid";
        LoaderLogger::LogErrorMessage("xrGetViewConfigurationProperties", error_message);
        return XR_ERROR_HANDLE_INVALID;
    }
}


XRAPI_ATTR XrResult XRAPI_CALL xrEnumerateViewConfigurationViews(
    XrInstance                                  instance,
    XrSystemId                                  systemId,
    XrViewConfigurationType                     viewConfigurationType,
    uint32_t                                    viewCapacityInput,
    uint32_t*                                   viewCountOutput,
    XrViewConfigurationView*                    views) {
    try {
        std::unique_lock<std::mutex> secondary_lock(g_instance_mutex);
        LoaderInstance *loader_instance = g_instance_map[instance];
        secondary_lock.unlock();
        if (nullptr == loader_instance) {
            XrLoaderLogObjectInfo bad_object = {};
            bad_object.type = XR_OBJECT_TYPE_INSTANCE;
            bad_object.handle = reinterpret_cast<uint64_t const&>(instance);
            std::vector<XrLoaderLogObjectInfo> loader_objects;
            loader_objects.push_back(bad_object);
            LoaderLogger::LogValidationErrorMessage("VUID-xrEnumerateViewConfigurationViews-instance-parameter", "xrEnumerateViewConfigurationViews",
                                                    "instance is not a valid XrInstance", loader_objects);
            return XR_ERROR_HANDLE_INVALID;
        }
        const std::unique_ptr<XrGeneratedDispatchTable>& dispatch_table = loader_instance->DispatchTable();
        return dispatch_table->EnumerateViewConfigurationViews(instance, systemId, viewConfigurationType, viewCapacityInput, viewCountOutput, views);
    } catch (...) {
        std::string error_message = "xrEnumerateViewConfigurationViews trampoline encountered an unknown error.  Likely XrInstance 0x";
        std::ostringstream oss;
        oss << std::hex << reinterpret_cast<const void*>(instance);
        error_message += oss.str();
        error_message += " is invalid";
        LoaderLogger::LogErrorMessage("xrEnumerateViewConfigurationViews", error_message);
        return XR_ERROR_HANDLE_INVALID;
    }
}


XRAPI_ATTR XrResult XRAPI_CALL xrEnumerateSwapchainFormats(
    XrSession                                   session,
    uint32_t                                    formatCapacityInput,
    uint32_t*                                   formatCountOutput,
    int64_t*                                    formats) {
    try {
        std::unique_lock<std::mutex> secondary_lock(g_session_mutex);
        LoaderInstance *loader_instance = g_session_map[session];
        secondary_lock.unlock();
        if (nullptr == loader_instance) {
            XrLoaderLogObjectInfo bad_object = {};
            bad_object.type = XR_OBJECT_TYPE_SESSION;
            bad_object.handle = reinterpret_cast<uint64_t const&>(session);
            std::vector<XrLoaderLogObjectInfo> loader_objects;
            loader_objects.push_back(bad_object);
            LoaderLogger::LogValidationErrorMessage("VUID-xrEnumerateSwapchainFormats-session-parameter", "xrEnumerateSwapchainFormats",
                                                    "session is not a valid XrSession", loader_objects);
            return XR_ERROR_HANDLE_INVALID;
        }
        const std::unique_ptr<XrGeneratedDispatchTable>& dispatch_table = loader_instance->DispatchTable();
        return dispatch_table->EnumerateSwapchainFormats(session, formatCapacityInput, formatCountOutput, formats);
    } catch (...) {
        LoaderLogger::LogErrorMessage("xrEnumerateSwapchainFormats", "xrEnumerateSwapchainFormats trampoline encountered an unknown error");
        // NOTE: Most calls only allow XR_SUCCESS as a return code
        return XR_SUCCESS;
    }
}


XRAPI_ATTR XrResult XRAPI_CALL xrCreateSwapchain(
    XrSession                                   session,
    const XrSwapchainCreateInfo*                createInfo,
    XrSwapchain*                                swapchain) {
    try {
        std::unique_lock<std::mutex> secondary_lock(g_session_mutex);
        LoaderInstance *loader_instance = g_session_map[session];
        secondary_lock.unlock();
        if (nullptr == loader_instance) {
            XrLoaderLogObjectInfo bad_object = {};
            bad_object.type = XR_OBJECT_TYPE_SESSION;
            bad_object.handle = reinterpret_cast<uint64_t const&>(session);
            std::vector<XrLoaderLogObjectInfo> loader_objects;
            loader_objects.push_back(bad_object);
            LoaderLogger::LogValidationErrorMessage("VUID-xrCreateSwapchain-session-parameter", "xrCreateSwapchain",
                                                    "session is not a valid XrSession", loader_objects);
            return XR_ERROR_HANDLE_INVALID;
        }
        const std::unique_ptr<XrGeneratedDispatchTable>& dispatch_table = loader_instance->DispatchTable();
        XrResult result = XR_SUCCESS;
        result = dispatch_table->CreateSwapchain(session, createInfo, swapchain);
        if (XR_SUCCESS == result && nullptr != swapchain) {
            std::unique_lock<std::mutex> swapchain_lock(g_swapchain_mutex);
            auto exists = g_swapchain_map.find(*swapchain);
            if (exists == g_swapchain_map.end()) {
                g_swapchain_map[*swapchain] = loader_instance;
            }
        }
        return result;
    } catch (std::bad_alloc &) {
        LoaderLogger::LogErrorMessage("xrCreateSwapchain", "xrCreateSwapchain trampoline failed allocating memory");
        return XR_ERROR_OUT_OF_MEMORY;
    } catch (...) {
        LoaderLogger::LogErrorMessage("xrCreateSwapchain", "xrCreateSwapchain trampoline encountered an unknown error");
        return XR_ERROR_INITIALIZATION_FAILED;
    }
}


XRAPI_ATTR XrResult XRAPI_CALL xrDestroySwapchain(
    XrSwapchain                                 swapchain) {
    try {
        std::unique_lock<std::mutex> secondary_lock(g_swapchain_mutex);
        LoaderInstance *loader_instance = g_swapchain_map[swapchain];
        // Destroy the mapping entry for this item if it was valid.
        if (nullptr != loader_instance) {
                g_swapchain_map.erase(swapchain);
        }
        secondary_lock.unlock();
        if (nullptr == loader_instance) {
            XrLoaderLogObjectInfo bad_object = {};
            bad_object.type = XR_OBJECT_TYPE_SWAPCHAIN;
            bad_object.handle = reinterpret_cast<uint64_t const&>(swapchain);
            std::vector<XrLoaderLogObjectInfo> loader_objects;
            loader_objects.push_back(bad_object);
            LoaderLogger::LogValidationErrorMessage("VUID-xrDestroySwapchain-swapchain-parameter", "xrDestroySwapchain",
                                                    "swapchain is not a valid XrSwapchain", loader_objects);
            return XR_ERROR_HANDLE_INVALID;
        }
        const std::unique_ptr<XrGeneratedDispatchTable>& dispatch_table = loader_instance->DispatchTable();
        XrResult result = XR_SUCCESS;
        result = dispatch_table->DestroySwapchain(swapchain);
        return result;
    } catch (...) {
        LoaderLogger::LogErrorMessage("xrDestroySwapchain", "xrDestroySwapchain trampoline encountered an unknown error");
        // NOTE: Most calls only allow XR_SUCCESS as a return code
        return XR_SUCCESS;
    }
}


XRAPI_ATTR XrResult XRAPI_CALL xrEnumerateSwapchainImages(
    XrSwapchain                                 swapchain,
    uint32_t                                    imageCapacityInput,
    uint32_t*                                   imageCountOutput,
    XrSwapchainImageBaseHeader*                 images) {
    try {
        std::unique_lock<std::mutex> secondary_lock(g_swapchain_mutex);
        LoaderInstance *loader_instance = g_swapchain_map[swapchain];
        secondary_lock.unlock();
        if (nullptr == loader_instance) {
            XrLoaderLogObjectInfo bad_object = {};
            bad_object.type = XR_OBJECT_TYPE_SWAPCHAIN;
            bad_object.handle = reinterpret_cast<uint64_t const&>(swapchain);
            std::vector<XrLoaderLogObjectInfo> loader_objects;
            loader_objects.push_back(bad_object);
            LoaderLogger::LogValidationErrorMessage("VUID-xrEnumerateSwapchainImages-swapchain-parameter", "xrEnumerateSwapchainImages",
                                                    "swapchain is not a valid XrSwapchain", loader_objects);
            return XR_ERROR_HANDLE_INVALID;
        }
        const std::unique_ptr<XrGeneratedDispatchTable>& dispatch_table = loader_instance->DispatchTable();
        return dispatch_table->EnumerateSwapchainImages(swapchain, imageCapacityInput, imageCountOutput, images);
    } catch (...) {
        LoaderLogger::LogErrorMessage("xrEnumerateSwapchainImages", "xrEnumerateSwapchainImages trampoline encountered an unknown error");
        // NOTE: Most calls only allow XR_SUCCESS as a return code
        return XR_SUCCESS;
    }
}


XRAPI_ATTR XrResult XRAPI_CALL xrAcquireSwapchainImage(
    XrSwapchain                                 swapchain,
    const XrSwapchainImageAcquireInfo*          acquireInfo,
    uint32_t*                                   index) {
    try {
        std::unique_lock<std::mutex> secondary_lock(g_swapchain_mutex);
        LoaderInstance *loader_instance = g_swapchain_map[swapchain];
        secondary_lock.unlock();
        if (nullptr == loader_instance) {
            XrLoaderLogObjectInfo bad_object = {};
            bad_object.type = XR_OBJECT_TYPE_SWAPCHAIN;
            bad_object.handle = reinterpret_cast<uint64_t const&>(swapchain);
            std::vector<XrLoaderLogObjectInfo> loader_objects;
            loader_objects.push_back(bad_object);
            LoaderLogger::LogValidationErrorMessage("VUID-xrAcquireSwapchainImage-swapchain-parameter", "xrAcquireSwapchainImage",
                                                    "swapchain is not a valid XrSwapchain", loader_objects);
            return XR_ERROR_HANDLE_INVALID;
        }
        const std::unique_ptr<XrGeneratedDispatchTable>& dispatch_table = loader_instance->DispatchTable();
        return dispatch_table->AcquireSwapchainImage(swapchain, acquireInfo, index);
    } catch (...) {
        LoaderLogger::LogErrorMessage("xrAcquireSwapchainImage", "xrAcquireSwapchainImage trampoline encountered an unknown error");
        // NOTE: Most calls only allow XR_SUCCESS as a return code
        return XR_SUCCESS;
    }
}


XRAPI_ATTR XrResult XRAPI_CALL xrWaitSwapchainImage(
    XrSwapchain                                 swapchain,
    const XrSwapchainImageWaitInfo*             waitInfo) {
    try {
        std::unique_lock<std::mutex> secondary_lock(g_swapchain_mutex);
        LoaderInstance *loader_instance = g_swapchain_map[swapchain];
        secondary_lock.unlock();
        if (nullptr == loader_instance) {
            XrLoaderLogObjectInfo bad_object = {};
            bad_object.type = XR_OBJECT_TYPE_SWAPCHAIN;
            bad_object.handle = reinterpret_cast<uint64_t const&>(swapchain);
            std::vector<XrLoaderLogObjectInfo> loader_objects;
            loader_objects.push_back(bad_object);
            LoaderLogger::LogValidationErrorMessage("VUID-xrWaitSwapchainImage-swapchain-parameter", "xrWaitSwapchainImage",
                                                    "swapchain is not a valid XrSwapchain", loader_objects);
            return XR_ERROR_HANDLE_INVALID;
        }
        const std::unique_ptr<XrGeneratedDispatchTable>& dispatch_table = loader_instance->DispatchTable();
        return dispatch_table->WaitSwapchainImage(swapchain, waitInfo);
    } catch (...) {
        LoaderLogger::LogErrorMessage("xrWaitSwapchainImage", "xrWaitSwapchainImage trampoline encountered an unknown error");
        // NOTE: Most calls only allow XR_SUCCESS as a return code
        return XR_SUCCESS;
    }
}


XRAPI_ATTR XrResult XRAPI_CALL xrReleaseSwapchainImage(
    XrSwapchain                                 swapchain,
    const XrSwapchainImageReleaseInfo*          releaseInfo) {
    try {
        std::unique_lock<std::mutex> secondary_lock(g_swapchain_mutex);
        LoaderInstance *loader_instance = g_swapchain_map[swapchain];
        secondary_lock.unlock();
        if (nullptr == loader_instance) {
            XrLoaderLogObjectInfo bad_object = {};
            bad_object.type = XR_OBJECT_TYPE_SWAPCHAIN;
            bad_object.handle = reinterpret_cast<uint64_t const&>(swapchain);
            std::vector<XrLoaderLogObjectInfo> loader_objects;
            loader_objects.push_back(bad_object);
            LoaderLogger::LogValidationErrorMessage("VUID-xrReleaseSwapchainImage-swapchain-parameter", "xrReleaseSwapchainImage",
                                                    "swapchain is not a valid XrSwapchain", loader_objects);
            return XR_ERROR_HANDLE_INVALID;
        }
        const std::unique_ptr<XrGeneratedDispatchTable>& dispatch_table = loader_instance->DispatchTable();
        return dispatch_table->ReleaseSwapchainImage(swapchain, releaseInfo);
    } catch (...) {
        LoaderLogger::LogErrorMessage("xrReleaseSwapchainImage", "xrReleaseSwapchainImage trampoline encountered an unknown error");
        // NOTE: Most calls only allow XR_SUCCESS as a return code
        return XR_SUCCESS;
    }
}


XRAPI_ATTR XrResult XRAPI_CALL xrBeginSession(
    XrSession                                   session,
    const XrSessionBeginInfo*                   beginInfo) {
    try {
        std::unique_lock<std::mutex> secondary_lock(g_session_mutex);
        LoaderInstance *loader_instance = g_session_map[session];
        secondary_lock.unlock();
        if (nullptr == loader_instance) {
            XrLoaderLogObjectInfo bad_object = {};
            bad_object.type = XR_OBJECT_TYPE_SESSION;
            bad_object.handle = reinterpret_cast<uint64_t const&>(session);
            std::vector<XrLoaderLogObjectInfo> loader_objects;
            loader_objects.push_back(bad_object);
            LoaderLogger::LogValidationErrorMessage("VUID-xrBeginSession-session-parameter", "xrBeginSession",
                                                    "session is not a valid XrSession", loader_objects);
            return XR_ERROR_HANDLE_INVALID;
        }
        const std::unique_ptr<XrGeneratedDispatchTable>& dispatch_table = loader_instance->DispatchTable();
        return dispatch_table->BeginSession(session, beginInfo);
    } catch (...) {
        LoaderLogger::LogErrorMessage("xrBeginSession", "xrBeginSession trampoline encountered an unknown error");
        // NOTE: Most calls only allow XR_SUCCESS as a return code
        return XR_SUCCESS;
    }
}


XRAPI_ATTR XrResult XRAPI_CALL xrEndSession(
    XrSession                                   session) {
    try {
        std::unique_lock<std::mutex> secondary_lock(g_session_mutex);
        LoaderInstance *loader_instance = g_session_map[session];
        secondary_lock.unlock();
        if (nullptr == loader_instance) {
            XrLoaderLogObjectInfo bad_object = {};
            bad_object.type = XR_OBJECT_TYPE_SESSION;
            bad_object.handle = reinterpret_cast<uint64_t const&>(session);
            std::vector<XrLoaderLogObjectInfo> loader_objects;
            loader_objects.push_back(bad_object);
            LoaderLogger::LogValidationErrorMessage("VUID-xrEndSession-session-parameter", "xrEndSession",
                                                    "session is not a valid XrSession", loader_objects);
            return XR_ERROR_HANDLE_INVALID;
        }
        const std::unique_ptr<XrGeneratedDispatchTable>& dispatch_table = loader_instance->DispatchTable();
        return dispatch_table->EndSession(session);
    } catch (...) {
        LoaderLogger::LogErrorMessage("xrEndSession", "xrEndSession trampoline encountered an unknown error");
        // NOTE: Most calls only allow XR_SUCCESS as a return code
        return XR_SUCCESS;
    }
}


XRAPI_ATTR XrResult XRAPI_CALL xrWaitFrame(
    XrSession                                   session,
    const XrFrameWaitInfo*                      frameWaitInfo,
    XrFrameState*                               frameState) {
    try {
        std::unique_lock<std::mutex> secondary_lock(g_session_mutex);
        LoaderInstance *loader_instance = g_session_map[session];
        secondary_lock.unlock();
        if (nullptr == loader_instance) {
            XrLoaderLogObjectInfo bad_object = {};
            bad_object.type = XR_OBJECT_TYPE_SESSION;
            bad_object.handle = reinterpret_cast<uint64_t const&>(session);
            std::vector<XrLoaderLogObjectInfo> loader_objects;
            loader_objects.push_back(bad_object);
            LoaderLogger::LogValidationErrorMessage("VUID-xrWaitFrame-session-parameter", "xrWaitFrame",
                                                    "session is not a valid XrSession", loader_objects);
            return XR_ERROR_HANDLE_INVALID;
        }
        const std::unique_ptr<XrGeneratedDispatchTable>& dispatch_table = loader_instance->DispatchTable();
        return dispatch_table->WaitFrame(session, frameWaitInfo, frameState);
    } catch (...) {
        LoaderLogger::LogErrorMessage("xrWaitFrame", "xrWaitFrame trampoline encountered an unknown error");
        // NOTE: Most calls only allow XR_SUCCESS as a return code
        return XR_SUCCESS;
    }
}


XRAPI_ATTR XrResult XRAPI_CALL xrBeginFrame(
    XrSession                                   session,
    const XrFrameBeginInfo*                     frameBeginInfo) {
    try {
        std::unique_lock<std::mutex> secondary_lock(g_session_mutex);
        LoaderInstance *loader_instance = g_session_map[session];
        secondary_lock.unlock();
        if (nullptr == loader_instance) {
            XrLoaderLogObjectInfo bad_object = {};
            bad_object.type = XR_OBJECT_TYPE_SESSION;
            bad_object.handle = reinterpret_cast<uint64_t const&>(session);
            std::vector<XrLoaderLogObjectInfo> loader_objects;
            loader_objects.push_back(bad_object);
            LoaderLogger::LogValidationErrorMessage("VUID-xrBeginFrame-session-parameter", "xrBeginFrame",
                                                    "session is not a valid XrSession", loader_objects);
            return XR_ERROR_HANDLE_INVALID;
        }
        const std::unique_ptr<XrGeneratedDispatchTable>& dispatch_table = loader_instance->DispatchTable();
        return dispatch_table->BeginFrame(session, frameBeginInfo);
    } catch (...) {
        LoaderLogger::LogErrorMessage("xrBeginFrame", "xrBeginFrame trampoline encountered an unknown error");
        // NOTE: Most calls only allow XR_SUCCESS as a return code
        return XR_SUCCESS;
    }
}


XRAPI_ATTR XrResult XRAPI_CALL xrEndFrame(
    XrSession                                   session,
    const XrFrameEndInfo*                       frameEndInfo) {
    try {
        std::unique_lock<std::mutex> secondary_lock(g_session_mutex);
        LoaderInstance *loader_instance = g_session_map[session];
        secondary_lock.unlock();
        if (nullptr == loader_instance) {
            XrLoaderLogObjectInfo bad_object = {};
            bad_object.type = XR_OBJECT_TYPE_SESSION;
            bad_object.handle = reinterpret_cast<uint64_t const&>(session);
            std::vector<XrLoaderLogObjectInfo> loader_objects;
            loader_objects.push_back(bad_object);
            LoaderLogger::LogValidationErrorMessage("VUID-xrEndFrame-session-parameter", "xrEndFrame",
                                                    "session is not a valid XrSession", loader_objects);
            return XR_ERROR_HANDLE_INVALID;
        }
        const std::unique_ptr<XrGeneratedDispatchTable>& dispatch_table = loader_instance->DispatchTable();
        return dispatch_table->EndFrame(session, frameEndInfo);
    } catch (...) {
        LoaderLogger::LogErrorMessage("xrEndFrame", "xrEndFrame trampoline encountered an unknown error");
        // NOTE: Most calls only allow XR_SUCCESS as a return code
        return XR_SUCCESS;
    }
}


XRAPI_ATTR XrResult XRAPI_CALL xrLocateViews(
    XrSession                                   session,
    const XrViewLocateInfo*                     viewLocateInfo,
    XrViewState*                                viewState,
    uint32_t                                    viewCapacityInput,
    uint32_t*                                   viewCountOutput,
    XrView*                                     views) {
    try {
        std::unique_lock<std::mutex> secondary_lock(g_session_mutex);
        LoaderInstance *loader_instance = g_session_map[session];
        secondary_lock.unlock();
        if (nullptr == loader_instance) {
            XrLoaderLogObjectInfo bad_object = {};
            bad_object.type = XR_OBJECT_TYPE_SESSION;
            bad_object.handle = reinterpret_cast<uint64_t const&>(session);
            std::vector<XrLoaderLogObjectInfo> loader_objects;
            loader_objects.push_back(bad_object);
            LoaderLogger::LogValidationErrorMessage("VUID-xrLocateViews-session-parameter", "xrLocateViews",
                                                    "session is not a valid XrSession", loader_objects);
            return XR_ERROR_HANDLE_INVALID;
        }
        const std::unique_ptr<XrGeneratedDispatchTable>& dispatch_table = loader_instance->DispatchTable();
        return dispatch_table->LocateViews(session, viewLocateInfo, viewState, viewCapacityInput, viewCountOutput, views);
    } catch (...) {
        LoaderLogger::LogErrorMessage("xrLocateViews", "xrLocateViews trampoline encountered an unknown error");
        // NOTE: Most calls only allow XR_SUCCESS as a return code
        return XR_SUCCESS;
    }
}


XRAPI_ATTR XrResult XRAPI_CALL xrStringToPath(
    XrInstance                                  instance,
    const char*                                 pathString,
    XrPath*                                     path) {
    try {
        std::unique_lock<std::mutex> secondary_lock(g_instance_mutex);
        LoaderInstance *loader_instance = g_instance_map[instance];
        secondary_lock.unlock();
        if (nullptr == loader_instance) {
            XrLoaderLogObjectInfo bad_object = {};
            bad_object.type = XR_OBJECT_TYPE_INSTANCE;
            bad_object.handle = reinterpret_cast<uint64_t const&>(instance);
            std::vector<XrLoaderLogObjectInfo> loader_objects;
            loader_objects.push_back(bad_object);
            LoaderLogger::LogValidationErrorMessage("VUID-xrStringToPath-instance-parameter", "xrStringToPath",
                                                    "instance is not a valid XrInstance", loader_objects);
            return XR_ERROR_HANDLE_INVALID;
        }
        const std::unique_ptr<XrGeneratedDispatchTable>& dispatch_table = loader_instance->DispatchTable();
        return dispatch_table->StringToPath(instance, pathString, path);
    } catch (...) {
        std::string error_message = "xrStringToPath trampoline encountered an unknown error.  Likely XrInstance 0x";
        std::ostringstream oss;
        oss << std::hex << reinterpret_cast<const void*>(instance);
        error_message += oss.str();
        error_message += " is invalid";
        LoaderLogger::LogErrorMessage("xrStringToPath", error_message);
        return XR_ERROR_HANDLE_INVALID;
    }
}


XRAPI_ATTR XrResult XRAPI_CALL xrPathToString(
    XrInstance                                  instance,
    XrPath                                      path,
    uint32_t                                    bufferCapacityInput,
    uint32_t*                                   bufferCountOutput,
    char*                                       buffer) {
    try {
        std::unique_lock<std::mutex> secondary_lock(g_instance_mutex);
        LoaderInstance *loader_instance = g_instance_map[instance];
        secondary_lock.unlock();
        if (nullptr == loader_instance) {
            XrLoaderLogObjectInfo bad_object = {};
            bad_object.type = XR_OBJECT_TYPE_INSTANCE;
            bad_object.handle = reinterpret_cast<uint64_t const&>(instance);
            std::vector<XrLoaderLogObjectInfo> loader_objects;
            loader_objects.push_back(bad_object);
            LoaderLogger::LogValidationErrorMessage("VUID-xrPathToString-instance-parameter", "xrPathToString",
                                                    "instance is not a valid XrInstance", loader_objects);
            return XR_ERROR_HANDLE_INVALID;
        }
        const std::unique_ptr<XrGeneratedDispatchTable>& dispatch_table = loader_instance->DispatchTable();
        return dispatch_table->PathToString(instance, path, bufferCapacityInput, bufferCountOutput, buffer);
    } catch (...) {
        std::string error_message = "xrPathToString trampoline encountered an unknown error.  Likely XrInstance 0x";
        std::ostringstream oss;
        oss << std::hex << reinterpret_cast<const void*>(instance);
        error_message += oss.str();
        error_message += " is invalid";
        LoaderLogger::LogErrorMessage("xrPathToString", error_message);
        return XR_ERROR_HANDLE_INVALID;
    }
}


XRAPI_ATTR XrResult XRAPI_CALL xrCreateActionSet(
    XrSession                                   session,
    const XrActionSetCreateInfo*                createInfo,
    XrActionSet*                                actionSet) {
    try {
        std::unique_lock<std::mutex> secondary_lock(g_session_mutex);
        LoaderInstance *loader_instance = g_session_map[session];
        secondary_lock.unlock();
        if (nullptr == loader_instance) {
            XrLoaderLogObjectInfo bad_object = {};
            bad_object.type = XR_OBJECT_TYPE_SESSION;
            bad_object.handle = reinterpret_cast<uint64_t const&>(session);
            std::vector<XrLoaderLogObjectInfo> loader_objects;
            loader_objects.push_back(bad_object);
            LoaderLogger::LogValidationErrorMessage("VUID-xrCreateActionSet-session-parameter", "xrCreateActionSet",
                                                    "session is not a valid XrSession", loader_objects);
            return XR_ERROR_HANDLE_INVALID;
        }
        const std::unique_ptr<XrGeneratedDispatchTable>& dispatch_table = loader_instance->DispatchTable();
        XrResult result = XR_SUCCESS;
        result = dispatch_table->CreateActionSet(session, createInfo, actionSet);
        if (XR_SUCCESS == result && nullptr != actionSet) {
            std::unique_lock<std::mutex> actionset_lock(g_actionset_mutex);
            auto exists = g_actionset_map.find(*actionSet);
            if (exists == g_actionset_map.end()) {
                g_actionset_map[*actionSet] = loader_instance;
            }
        }
        return result;
    } catch (std::bad_alloc &) {
        LoaderLogger::LogErrorMessage("xrCreateActionSet", "xrCreateActionSet trampoline failed allocating memory");
        return XR_ERROR_OUT_OF_MEMORY;
    } catch (...) {
        LoaderLogger::LogErrorMessage("xrCreateActionSet", "xrCreateActionSet trampoline encountered an unknown error");
        return XR_ERROR_INITIALIZATION_FAILED;
    }
}


XRAPI_ATTR XrResult XRAPI_CALL xrDestroyActionSet(
    XrActionSet                                 actionSet) {
    try {
        std::unique_lock<std::mutex> secondary_lock(g_actionset_mutex);
        LoaderInstance *loader_instance = g_actionset_map[actionSet];
        // Destroy the mapping entry for this item if it was valid.
        if (nullptr != loader_instance) {
                g_actionset_map.erase(actionSet);
        }
        secondary_lock.unlock();
        if (nullptr == loader_instance) {
            XrLoaderLogObjectInfo bad_object = {};
            bad_object.type = XR_OBJECT_TYPE_ACTION_SET;
            bad_object.handle = reinterpret_cast<uint64_t const&>(actionSet);
            std::vector<XrLoaderLogObjectInfo> loader_objects;
            loader_objects.push_back(bad_object);
            LoaderLogger::LogValidationErrorMessage("VUID-xrDestroyActionSet-actionSet-parameter", "xrDestroyActionSet",
                                                    "actionSet is not a valid XrActionSet", loader_objects);
            return XR_ERROR_HANDLE_INVALID;
        }
        const std::unique_ptr<XrGeneratedDispatchTable>& dispatch_table = loader_instance->DispatchTable();
        XrResult result = XR_SUCCESS;
        result = dispatch_table->DestroyActionSet(actionSet);
        return result;
    } catch (...) {
        LoaderLogger::LogErrorMessage("xrDestroyActionSet", "xrDestroyActionSet trampoline encountered an unknown error");
        // NOTE: Most calls only allow XR_SUCCESS as a return code
        return XR_SUCCESS;
    }
}


XRAPI_ATTR XrResult XRAPI_CALL xrCreateAction(
    XrActionSet                                 actionSet,
    const XrActionCreateInfo*                   createInfo,
    XrAction*                                   action) {
    try {
        std::unique_lock<std::mutex> secondary_lock(g_actionset_mutex);
        LoaderInstance *loader_instance = g_actionset_map[actionSet];
        secondary_lock.unlock();
        if (nullptr == loader_instance) {
            XrLoaderLogObjectInfo bad_object = {};
            bad_object.type = XR_OBJECT_TYPE_ACTION_SET;
            bad_object.handle = reinterpret_cast<uint64_t const&>(actionSet);
            std::vector<XrLoaderLogObjectInfo> loader_objects;
            loader_objects.push_back(bad_object);
            LoaderLogger::LogValidationErrorMessage("VUID-xrCreateAction-actionSet-parameter", "xrCreateAction",
                                                    "actionSet is not a valid XrActionSet", loader_objects);
            return XR_ERROR_HANDLE_INVALID;
        }
        const std::unique_ptr<XrGeneratedDispatchTable>& dispatch_table = loader_instance->DispatchTable();
        XrResult result = XR_SUCCESS;
        result = dispatch_table->CreateAction(actionSet, createInfo, action);
        if (XR_SUCCESS == result && nullptr != action) {
            std::unique_lock<std::mutex> action_lock(g_action_mutex);
            auto exists = g_action_map.find(*action);
            if (exists == g_action_map.end()) {
                g_action_map[*action] = loader_instance;
            }
        }
        return result;
    } catch (std::bad_alloc &) {
        LoaderLogger::LogErrorMessage("xrCreateAction", "xrCreateAction trampoline failed allocating memory");
        return XR_ERROR_OUT_OF_MEMORY;
    } catch (...) {
        LoaderLogger::LogErrorMessage("xrCreateAction", "xrCreateAction trampoline encountered an unknown error");
        return XR_ERROR_INITIALIZATION_FAILED;
    }
}


XRAPI_ATTR XrResult XRAPI_CALL xrDestroyAction(
    XrAction                                    action) {
    try {
        std::unique_lock<std::mutex> secondary_lock(g_action_mutex);
        LoaderInstance *loader_instance = g_action_map[action];
        // Destroy the mapping entry for this item if it was valid.
        if (nullptr != loader_instance) {
                g_action_map.erase(action);
        }
        secondary_lock.unlock();
        if (nullptr == loader_instance) {
            XrLoaderLogObjectInfo bad_object = {};
            bad_object.type = XR_OBJECT_TYPE_ACTION;
            bad_object.handle = reinterpret_cast<uint64_t const&>(action);
            std::vector<XrLoaderLogObjectInfo> loader_objects;
            loader_objects.push_back(bad_object);
            LoaderLogger::LogValidationErrorMessage("VUID-xrDestroyAction-action-parameter", "xrDestroyAction",
                                                    "action is not a valid XrAction", loader_objects);
            return XR_ERROR_HANDLE_INVALID;
        }
        const std::unique_ptr<XrGeneratedDispatchTable>& dispatch_table = loader_instance->DispatchTable();
        XrResult result = XR_SUCCESS;
        result = dispatch_table->DestroyAction(action);
        return result;
    } catch (...) {
        LoaderLogger::LogErrorMessage("xrDestroyAction", "xrDestroyAction trampoline encountered an unknown error");
        // NOTE: Most calls only allow XR_SUCCESS as a return code
        return XR_SUCCESS;
    }
}


XRAPI_ATTR XrResult XRAPI_CALL xrSetInteractionProfileSuggestedBindings(
    XrSession                                   session,
    const XrInteractionProfileSuggestedBinding* suggestedBindings) {
    try {
        std::unique_lock<std::mutex> secondary_lock(g_session_mutex);
        LoaderInstance *loader_instance = g_session_map[session];
        secondary_lock.unlock();
        if (nullptr == loader_instance) {
            XrLoaderLogObjectInfo bad_object = {};
            bad_object.type = XR_OBJECT_TYPE_SESSION;
            bad_object.handle = reinterpret_cast<uint64_t const&>(session);
            std::vector<XrLoaderLogObjectInfo> loader_objects;
            loader_objects.push_back(bad_object);
            LoaderLogger::LogValidationErrorMessage("VUID-xrSetInteractionProfileSuggestedBindings-session-parameter", "xrSetInteractionProfileSuggestedBindings",
                                                    "session is not a valid XrSession", loader_objects);
            return XR_ERROR_HANDLE_INVALID;
        }
        const std::unique_ptr<XrGeneratedDispatchTable>& dispatch_table = loader_instance->DispatchTable();
        return dispatch_table->SetInteractionProfileSuggestedBindings(session, suggestedBindings);
    } catch (...) {
        LoaderLogger::LogErrorMessage("xrSetInteractionProfileSuggestedBindings", "xrSetInteractionProfileSuggestedBindings trampoline encountered an unknown error");
        // NOTE: Most calls only allow XR_SUCCESS as a return code
        return XR_SUCCESS;
    }
}


XRAPI_ATTR XrResult XRAPI_CALL xrGetCurrentInteractionProfile(
    XrSession                                   session,
    XrPath                                      topLevelUserPath,
    XrInteractionProfileInfo*                   interactionProfile) {
    try {
        std::unique_lock<std::mutex> secondary_lock(g_session_mutex);
        LoaderInstance *loader_instance = g_session_map[session];
        secondary_lock.unlock();
        if (nullptr == loader_instance) {
            XrLoaderLogObjectInfo bad_object = {};
            bad_object.type = XR_OBJECT_TYPE_SESSION;
            bad_object.handle = reinterpret_cast<uint64_t const&>(session);
            std::vector<XrLoaderLogObjectInfo> loader_objects;
            loader_objects.push_back(bad_object);
            LoaderLogger::LogValidationErrorMessage("VUID-xrGetCurrentInteractionProfile-session-parameter", "xrGetCurrentInteractionProfile",
                                                    "session is not a valid XrSession", loader_objects);
            return XR_ERROR_HANDLE_INVALID;
        }
        const std::unique_ptr<XrGeneratedDispatchTable>& dispatch_table = loader_instance->DispatchTable();
        return dispatch_table->GetCurrentInteractionProfile(session, topLevelUserPath, interactionProfile);
    } catch (...) {
        LoaderLogger::LogErrorMessage("xrGetCurrentInteractionProfile", "xrGetCurrentInteractionProfile trampoline encountered an unknown error");
        // NOTE: Most calls only allow XR_SUCCESS as a return code
        return XR_SUCCESS;
    }
}


XRAPI_ATTR XrResult XRAPI_CALL xrGetActionStateBoolean(
    XrAction                                    action,
    uint32_t                                    countSubactionPaths,
    const XrPath*                               subactionPaths,
    XrActionStateBoolean*                       data) {
    try {
        std::unique_lock<std::mutex> secondary_lock(g_action_mutex);
        LoaderInstance *loader_instance = g_action_map[action];
        secondary_lock.unlock();
        if (nullptr == loader_instance) {
            XrLoaderLogObjectInfo bad_object = {};
            bad_object.type = XR_OBJECT_TYPE_ACTION;
            bad_object.handle = reinterpret_cast<uint64_t const&>(action);
            std::vector<XrLoaderLogObjectInfo> loader_objects;
            loader_objects.push_back(bad_object);
            LoaderLogger::LogValidationErrorMessage("VUID-xrGetActionStateBoolean-action-parameter", "xrGetActionStateBoolean",
                                                    "action is not a valid XrAction", loader_objects);
            return XR_ERROR_HANDLE_INVALID;
        }
        const std::unique_ptr<XrGeneratedDispatchTable>& dispatch_table = loader_instance->DispatchTable();
        return dispatch_table->GetActionStateBoolean(action, countSubactionPaths, subactionPaths, data);
    } catch (...) {
        LoaderLogger::LogErrorMessage("xrGetActionStateBoolean", "xrGetActionStateBoolean trampoline encountered an unknown error");
        // NOTE: Most calls only allow XR_SUCCESS as a return code
        return XR_SUCCESS;
    }
}


XRAPI_ATTR XrResult XRAPI_CALL xrGetActionStateVector1f(
    XrAction                                    action,
    uint32_t                                    countSubactionPaths,
    const XrPath*                               subactionPaths,
    XrActionStateVector1f*                      data) {
    try {
        std::unique_lock<std::mutex> secondary_lock(g_action_mutex);
        LoaderInstance *loader_instance = g_action_map[action];
        secondary_lock.unlock();
        if (nullptr == loader_instance) {
            XrLoaderLogObjectInfo bad_object = {};
            bad_object.type = XR_OBJECT_TYPE_ACTION;
            bad_object.handle = reinterpret_cast<uint64_t const&>(action);
            std::vector<XrLoaderLogObjectInfo> loader_objects;
            loader_objects.push_back(bad_object);
            LoaderLogger::LogValidationErrorMessage("VUID-xrGetActionStateVector1f-action-parameter", "xrGetActionStateVector1f",
                                                    "action is not a valid XrAction", loader_objects);
            return XR_ERROR_HANDLE_INVALID;
        }
        const std::unique_ptr<XrGeneratedDispatchTable>& dispatch_table = loader_instance->DispatchTable();
        return dispatch_table->GetActionStateVector1f(action, countSubactionPaths, subactionPaths, data);
    } catch (...) {
        LoaderLogger::LogErrorMessage("xrGetActionStateVector1f", "xrGetActionStateVector1f trampoline encountered an unknown error");
        // NOTE: Most calls only allow XR_SUCCESS as a return code
        return XR_SUCCESS;
    }
}


XRAPI_ATTR XrResult XRAPI_CALL xrGetActionStateVector2f(
    XrAction                                    action,
    uint32_t                                    countSubactionPaths,
    const XrPath*                               subactionPaths,
    XrActionStateVector2f*                      data) {
    try {
        std::unique_lock<std::mutex> secondary_lock(g_action_mutex);
        LoaderInstance *loader_instance = g_action_map[action];
        secondary_lock.unlock();
        if (nullptr == loader_instance) {
            XrLoaderLogObjectInfo bad_object = {};
            bad_object.type = XR_OBJECT_TYPE_ACTION;
            bad_object.handle = reinterpret_cast<uint64_t const&>(action);
            std::vector<XrLoaderLogObjectInfo> loader_objects;
            loader_objects.push_back(bad_object);
            LoaderLogger::LogValidationErrorMessage("VUID-xrGetActionStateVector2f-action-parameter", "xrGetActionStateVector2f",
                                                    "action is not a valid XrAction", loader_objects);
            return XR_ERROR_HANDLE_INVALID;
        }
        const std::unique_ptr<XrGeneratedDispatchTable>& dispatch_table = loader_instance->DispatchTable();
        return dispatch_table->GetActionStateVector2f(action, countSubactionPaths, subactionPaths, data);
    } catch (...) {
        LoaderLogger::LogErrorMessage("xrGetActionStateVector2f", "xrGetActionStateVector2f trampoline encountered an unknown error");
        // NOTE: Most calls only allow XR_SUCCESS as a return code
        return XR_SUCCESS;
    }
}


XRAPI_ATTR XrResult XRAPI_CALL xrGetActionStatePose(
    XrAction                                    action,
    XrPath                                      subactionPath,
    XrActionStatePose*                          data) {
    try {
        std::unique_lock<std::mutex> secondary_lock(g_action_mutex);
        LoaderInstance *loader_instance = g_action_map[action];
        secondary_lock.unlock();
        if (nullptr == loader_instance) {
            XrLoaderLogObjectInfo bad_object = {};
            bad_object.type = XR_OBJECT_TYPE_ACTION;
            bad_object.handle = reinterpret_cast<uint64_t const&>(action);
            std::vector<XrLoaderLogObjectInfo> loader_objects;
            loader_objects.push_back(bad_object);
            LoaderLogger::LogValidationErrorMessage("VUID-xrGetActionStatePose-action-parameter", "xrGetActionStatePose",
                                                    "action is not a valid XrAction", loader_objects);
            return XR_ERROR_HANDLE_INVALID;
        }
        const std::unique_ptr<XrGeneratedDispatchTable>& dispatch_table = loader_instance->DispatchTable();
        return dispatch_table->GetActionStatePose(action, subactionPath, data);
    } catch (...) {
        LoaderLogger::LogErrorMessage("xrGetActionStatePose", "xrGetActionStatePose trampoline encountered an unknown error");
        // NOTE: Most calls only allow XR_SUCCESS as a return code
        return XR_SUCCESS;
    }
}


XRAPI_ATTR XrResult XRAPI_CALL xrSyncActionData(
    XrSession                                   session,
    uint32_t                                    countActionSets,
    const XrActiveActionSet*                    actionSets) {
    try {
        std::unique_lock<std::mutex> secondary_lock(g_session_mutex);
        LoaderInstance *loader_instance = g_session_map[session];
        secondary_lock.unlock();
        if (nullptr == loader_instance) {
            XrLoaderLogObjectInfo bad_object = {};
            bad_object.type = XR_OBJECT_TYPE_SESSION;
            bad_object.handle = reinterpret_cast<uint64_t const&>(session);
            std::vector<XrLoaderLogObjectInfo> loader_objects;
            loader_objects.push_back(bad_object);
            LoaderLogger::LogValidationErrorMessage("VUID-xrSyncActionData-session-parameter", "xrSyncActionData",
                                                    "session is not a valid XrSession", loader_objects);
            return XR_ERROR_HANDLE_INVALID;
        }
        const std::unique_ptr<XrGeneratedDispatchTable>& dispatch_table = loader_instance->DispatchTable();
        return dispatch_table->SyncActionData(session, countActionSets, actionSets);
    } catch (...) {
        LoaderLogger::LogErrorMessage("xrSyncActionData", "xrSyncActionData trampoline encountered an unknown error");
        // NOTE: Most calls only allow XR_SUCCESS as a return code
        return XR_SUCCESS;
    }
}


XRAPI_ATTR XrResult XRAPI_CALL xrGetBoundSourcesForAction(
    XrAction                                    action,
    uint32_t                                    sourceCapacityInput,
    uint32_t*                                   sourceCountOutput,
    XrPath*                                     sources) {
    try {
        std::unique_lock<std::mutex> secondary_lock(g_action_mutex);
        LoaderInstance *loader_instance = g_action_map[action];
        secondary_lock.unlock();
        if (nullptr == loader_instance) {
            XrLoaderLogObjectInfo bad_object = {};
            bad_object.type = XR_OBJECT_TYPE_ACTION;
            bad_object.handle = reinterpret_cast<uint64_t const&>(action);
            std::vector<XrLoaderLogObjectInfo> loader_objects;
            loader_objects.push_back(bad_object);
            LoaderLogger::LogValidationErrorMessage("VUID-xrGetBoundSourcesForAction-action-parameter", "xrGetBoundSourcesForAction",
                                                    "action is not a valid XrAction", loader_objects);
            return XR_ERROR_HANDLE_INVALID;
        }
        const std::unique_ptr<XrGeneratedDispatchTable>& dispatch_table = loader_instance->DispatchTable();
        return dispatch_table->GetBoundSourcesForAction(action, sourceCapacityInput, sourceCountOutput, sources);
    } catch (...) {
        LoaderLogger::LogErrorMessage("xrGetBoundSourcesForAction", "xrGetBoundSourcesForAction trampoline encountered an unknown error");
        // NOTE: Most calls only allow XR_SUCCESS as a return code
        return XR_SUCCESS;
    }
}


XRAPI_ATTR XrResult XRAPI_CALL xrGetInputSourceLocalizedName(
    XrSession                                   session,
    XrPath                                      source,
    XrInputSourceLocalizedNameFlags             whichComponents,
    uint32_t                                    bufferCapacityInput,
    uint32_t*                                   bufferCountOutput,
    char*                                       buffer) {
    try {
        std::unique_lock<std::mutex> secondary_lock(g_session_mutex);
        LoaderInstance *loader_instance = g_session_map[session];
        secondary_lock.unlock();
        if (nullptr == loader_instance) {
            XrLoaderLogObjectInfo bad_object = {};
            bad_object.type = XR_OBJECT_TYPE_SESSION;
            bad_object.handle = reinterpret_cast<uint64_t const&>(session);
            std::vector<XrLoaderLogObjectInfo> loader_objects;
            loader_objects.push_back(bad_object);
            LoaderLogger::LogValidationErrorMessage("VUID-xrGetInputSourceLocalizedName-session-parameter", "xrGetInputSourceLocalizedName",
                                                    "session is not a valid XrSession", loader_objects);
            return XR_ERROR_HANDLE_INVALID;
        }
        const std::unique_ptr<XrGeneratedDispatchTable>& dispatch_table = loader_instance->DispatchTable();
        return dispatch_table->GetInputSourceLocalizedName(session, source, whichComponents, bufferCapacityInput, bufferCountOutput, buffer);
    } catch (...) {
        LoaderLogger::LogErrorMessage("xrGetInputSourceLocalizedName", "xrGetInputSourceLocalizedName trampoline encountered an unknown error");
        // NOTE: Most calls only allow XR_SUCCESS as a return code
        return XR_SUCCESS;
    }
}


XRAPI_ATTR XrResult XRAPI_CALL xrApplyHapticFeedback(
    XrAction                                    hapticAction,
    uint32_t                                    countSubactionPaths,
    const XrPath*                               subactionPaths,
    const XrHapticBaseHeader*                   hapticEvent) {
    try {
        std::unique_lock<std::mutex> secondary_lock(g_action_mutex);
        LoaderInstance *loader_instance = g_action_map[hapticAction];
        secondary_lock.unlock();
        if (nullptr == loader_instance) {
            XrLoaderLogObjectInfo bad_object = {};
            bad_object.type = XR_OBJECT_TYPE_ACTION;
            bad_object.handle = reinterpret_cast<uint64_t const&>(hapticAction);
            std::vector<XrLoaderLogObjectInfo> loader_objects;
            loader_objects.push_back(bad_object);
            LoaderLogger::LogValidationErrorMessage("VUID-xrApplyHapticFeedback-hapticAction-parameter", "xrApplyHapticFeedback",
                                                    "hapticAction is not a valid XrAction", loader_objects);
            return XR_ERROR_HANDLE_INVALID;
        }
        const std::unique_ptr<XrGeneratedDispatchTable>& dispatch_table = loader_instance->DispatchTable();
        return dispatch_table->ApplyHapticFeedback(hapticAction, countSubactionPaths, subactionPaths, hapticEvent);
    } catch (...) {
        LoaderLogger::LogErrorMessage("xrApplyHapticFeedback", "xrApplyHapticFeedback trampoline encountered an unknown error");
        // NOTE: Most calls only allow XR_SUCCESS as a return code
        return XR_SUCCESS;
    }
}


XRAPI_ATTR XrResult XRAPI_CALL xrStopHapticFeedback(
    XrAction                                    hapticAction,
    uint32_t                                    countSubactionPaths,
    const XrPath*                               subactionPaths) {
    try {
        std::unique_lock<std::mutex> secondary_lock(g_action_mutex);
        LoaderInstance *loader_instance = g_action_map[hapticAction];
        secondary_lock.unlock();
        if (nullptr == loader_instance) {
            XrLoaderLogObjectInfo bad_object = {};
            bad_object.type = XR_OBJECT_TYPE_ACTION;
            bad_object.handle = reinterpret_cast<uint64_t const&>(hapticAction);
            std::vector<XrLoaderLogObjectInfo> loader_objects;
            loader_objects.push_back(bad_object);
            LoaderLogger::LogValidationErrorMessage("VUID-xrStopHapticFeedback-hapticAction-parameter", "xrStopHapticFeedback",
                                                    "hapticAction is not a valid XrAction", loader_objects);
            return XR_ERROR_HANDLE_INVALID;
        }
        const std::unique_ptr<XrGeneratedDispatchTable>& dispatch_table = loader_instance->DispatchTable();
        return dispatch_table->StopHapticFeedback(hapticAction, countSubactionPaths, subactionPaths);
    } catch (...) {
        LoaderLogger::LogErrorMessage("xrStopHapticFeedback", "xrStopHapticFeedback trampoline encountered an unknown error");
        // NOTE: Most calls only allow XR_SUCCESS as a return code
        return XR_SUCCESS;
    }
}



// ---- XR_KHR_android_thread_settings extension commands
#if defined(XR_USE_PLATFORM_ANDROID)
XRAPI_ATTR XrResult XRAPI_CALL xrSetAndroidApplicationThreadKHR(
    XrSession                                   session,
    XrAndroidThreadTypeKHR                      threadType,
    uint32_t                                    threadId) {
    try {
        std::unique_lock<std::mutex> secondary_lock(g_session_mutex);
        LoaderInstance *loader_instance = g_session_map[session];
        secondary_lock.unlock();
        if (nullptr == loader_instance) {
            XrLoaderLogObjectInfo bad_object = {};
            bad_object.type = XR_OBJECT_TYPE_SESSION;
            bad_object.handle = reinterpret_cast<uint64_t const&>(session);
            std::vector<XrLoaderLogObjectInfo> loader_objects;
            loader_objects.push_back(bad_object);
            LoaderLogger::LogValidationErrorMessage("VUID-xrSetAndroidApplicationThreadKHR-session-parameter", "xrSetAndroidApplicationThreadKHR",
                                                    "session is not a valid XrSession", loader_objects);
            return XR_ERROR_HANDLE_INVALID;
        }
        const std::unique_ptr<XrGeneratedDispatchTable>& dispatch_table = loader_instance->DispatchTable();
        if (!loader_instance->ExtensionIsEnabled("XR_KHR_android_thread_settings")) {
            LoaderLogger::LogValidationErrorMessage("VUID-xrSetAndroidApplicationThreadKHR-extension-notenabled",
                                                    "xrSetAndroidApplicationThreadKHR",
                                                    "The XR_KHR_android_thread_settings extension has not been enabled prior to calling xrSetAndroidApplicationThreadKHR");
            return XR_ERROR_FUNCTION_UNSUPPORTED;
        }

        return dispatch_table->SetAndroidApplicationThreadKHR(session, threadType, threadId);
    } catch (...) {
        LoaderLogger::LogErrorMessage("xrSetAndroidApplicationThreadKHR", "xrSetAndroidApplicationThreadKHR trampoline encountered an unknown error");
        // NOTE: Most calls only allow XR_SUCCESS as a return code
        return XR_SUCCESS;
    }
}

#endif // defined(XR_USE_PLATFORM_ANDROID)


// ---- XR_KHR_android_surface_swapchain extension commands
#if defined(XR_USE_PLATFORM_ANDROID)
XRAPI_ATTR XrResult XRAPI_CALL xrCreateSwapchainAndroidSurfaceKHR(
    XrSession                                   session,
    const XrSwapchainCreateInfo*                info,
    XrSwapchain*                                swapchain,
    jobject*                                    surface) {
    try {
        std::unique_lock<std::mutex> secondary_lock(g_session_mutex);
        LoaderInstance *loader_instance = g_session_map[session];
        secondary_lock.unlock();
        if (nullptr == loader_instance) {
            XrLoaderLogObjectInfo bad_object = {};
            bad_object.type = XR_OBJECT_TYPE_SESSION;
            bad_object.handle = reinterpret_cast<uint64_t const&>(session);
            std::vector<XrLoaderLogObjectInfo> loader_objects;
            loader_objects.push_back(bad_object);
            LoaderLogger::LogValidationErrorMessage("VUID-xrCreateSwapchainAndroidSurfaceKHR-session-parameter", "xrCreateSwapchainAndroidSurfaceKHR",
                                                    "session is not a valid XrSession", loader_objects);
            return XR_ERROR_HANDLE_INVALID;
        }
        const std::unique_ptr<XrGeneratedDispatchTable>& dispatch_table = loader_instance->DispatchTable();
        XrResult result = XR_SUCCESS;
        if (!loader_instance->ExtensionIsEnabled("XR_KHR_android_surface_swapchain")) {
            LoaderLogger::LogValidationErrorMessage("VUID-xrCreateSwapchainAndroidSurfaceKHR-extension-notenabled",
                                                    "xrCreateSwapchainAndroidSurfaceKHR",
                                                    "The XR_KHR_android_surface_swapchain extension has not been enabled prior to calling xrCreateSwapchainAndroidSurfaceKHR");
            return XR_ERROR_FUNCTION_UNSUPPORTED;
        }

        result = dispatch_table->CreateSwapchainAndroidSurfaceKHR(session, info, swapchain, surface);
        return result;
    } catch (std::bad_alloc &) {
        LoaderLogger::LogErrorMessage("xrCreateSwapchainAndroidSurfaceKHR", "xrCreateSwapchainAndroidSurfaceKHR trampoline failed allocating memory");
        return XR_ERROR_OUT_OF_MEMORY;
    } catch (...) {
        LoaderLogger::LogErrorMessage("xrCreateSwapchainAndroidSurfaceKHR", "xrCreateSwapchainAndroidSurfaceKHR trampoline encountered an unknown error");
        return XR_ERROR_INITIALIZATION_FAILED;
    }
}

#endif // defined(XR_USE_PLATFORM_ANDROID)


// ---- XR_KHR_opengl_enable extension commands
#if defined(XR_USE_GRAPHICS_API_OPENGL)
XRAPI_ATTR XrResult XRAPI_CALL xrGetOpenGLGraphicsRequirementsKHR(
    XrInstance                                  instance,
    XrSystemId                                  systemId,
    XrGraphicsRequirementsOpenGLKHR*            graphicsRequirements) {
    try {
        std::unique_lock<std::mutex> secondary_lock(g_instance_mutex);
        LoaderInstance *loader_instance = g_instance_map[instance];
        secondary_lock.unlock();
        if (nullptr == loader_instance) {
            XrLoaderLogObjectInfo bad_object = {};
            bad_object.type = XR_OBJECT_TYPE_INSTANCE;
            bad_object.handle = reinterpret_cast<uint64_t const&>(instance);
            std::vector<XrLoaderLogObjectInfo> loader_objects;
            loader_objects.push_back(bad_object);
            LoaderLogger::LogValidationErrorMessage("VUID-xrGetOpenGLGraphicsRequirementsKHR-instance-parameter", "xrGetOpenGLGraphicsRequirementsKHR",
                                                    "instance is not a valid XrInstance", loader_objects);
            return XR_ERROR_HANDLE_INVALID;
        }
        const std::unique_ptr<XrGeneratedDispatchTable>& dispatch_table = loader_instance->DispatchTable();
        if (!loader_instance->ExtensionIsEnabled("XR_KHR_opengl_enable")) {
            LoaderLogger::LogValidationErrorMessage("VUID-xrGetOpenGLGraphicsRequirementsKHR-extension-notenabled",
                                                    "xrGetOpenGLGraphicsRequirementsKHR",
                                                    "The XR_KHR_opengl_enable extension has not been enabled prior to calling xrGetOpenGLGraphicsRequirementsKHR");
            return XR_ERROR_FUNCTION_UNSUPPORTED;
        }

        return dispatch_table->GetOpenGLGraphicsRequirementsKHR(instance, systemId, graphicsRequirements);
    } catch (...) {
        std::string error_message = "xrGetOpenGLGraphicsRequirementsKHR trampoline encountered an unknown error.  Likely XrInstance 0x";
        std::ostringstream oss;
        oss << std::hex << reinterpret_cast<const void*>(instance);
        error_message += oss.str();
        error_message += " is invalid";
        LoaderLogger::LogErrorMessage("xrGetOpenGLGraphicsRequirementsKHR", error_message);
        return XR_ERROR_HANDLE_INVALID;
    }
}

#endif // defined(XR_USE_GRAPHICS_API_OPENGL)


// ---- XR_KHR_opengl_es_enable extension commands
#if defined(XR_USE_GRAPHICS_API_OPENGL_ES)
XRAPI_ATTR XrResult XRAPI_CALL xrGetOpenGLESGraphicsRequirementsKHR(
    XrInstance                                  instance,
    XrSystemId                                  systemId,
    XrGraphicsRequirementsOpenGLESKHR*          graphicsRequirements) {
    try {
        std::unique_lock<std::mutex> secondary_lock(g_instance_mutex);
        LoaderInstance *loader_instance = g_instance_map[instance];
        secondary_lock.unlock();
        if (nullptr == loader_instance) {
            XrLoaderLogObjectInfo bad_object = {};
            bad_object.type = XR_OBJECT_TYPE_INSTANCE;
            bad_object.handle = reinterpret_cast<uint64_t const&>(instance);
            std::vector<XrLoaderLogObjectInfo> loader_objects;
            loader_objects.push_back(bad_object);
            LoaderLogger::LogValidationErrorMessage("VUID-xrGetOpenGLESGraphicsRequirementsKHR-instance-parameter", "xrGetOpenGLESGraphicsRequirementsKHR",
                                                    "instance is not a valid XrInstance", loader_objects);
            return XR_ERROR_HANDLE_INVALID;
        }
        const std::unique_ptr<XrGeneratedDispatchTable>& dispatch_table = loader_instance->DispatchTable();
        if (!loader_instance->ExtensionIsEnabled("XR_KHR_opengl_es_enable")) {
            LoaderLogger::LogValidationErrorMessage("VUID-xrGetOpenGLESGraphicsRequirementsKHR-extension-notenabled",
                                                    "xrGetOpenGLESGraphicsRequirementsKHR",
                                                    "The XR_KHR_opengl_es_enable extension has not been enabled prior to calling xrGetOpenGLESGraphicsRequirementsKHR");
            return XR_ERROR_FUNCTION_UNSUPPORTED;
        }

        return dispatch_table->GetOpenGLESGraphicsRequirementsKHR(instance, systemId, graphicsRequirements);
    } catch (...) {
        std::string error_message = "xrGetOpenGLESGraphicsRequirementsKHR trampoline encountered an unknown error.  Likely XrInstance 0x";
        std::ostringstream oss;
        oss << std::hex << reinterpret_cast<const void*>(instance);
        error_message += oss.str();
        error_message += " is invalid";
        LoaderLogger::LogErrorMessage("xrGetOpenGLESGraphicsRequirementsKHR", error_message);
        return XR_ERROR_HANDLE_INVALID;
    }
}

#endif // defined(XR_USE_GRAPHICS_API_OPENGL_ES)


// ---- XR_KHR_vulkan_enable extension commands
#if defined(XR_USE_GRAPHICS_API_VULKAN)
XRAPI_ATTR XrResult XRAPI_CALL xrGetVulkanInstanceExtensionsKHR(
    XrInstance                                  instance,
    XrSystemId                                  systemId,
    uint32_t                                    namesCapacityInput,
    uint32_t*                                   namesCountOutput,
    char*                                       namesString) {
    try {
        std::unique_lock<std::mutex> secondary_lock(g_instance_mutex);
        LoaderInstance *loader_instance = g_instance_map[instance];
        secondary_lock.unlock();
        if (nullptr == loader_instance) {
            XrLoaderLogObjectInfo bad_object = {};
            bad_object.type = XR_OBJECT_TYPE_INSTANCE;
            bad_object.handle = reinterpret_cast<uint64_t const&>(instance);
            std::vector<XrLoaderLogObjectInfo> loader_objects;
            loader_objects.push_back(bad_object);
            LoaderLogger::LogValidationErrorMessage("VUID-xrGetVulkanInstanceExtensionsKHR-instance-parameter", "xrGetVulkanInstanceExtensionsKHR",
                                                    "instance is not a valid XrInstance", loader_objects);
            return XR_ERROR_HANDLE_INVALID;
        }
        const std::unique_ptr<XrGeneratedDispatchTable>& dispatch_table = loader_instance->DispatchTable();
        if (!loader_instance->ExtensionIsEnabled("XR_KHR_vulkan_enable")) {
            LoaderLogger::LogValidationErrorMessage("VUID-xrGetVulkanInstanceExtensionsKHR-extension-notenabled",
                                                    "xrGetVulkanInstanceExtensionsKHR",
                                                    "The XR_KHR_vulkan_enable extension has not been enabled prior to calling xrGetVulkanInstanceExtensionsKHR");
            return XR_ERROR_FUNCTION_UNSUPPORTED;
        }

        return dispatch_table->GetVulkanInstanceExtensionsKHR(instance, systemId, namesCapacityInput, namesCountOutput, namesString);
    } catch (...) {
        std::string error_message = "xrGetVulkanInstanceExtensionsKHR trampoline encountered an unknown error.  Likely XrInstance 0x";
        std::ostringstream oss;
        oss << std::hex << reinterpret_cast<const void*>(instance);
        error_message += oss.str();
        error_message += " is invalid";
        LoaderLogger::LogErrorMessage("xrGetVulkanInstanceExtensionsKHR", error_message);
        return XR_ERROR_HANDLE_INVALID;
    }
}

#endif // defined(XR_USE_GRAPHICS_API_VULKAN)

#if defined(XR_USE_GRAPHICS_API_VULKAN)
XRAPI_ATTR XrResult XRAPI_CALL xrGetVulkanDeviceExtensionsKHR(
    XrInstance                                  instance,
    XrSystemId                                  systemId,
    uint32_t                                    namesCapacityInput,
    uint32_t*                                   namesCountOutput,
    char*                                       namesString) {
    try {
        std::unique_lock<std::mutex> secondary_lock(g_instance_mutex);
        LoaderInstance *loader_instance = g_instance_map[instance];
        secondary_lock.unlock();
        if (nullptr == loader_instance) {
            XrLoaderLogObjectInfo bad_object = {};
            bad_object.type = XR_OBJECT_TYPE_INSTANCE;
            bad_object.handle = reinterpret_cast<uint64_t const&>(instance);
            std::vector<XrLoaderLogObjectInfo> loader_objects;
            loader_objects.push_back(bad_object);
            LoaderLogger::LogValidationErrorMessage("VUID-xrGetVulkanDeviceExtensionsKHR-instance-parameter", "xrGetVulkanDeviceExtensionsKHR",
                                                    "instance is not a valid XrInstance", loader_objects);
            return XR_ERROR_HANDLE_INVALID;
        }
        const std::unique_ptr<XrGeneratedDispatchTable>& dispatch_table = loader_instance->DispatchTable();
        if (!loader_instance->ExtensionIsEnabled("XR_KHR_vulkan_enable")) {
            LoaderLogger::LogValidationErrorMessage("VUID-xrGetVulkanDeviceExtensionsKHR-extension-notenabled",
                                                    "xrGetVulkanDeviceExtensionsKHR",
                                                    "The XR_KHR_vulkan_enable extension has not been enabled prior to calling xrGetVulkanDeviceExtensionsKHR");
            return XR_ERROR_FUNCTION_UNSUPPORTED;
        }

        return dispatch_table->GetVulkanDeviceExtensionsKHR(instance, systemId, namesCapacityInput, namesCountOutput, namesString);
    } catch (...) {
        std::string error_message = "xrGetVulkanDeviceExtensionsKHR trampoline encountered an unknown error.  Likely XrInstance 0x";
        std::ostringstream oss;
        oss << std::hex << reinterpret_cast<const void*>(instance);
        error_message += oss.str();
        error_message += " is invalid";
        LoaderLogger::LogErrorMessage("xrGetVulkanDeviceExtensionsKHR", error_message);
        return XR_ERROR_HANDLE_INVALID;
    }
}

#endif // defined(XR_USE_GRAPHICS_API_VULKAN)

#if defined(XR_USE_GRAPHICS_API_VULKAN)
XRAPI_ATTR XrResult XRAPI_CALL xrGetVulkanGraphicsDeviceKHR(
    XrInstance                                  instance,
    XrSystemId                                  systemId,
    VkInstance                                  vkInstance,
    VkPhysicalDevice*                           vkPhysicalDevice) {
    try {
        std::unique_lock<std::mutex> secondary_lock(g_instance_mutex);
        LoaderInstance *loader_instance = g_instance_map[instance];
        secondary_lock.unlock();
        if (nullptr == loader_instance) {
            XrLoaderLogObjectInfo bad_object = {};
            bad_object.type = XR_OBJECT_TYPE_INSTANCE;
            bad_object.handle = reinterpret_cast<uint64_t const&>(instance);
            std::vector<XrLoaderLogObjectInfo> loader_objects;
            loader_objects.push_back(bad_object);
            LoaderLogger::LogValidationErrorMessage("VUID-xrGetVulkanGraphicsDeviceKHR-instance-parameter", "xrGetVulkanGraphicsDeviceKHR",
                                                    "instance is not a valid XrInstance", loader_objects);
            return XR_ERROR_HANDLE_INVALID;
        }
        const std::unique_ptr<XrGeneratedDispatchTable>& dispatch_table = loader_instance->DispatchTable();
        if (!loader_instance->ExtensionIsEnabled("XR_KHR_vulkan_enable")) {
            LoaderLogger::LogValidationErrorMessage("VUID-xrGetVulkanGraphicsDeviceKHR-extension-notenabled",
                                                    "xrGetVulkanGraphicsDeviceKHR",
                                                    "The XR_KHR_vulkan_enable extension has not been enabled prior to calling xrGetVulkanGraphicsDeviceKHR");
            return XR_ERROR_FUNCTION_UNSUPPORTED;
        }

        return dispatch_table->GetVulkanGraphicsDeviceKHR(instance, systemId, vkInstance, vkPhysicalDevice);
    } catch (...) {
        std::string error_message = "xrGetVulkanGraphicsDeviceKHR trampoline encountered an unknown error.  Likely XrInstance 0x";
        std::ostringstream oss;
        oss << std::hex << reinterpret_cast<const void*>(instance);
        error_message += oss.str();
        error_message += " is invalid";
        LoaderLogger::LogErrorMessage("xrGetVulkanGraphicsDeviceKHR", error_message);
        return XR_ERROR_HANDLE_INVALID;
    }
}

#endif // defined(XR_USE_GRAPHICS_API_VULKAN)

#if defined(XR_USE_GRAPHICS_API_VULKAN)
XRAPI_ATTR XrResult XRAPI_CALL xrGetVulkanGraphicsRequirementsKHR(
    XrInstance                                  instance,
    XrSystemId                                  systemId,
    XrGraphicsRequirementsVulkanKHR*            graphicsRequirements) {
    try {
        std::unique_lock<std::mutex> secondary_lock(g_instance_mutex);
        LoaderInstance *loader_instance = g_instance_map[instance];
        secondary_lock.unlock();
        if (nullptr == loader_instance) {
            XrLoaderLogObjectInfo bad_object = {};
            bad_object.type = XR_OBJECT_TYPE_INSTANCE;
            bad_object.handle = reinterpret_cast<uint64_t const&>(instance);
            std::vector<XrLoaderLogObjectInfo> loader_objects;
            loader_objects.push_back(bad_object);
            LoaderLogger::LogValidationErrorMessage("VUID-xrGetVulkanGraphicsRequirementsKHR-instance-parameter", "xrGetVulkanGraphicsRequirementsKHR",
                                                    "instance is not a valid XrInstance", loader_objects);
            return XR_ERROR_HANDLE_INVALID;
        }
        const std::unique_ptr<XrGeneratedDispatchTable>& dispatch_table = loader_instance->DispatchTable();
        if (!loader_instance->ExtensionIsEnabled("XR_KHR_vulkan_enable")) {
            LoaderLogger::LogValidationErrorMessage("VUID-xrGetVulkanGraphicsRequirementsKHR-extension-notenabled",
                                                    "xrGetVulkanGraphicsRequirementsKHR",
                                                    "The XR_KHR_vulkan_enable extension has not been enabled prior to calling xrGetVulkanGraphicsRequirementsKHR");
            return XR_ERROR_FUNCTION_UNSUPPORTED;
        }

        return dispatch_table->GetVulkanGraphicsRequirementsKHR(instance, systemId, graphicsRequirements);
    } catch (...) {
        std::string error_message = "xrGetVulkanGraphicsRequirementsKHR trampoline encountered an unknown error.  Likely XrInstance 0x";
        std::ostringstream oss;
        oss << std::hex << reinterpret_cast<const void*>(instance);
        error_message += oss.str();
        error_message += " is invalid";
        LoaderLogger::LogErrorMessage("xrGetVulkanGraphicsRequirementsKHR", error_message);
        return XR_ERROR_HANDLE_INVALID;
    }
}

#endif // defined(XR_USE_GRAPHICS_API_VULKAN)


// ---- XR_KHR_D3D10_enable extension commands
#if defined(XR_USE_GRAPHICS_API_D3D10)
XRAPI_ATTR XrResult XRAPI_CALL xrGetD3D10GraphicsRequirementsKHR(
    XrInstance                                  instance,
    XrSystemId                                  systemId,
    XrGraphicsRequirementsD3D10KHR*             graphicsRequirements) {
    try {
        std::unique_lock<std::mutex> secondary_lock(g_instance_mutex);
        LoaderInstance *loader_instance = g_instance_map[instance];
        secondary_lock.unlock();
        if (nullptr == loader_instance) {
            XrLoaderLogObjectInfo bad_object = {};
            bad_object.type = XR_OBJECT_TYPE_INSTANCE;
            bad_object.handle = reinterpret_cast<uint64_t const&>(instance);
            std::vector<XrLoaderLogObjectInfo> loader_objects;
            loader_objects.push_back(bad_object);
            LoaderLogger::LogValidationErrorMessage("VUID-xrGetD3D10GraphicsRequirementsKHR-instance-parameter", "xrGetD3D10GraphicsRequirementsKHR",
                                                    "instance is not a valid XrInstance", loader_objects);
            return XR_ERROR_HANDLE_INVALID;
        }
        const std::unique_ptr<XrGeneratedDispatchTable>& dispatch_table = loader_instance->DispatchTable();
        if (!loader_instance->ExtensionIsEnabled("XR_KHR_D3D10_enable")) {
            LoaderLogger::LogValidationErrorMessage("VUID-xrGetD3D10GraphicsRequirementsKHR-extension-notenabled",
                                                    "xrGetD3D10GraphicsRequirementsKHR",
                                                    "The XR_KHR_D3D10_enable extension has not been enabled prior to calling xrGetD3D10GraphicsRequirementsKHR");
            return XR_ERROR_FUNCTION_UNSUPPORTED;
        }

        return dispatch_table->GetD3D10GraphicsRequirementsKHR(instance, systemId, graphicsRequirements);
    } catch (...) {
        std::string error_message = "xrGetD3D10GraphicsRequirementsKHR trampoline encountered an unknown error.  Likely XrInstance 0x";
        std::ostringstream oss;
        oss << std::hex << reinterpret_cast<const void*>(instance);
        error_message += oss.str();
        error_message += " is invalid";
        LoaderLogger::LogErrorMessage("xrGetD3D10GraphicsRequirementsKHR", error_message);
        return XR_ERROR_HANDLE_INVALID;
    }
}

#endif // defined(XR_USE_GRAPHICS_API_D3D10)


// ---- XR_KHR_D3D11_enable extension commands
#if defined(XR_USE_GRAPHICS_API_D3D11)
XRAPI_ATTR XrResult XRAPI_CALL xrGetD3D11GraphicsRequirementsKHR(
    XrInstance                                  instance,
    XrSystemId                                  systemId,
    XrGraphicsRequirementsD3D11KHR*             graphicsRequirements) {
    try {
        std::unique_lock<std::mutex> secondary_lock(g_instance_mutex);
        LoaderInstance *loader_instance = g_instance_map[instance];
        secondary_lock.unlock();
        if (nullptr == loader_instance) {
            XrLoaderLogObjectInfo bad_object = {};
            bad_object.type = XR_OBJECT_TYPE_INSTANCE;
            bad_object.handle = reinterpret_cast<uint64_t const&>(instance);
            std::vector<XrLoaderLogObjectInfo> loader_objects;
            loader_objects.push_back(bad_object);
            LoaderLogger::LogValidationErrorMessage("VUID-xrGetD3D11GraphicsRequirementsKHR-instance-parameter", "xrGetD3D11GraphicsRequirementsKHR",
                                                    "instance is not a valid XrInstance", loader_objects);
            return XR_ERROR_HANDLE_INVALID;
        }
        const std::unique_ptr<XrGeneratedDispatchTable>& dispatch_table = loader_instance->DispatchTable();
        if (!loader_instance->ExtensionIsEnabled("XR_KHR_D3D11_enable")) {
            LoaderLogger::LogValidationErrorMessage("VUID-xrGetD3D11GraphicsRequirementsKHR-extension-notenabled",
                                                    "xrGetD3D11GraphicsRequirementsKHR",
                                                    "The XR_KHR_D3D11_enable extension has not been enabled prior to calling xrGetD3D11GraphicsRequirementsKHR");
            return XR_ERROR_FUNCTION_UNSUPPORTED;
        }

        return dispatch_table->GetD3D11GraphicsRequirementsKHR(instance, systemId, graphicsRequirements);
    } catch (...) {
        std::string error_message = "xrGetD3D11GraphicsRequirementsKHR trampoline encountered an unknown error.  Likely XrInstance 0x";
        std::ostringstream oss;
        oss << std::hex << reinterpret_cast<const void*>(instance);
        error_message += oss.str();
        error_message += " is invalid";
        LoaderLogger::LogErrorMessage("xrGetD3D11GraphicsRequirementsKHR", error_message);
        return XR_ERROR_HANDLE_INVALID;
    }
}

#endif // defined(XR_USE_GRAPHICS_API_D3D11)


// ---- XR_KHR_D3D12_enable extension commands
#if defined(XR_USE_GRAPHICS_API_D3D12)
XRAPI_ATTR XrResult XRAPI_CALL xrGetD3D12GraphicsRequirementsKHR(
    XrInstance                                  instance,
    XrSystemId                                  systemId,
    XrGraphicsRequirementsD3D12KHR*             graphicsRequirements) {
    try {
        std::unique_lock<std::mutex> secondary_lock(g_instance_mutex);
        LoaderInstance *loader_instance = g_instance_map[instance];
        secondary_lock.unlock();
        if (nullptr == loader_instance) {
            XrLoaderLogObjectInfo bad_object = {};
            bad_object.type = XR_OBJECT_TYPE_INSTANCE;
            bad_object.handle = reinterpret_cast<uint64_t const&>(instance);
            std::vector<XrLoaderLogObjectInfo> loader_objects;
            loader_objects.push_back(bad_object);
            LoaderLogger::LogValidationErrorMessage("VUID-xrGetD3D12GraphicsRequirementsKHR-instance-parameter", "xrGetD3D12GraphicsRequirementsKHR",
                                                    "instance is not a valid XrInstance", loader_objects);
            return XR_ERROR_HANDLE_INVALID;
        }
        const std::unique_ptr<XrGeneratedDispatchTable>& dispatch_table = loader_instance->DispatchTable();
        if (!loader_instance->ExtensionIsEnabled("XR_KHR_D3D12_enable")) {
            LoaderLogger::LogValidationErrorMessage("VUID-xrGetD3D12GraphicsRequirementsKHR-extension-notenabled",
                                                    "xrGetD3D12GraphicsRequirementsKHR",
                                                    "The XR_KHR_D3D12_enable extension has not been enabled prior to calling xrGetD3D12GraphicsRequirementsKHR");
            return XR_ERROR_FUNCTION_UNSUPPORTED;
        }

        return dispatch_table->GetD3D12GraphicsRequirementsKHR(instance, systemId, graphicsRequirements);
    } catch (...) {
        std::string error_message = "xrGetD3D12GraphicsRequirementsKHR trampoline encountered an unknown error.  Likely XrInstance 0x";
        std::ostringstream oss;
        oss << std::hex << reinterpret_cast<const void*>(instance);
        error_message += oss.str();
        error_message += " is invalid";
        LoaderLogger::LogErrorMessage("xrGetD3D12GraphicsRequirementsKHR", error_message);
        return XR_ERROR_HANDLE_INVALID;
    }
}

#endif // defined(XR_USE_GRAPHICS_API_D3D12)


// ---- XR_KHR_visibility_mask extension commands
XRAPI_ATTR XrResult XRAPI_CALL xrGetVisibilityMaskKHR(
    XrSession                                   session,
    XrViewConfigurationType                     viewConfigurationType,
    uint32_t                                    viewIndex,
    XrVisibilityMaskTypeKHR                     visibilityMaskType,
    XrVisibilityMaskKHR*                        visibilityMask) {
    try {
        std::unique_lock<std::mutex> secondary_lock(g_session_mutex);
        LoaderInstance *loader_instance = g_session_map[session];
        secondary_lock.unlock();
        if (nullptr == loader_instance) {
            XrLoaderLogObjectInfo bad_object = {};
            bad_object.type = XR_OBJECT_TYPE_SESSION;
            bad_object.handle = reinterpret_cast<uint64_t const&>(session);
            std::vector<XrLoaderLogObjectInfo> loader_objects;
            loader_objects.push_back(bad_object);
            LoaderLogger::LogValidationErrorMessage("VUID-xrGetVisibilityMaskKHR-session-parameter", "xrGetVisibilityMaskKHR",
                                                    "session is not a valid XrSession", loader_objects);
            return XR_ERROR_HANDLE_INVALID;
        }
        const std::unique_ptr<XrGeneratedDispatchTable>& dispatch_table = loader_instance->DispatchTable();
        if (!loader_instance->ExtensionIsEnabled("XR_KHR_visibility_mask")) {
            LoaderLogger::LogValidationErrorMessage("VUID-xrGetVisibilityMaskKHR-extension-notenabled",
                                                    "xrGetVisibilityMaskKHR",
                                                    "The XR_KHR_visibility_mask extension has not been enabled prior to calling xrGetVisibilityMaskKHR");
            return XR_ERROR_FUNCTION_UNSUPPORTED;
        }

        return dispatch_table->GetVisibilityMaskKHR(session, viewConfigurationType, viewIndex, visibilityMaskType, visibilityMask);
    } catch (...) {
        LoaderLogger::LogErrorMessage("xrGetVisibilityMaskKHR", "xrGetVisibilityMaskKHR trampoline encountered an unknown error");
        // NOTE: Most calls only allow XR_SUCCESS as a return code
        return XR_SUCCESS;
    }
}



// ---- XR_KHR_win32_convert_performance_counter_time extension commands
#if defined(XR_USE_PLATFORM_WIN32)
XRAPI_ATTR XrResult XRAPI_CALL xrConvertWin32PerformanceCounterToTimeKHR(
    XrInstance                                  instance,
    const LARGE_INTEGER*                        performanceCounter,
    XrTime*                                     time) {
    try {
        std::unique_lock<std::mutex> secondary_lock(g_instance_mutex);
        LoaderInstance *loader_instance = g_instance_map[instance];
        secondary_lock.unlock();
        if (nullptr == loader_instance) {
            XrLoaderLogObjectInfo bad_object = {};
            bad_object.type = XR_OBJECT_TYPE_INSTANCE;
            bad_object.handle = reinterpret_cast<uint64_t const&>(instance);
            std::vector<XrLoaderLogObjectInfo> loader_objects;
            loader_objects.push_back(bad_object);
            LoaderLogger::LogValidationErrorMessage("VUID-xrConvertWin32PerformanceCounterToTimeKHR-instance-parameter", "xrConvertWin32PerformanceCounterToTimeKHR",
                                                    "instance is not a valid XrInstance", loader_objects);
            return XR_ERROR_HANDLE_INVALID;
        }
        const std::unique_ptr<XrGeneratedDispatchTable>& dispatch_table = loader_instance->DispatchTable();
        if (!loader_instance->ExtensionIsEnabled("XR_KHR_win32_convert_performance_counter_time")) {
            LoaderLogger::LogValidationErrorMessage("VUID-xrConvertWin32PerformanceCounterToTimeKHR-extension-notenabled",
                                                    "xrConvertWin32PerformanceCounterToTimeKHR",
                                                    "The XR_KHR_win32_convert_performance_counter_time extension has not been enabled prior to calling xrConvertWin32PerformanceCounterToTimeKHR");
            return XR_ERROR_FUNCTION_UNSUPPORTED;
        }

        return dispatch_table->ConvertWin32PerformanceCounterToTimeKHR(instance, performanceCounter, time);
    } catch (...) {
        std::string error_message = "xrConvertWin32PerformanceCounterToTimeKHR trampoline encountered an unknown error.  Likely XrInstance 0x";
        std::ostringstream oss;
        oss << std::hex << reinterpret_cast<const void*>(instance);
        error_message += oss.str();
        error_message += " is invalid";
        LoaderLogger::LogErrorMessage("xrConvertWin32PerformanceCounterToTimeKHR", error_message);
        return XR_ERROR_HANDLE_INVALID;
    }
}

#endif // defined(XR_USE_PLATFORM_WIN32)

#if defined(XR_USE_PLATFORM_WIN32)
XRAPI_ATTR XrResult XRAPI_CALL xrConvertTimeToWin32PerformanceCounterKHR(
    XrInstance                                  instance,
    XrTime                                      time,
    LARGE_INTEGER*                              performanceCounter) {
    try {
        std::unique_lock<std::mutex> secondary_lock(g_instance_mutex);
        LoaderInstance *loader_instance = g_instance_map[instance];
        secondary_lock.unlock();
        if (nullptr == loader_instance) {
            XrLoaderLogObjectInfo bad_object = {};
            bad_object.type = XR_OBJECT_TYPE_INSTANCE;
            bad_object.handle = reinterpret_cast<uint64_t const&>(instance);
            std::vector<XrLoaderLogObjectInfo> loader_objects;
            loader_objects.push_back(bad_object);
            LoaderLogger::LogValidationErrorMessage("VUID-xrConvertTimeToWin32PerformanceCounterKHR-instance-parameter", "xrConvertTimeToWin32PerformanceCounterKHR",
                                                    "instance is not a valid XrInstance", loader_objects);
            return XR_ERROR_HANDLE_INVALID;
        }
        const std::unique_ptr<XrGeneratedDispatchTable>& dispatch_table = loader_instance->DispatchTable();
        if (!loader_instance->ExtensionIsEnabled("XR_KHR_win32_convert_performance_counter_time")) {
            LoaderLogger::LogValidationErrorMessage("VUID-xrConvertTimeToWin32PerformanceCounterKHR-extension-notenabled",
                                                    "xrConvertTimeToWin32PerformanceCounterKHR",
                                                    "The XR_KHR_win32_convert_performance_counter_time extension has not been enabled prior to calling xrConvertTimeToWin32PerformanceCounterKHR");
            return XR_ERROR_FUNCTION_UNSUPPORTED;
        }

        return dispatch_table->ConvertTimeToWin32PerformanceCounterKHR(instance, time, performanceCounter);
    } catch (...) {
        std::string error_message = "xrConvertTimeToWin32PerformanceCounterKHR trampoline encountered an unknown error.  Likely XrInstance 0x";
        std::ostringstream oss;
        oss << std::hex << reinterpret_cast<const void*>(instance);
        error_message += oss.str();
        error_message += " is invalid";
        LoaderLogger::LogErrorMessage("xrConvertTimeToWin32PerformanceCounterKHR", error_message);
        return XR_ERROR_HANDLE_INVALID;
    }
}

#endif // defined(XR_USE_PLATFORM_WIN32)


// ---- XR_KHR_convert_timespec_time extension commands
#if defined(XR_USE_TIMESPEC)
XRAPI_ATTR XrResult XRAPI_CALL xrConvertTimespecTimeToTimeKHR(
    XrInstance                                  instance,
    const struct timespec*                      timespecTime,
    XrTime*                                     time) {
    try {
        std::unique_lock<std::mutex> secondary_lock(g_instance_mutex);
        LoaderInstance *loader_instance = g_instance_map[instance];
        secondary_lock.unlock();
        if (nullptr == loader_instance) {
            XrLoaderLogObjectInfo bad_object = {};
            bad_object.type = XR_OBJECT_TYPE_INSTANCE;
            bad_object.handle = reinterpret_cast<uint64_t const&>(instance);
            std::vector<XrLoaderLogObjectInfo> loader_objects;
            loader_objects.push_back(bad_object);
            LoaderLogger::LogValidationErrorMessage("VUID-xrConvertTimespecTimeToTimeKHR-instance-parameter", "xrConvertTimespecTimeToTimeKHR",
                                                    "instance is not a valid XrInstance", loader_objects);
            return XR_ERROR_HANDLE_INVALID;
        }
        const std::unique_ptr<XrGeneratedDispatchTable>& dispatch_table = loader_instance->DispatchTable();
        if (!loader_instance->ExtensionIsEnabled("XR_KHR_convert_timespec_time")) {
            LoaderLogger::LogValidationErrorMessage("VUID-xrConvertTimespecTimeToTimeKHR-extension-notenabled",
                                                    "xrConvertTimespecTimeToTimeKHR",
                                                    "The XR_KHR_convert_timespec_time extension has not been enabled prior to calling xrConvertTimespecTimeToTimeKHR");
            return XR_ERROR_FUNCTION_UNSUPPORTED;
        }

        return dispatch_table->ConvertTimespecTimeToTimeKHR(instance, timespecTime, time);
    } catch (...) {
        std::string error_message = "xrConvertTimespecTimeToTimeKHR trampoline encountered an unknown error.  Likely XrInstance 0x";
        std::ostringstream oss;
        oss << std::hex << reinterpret_cast<const void*>(instance);
        error_message += oss.str();
        error_message += " is invalid";
        LoaderLogger::LogErrorMessage("xrConvertTimespecTimeToTimeKHR", error_message);
        return XR_ERROR_HANDLE_INVALID;
    }
}

#endif // defined(XR_USE_TIMESPEC)

#if defined(XR_USE_TIMESPEC)
XRAPI_ATTR XrResult XRAPI_CALL xrConvertTimeToTimespecTimeKHR(
    XrInstance                                  instance,
    XrTime                                      time,
    struct timespec*                            timespecTime) {
    try {
        std::unique_lock<std::mutex> secondary_lock(g_instance_mutex);
        LoaderInstance *loader_instance = g_instance_map[instance];
        secondary_lock.unlock();
        if (nullptr == loader_instance) {
            XrLoaderLogObjectInfo bad_object = {};
            bad_object.type = XR_OBJECT_TYPE_INSTANCE;
            bad_object.handle = reinterpret_cast<uint64_t const&>(instance);
            std::vector<XrLoaderLogObjectInfo> loader_objects;
            loader_objects.push_back(bad_object);
            LoaderLogger::LogValidationErrorMessage("VUID-xrConvertTimeToTimespecTimeKHR-instance-parameter", "xrConvertTimeToTimespecTimeKHR",
                                                    "instance is not a valid XrInstance", loader_objects);
            return XR_ERROR_HANDLE_INVALID;
        }
        const std::unique_ptr<XrGeneratedDispatchTable>& dispatch_table = loader_instance->DispatchTable();
        if (!loader_instance->ExtensionIsEnabled("XR_KHR_convert_timespec_time")) {
            LoaderLogger::LogValidationErrorMessage("VUID-xrConvertTimeToTimespecTimeKHR-extension-notenabled",
                                                    "xrConvertTimeToTimespecTimeKHR",
                                                    "The XR_KHR_convert_timespec_time extension has not been enabled prior to calling xrConvertTimeToTimespecTimeKHR");
            return XR_ERROR_FUNCTION_UNSUPPORTED;
        }

        return dispatch_table->ConvertTimeToTimespecTimeKHR(instance, time, timespecTime);
    } catch (...) {
        std::string error_message = "xrConvertTimeToTimespecTimeKHR trampoline encountered an unknown error.  Likely XrInstance 0x";
        std::ostringstream oss;
        oss << std::hex << reinterpret_cast<const void*>(instance);
        error_message += oss.str();
        error_message += " is invalid";
        LoaderLogger::LogErrorMessage("xrConvertTimeToTimespecTimeKHR", error_message);
        return XR_ERROR_HANDLE_INVALID;
    }
}

#endif // defined(XR_USE_TIMESPEC)


// ---- XR_EXT_debug_utils extension commands
XRAPI_ATTR XrResult XRAPI_CALL xrSetDebugUtilsObjectNameEXT(
    XrInstance                                  instance,
    const XrDebugUtilsObjectNameInfoEXT*        nameInfo) {
    try {
        std::unique_lock<std::mutex> secondary_lock(g_instance_mutex);
        LoaderInstance *loader_instance = g_instance_map[instance];
        secondary_lock.unlock();
        if (nullptr == loader_instance) {
            XrLoaderLogObjectInfo bad_object = {};
            bad_object.type = XR_OBJECT_TYPE_INSTANCE;
            bad_object.handle = reinterpret_cast<uint64_t const&>(instance);
            std::vector<XrLoaderLogObjectInfo> loader_objects;
            loader_objects.push_back(bad_object);
            LoaderLogger::LogValidationErrorMessage("VUID-xrSetDebugUtilsObjectNameEXT-instance-parameter", "xrSetDebugUtilsObjectNameEXT",
                                                    "instance is not a valid XrInstance", loader_objects);
            return XR_ERROR_HANDLE_INVALID;
        }
        const std::unique_ptr<XrGeneratedDispatchTable>& dispatch_table = loader_instance->DispatchTable();
        if (!loader_instance->ExtensionIsEnabled("XR_EXT_debug_utils")) {
            LoaderLogger::LogValidationErrorMessage("VUID-xrSetDebugUtilsObjectNameEXT-extension-notenabled",
                                                    "xrSetDebugUtilsObjectNameEXT",
                                                    "The XR_EXT_debug_utils extension has not been enabled prior to calling xrSetDebugUtilsObjectNameEXT");
            return XR_ERROR_FUNCTION_UNSUPPORTED;
        }

        return dispatch_table->SetDebugUtilsObjectNameEXT(instance, nameInfo);
    } catch (...) {
        std::string error_message = "xrSetDebugUtilsObjectNameEXT trampoline encountered an unknown error.  Likely XrInstance 0x";
        std::ostringstream oss;
        oss << std::hex << reinterpret_cast<const void*>(instance);
        error_message += oss.str();
        error_message += " is invalid";
        LoaderLogger::LogErrorMessage("xrSetDebugUtilsObjectNameEXT", error_message);
        return XR_ERROR_HANDLE_INVALID;
    }
}


XRAPI_ATTR XrResult XRAPI_CALL xrSubmitDebugUtilsMessageEXT(
    XrInstance                                  instance,
    XrDebugUtilsMessageSeverityFlagsEXT         messageSeverity,
    XrDebugUtilsMessageTypeFlagsEXT             messageTypes,
    const XrDebugUtilsMessengerCallbackDataEXT* callbackData) {
    try {
        std::unique_lock<std::mutex> secondary_lock(g_instance_mutex);
        LoaderInstance *loader_instance = g_instance_map[instance];
        secondary_lock.unlock();
        if (nullptr == loader_instance) {
            XrLoaderLogObjectInfo bad_object = {};
            bad_object.type = XR_OBJECT_TYPE_INSTANCE;
            bad_object.handle = reinterpret_cast<uint64_t const&>(instance);
            std::vector<XrLoaderLogObjectInfo> loader_objects;
            loader_objects.push_back(bad_object);
            LoaderLogger::LogValidationErrorMessage("VUID-xrSubmitDebugUtilsMessageEXT-instance-parameter", "xrSubmitDebugUtilsMessageEXT",
                                                    "instance is not a valid XrInstance", loader_objects);
            return XR_ERROR_HANDLE_INVALID;
        }
        const std::unique_ptr<XrGeneratedDispatchTable>& dispatch_table = loader_instance->DispatchTable();
        if (!loader_instance->ExtensionIsEnabled("XR_EXT_debug_utils")) {
            LoaderLogger::LogValidationErrorMessage("VUID-xrSubmitDebugUtilsMessageEXT-extension-notenabled",
                                                    "xrSubmitDebugUtilsMessageEXT",
                                                    "The XR_EXT_debug_utils extension has not been enabled prior to calling xrSubmitDebugUtilsMessageEXT");
            return XR_ERROR_FUNCTION_UNSUPPORTED;
        }

        return dispatch_table->SubmitDebugUtilsMessageEXT(instance, messageSeverity, messageTypes, callbackData);
    } catch (...) {
        std::string error_message = "xrSubmitDebugUtilsMessageEXT trampoline encountered an unknown error.  Likely XrInstance 0x";
        std::ostringstream oss;
        oss << std::hex << reinterpret_cast<const void*>(instance);
        error_message += oss.str();
        error_message += " is invalid";
        LoaderLogger::LogErrorMessage("xrSubmitDebugUtilsMessageEXT", error_message);
        return XR_ERROR_HANDLE_INVALID;
    }
}



// ---- XR_MSFT_secondary_view_configuration extension commands
XRAPI_ATTR XrResult XRAPI_CALL xrSubmitFrameCompositionLayersMSFT(
    XrSession                                   session,
    XrViewConfigurationType                     viewConfigurationType,
    const XrFrameEndInfo*                       frameEndInfo) {
    try {
        std::unique_lock<std::mutex> secondary_lock(g_session_mutex);
        LoaderInstance *loader_instance = g_session_map[session];
        secondary_lock.unlock();
        if (nullptr == loader_instance) {
            XrLoaderLogObjectInfo bad_object = {};
            bad_object.type = XR_OBJECT_TYPE_SESSION;
            bad_object.handle = reinterpret_cast<uint64_t const&>(session);
            std::vector<XrLoaderLogObjectInfo> loader_objects;
            loader_objects.push_back(bad_object);
            LoaderLogger::LogValidationErrorMessage("VUID-xrSubmitFrameCompositionLayersMSFT-session-parameter", "xrSubmitFrameCompositionLayersMSFT",
                                                    "session is not a valid XrSession", loader_objects);
            return XR_ERROR_HANDLE_INVALID;
        }
        const std::unique_ptr<XrGeneratedDispatchTable>& dispatch_table = loader_instance->DispatchTable();
        if (!loader_instance->ExtensionIsEnabled("XR_MSFT_secondary_view_configuration")) {
            LoaderLogger::LogValidationErrorMessage("VUID-xrSubmitFrameCompositionLayersMSFT-extension-notenabled",
                                                    "xrSubmitFrameCompositionLayersMSFT",
                                                    "The XR_MSFT_secondary_view_configuration extension has not been enabled prior to calling xrSubmitFrameCompositionLayersMSFT");
            return XR_ERROR_FUNCTION_UNSUPPORTED;
        }

        return dispatch_table->SubmitFrameCompositionLayersMSFT(session, viewConfigurationType, frameEndInfo);
    } catch (...) {
        LoaderLogger::LogErrorMessage("xrSubmitFrameCompositionLayersMSFT", "xrSubmitFrameCompositionLayersMSFT trampoline encountered an unknown error");
        // NOTE: Most calls only allow XR_SUCCESS as a return code
        return XR_SUCCESS;
    }
}



// ---- XR_MSFT_spatial_perception_bridge extension commands
#if defined(XR_USE_PLATFORM_WIN32)
XRAPI_ATTR XrResult XRAPI_CALL xrCreateSpaceFromSpatialCoordinateSystemMSFT(
    XrSession                                   session,
    const XrSpatialCoordinateSystemSpaceCreateInfoMSFT* spaceCreateInfo,
    XrSpace*                                    space) {
    try {
        std::unique_lock<std::mutex> secondary_lock(g_session_mutex);
        LoaderInstance *loader_instance = g_session_map[session];
        secondary_lock.unlock();
        if (nullptr == loader_instance) {
            XrLoaderLogObjectInfo bad_object = {};
            bad_object.type = XR_OBJECT_TYPE_SESSION;
            bad_object.handle = reinterpret_cast<uint64_t const&>(session);
            std::vector<XrLoaderLogObjectInfo> loader_objects;
            loader_objects.push_back(bad_object);
            LoaderLogger::LogValidationErrorMessage("VUID-xrCreateSpaceFromSpatialCoordinateSystemMSFT-session-parameter", "xrCreateSpaceFromSpatialCoordinateSystemMSFT",
                                                    "session is not a valid XrSession", loader_objects);
            return XR_ERROR_HANDLE_INVALID;
        }
        const std::unique_ptr<XrGeneratedDispatchTable>& dispatch_table = loader_instance->DispatchTable();
        XrResult result = XR_SUCCESS;
        if (!loader_instance->ExtensionIsEnabled("XR_MSFT_spatial_perception_bridge")) {
            LoaderLogger::LogValidationErrorMessage("VUID-xrCreateSpaceFromSpatialCoordinateSystemMSFT-extension-notenabled",
                                                    "xrCreateSpaceFromSpatialCoordinateSystemMSFT",
                                                    "The XR_MSFT_spatial_perception_bridge extension has not been enabled prior to calling xrCreateSpaceFromSpatialCoordinateSystemMSFT");
            return XR_ERROR_FUNCTION_UNSUPPORTED;
        }

        result = dispatch_table->CreateSpaceFromSpatialCoordinateSystemMSFT(session, spaceCreateInfo, space);
        if (XR_SUCCESS == result && nullptr != space) {
            std::unique_lock<std::mutex> space_lock(g_space_mutex);
            auto exists = g_space_map.find(*space);
            if (exists == g_space_map.end()) {
                g_space_map[*space] = loader_instance;
            }
        }
        return result;
    } catch (std::bad_alloc &) {
        LoaderLogger::LogErrorMessage("xrCreateSpaceFromSpatialCoordinateSystemMSFT", "xrCreateSpaceFromSpatialCoordinateSystemMSFT trampoline failed allocating memory");
        return XR_ERROR_OUT_OF_MEMORY;
    } catch (...) {
        LoaderLogger::LogErrorMessage("xrCreateSpaceFromSpatialCoordinateSystemMSFT", "xrCreateSpaceFromSpatialCoordinateSystemMSFT trampoline encountered an unknown error");
        return XR_ERROR_INITIALIZATION_FAILED;
    }
}

#endif // defined(XR_USE_PLATFORM_WIN32)


// ---- XR_MSFT_controller_render_model extension commands
XRAPI_ATTR XrResult XRAPI_CALL xrLoadControllerRenderModelMSFT(
    XrSession                                   session,
    uint64_t                                    modelKey,
    uint32_t                                    sizeInput,
    uint32_t*                                   sizeOutput,
    uint8_t*                                    buffer) {
    try {
        std::unique_lock<std::mutex> secondary_lock(g_session_mutex);
        LoaderInstance *loader_instance = g_session_map[session];
        secondary_lock.unlock();
        if (nullptr == loader_instance) {
            XrLoaderLogObjectInfo bad_object = {};
            bad_object.type = XR_OBJECT_TYPE_SESSION;
            bad_object.handle = reinterpret_cast<uint64_t const&>(session);
            std::vector<XrLoaderLogObjectInfo> loader_objects;
            loader_objects.push_back(bad_object);
            LoaderLogger::LogValidationErrorMessage("VUID-xrLoadControllerRenderModelMSFT-session-parameter", "xrLoadControllerRenderModelMSFT",
                                                    "session is not a valid XrSession", loader_objects);
            return XR_ERROR_HANDLE_INVALID;
        }
        const std::unique_ptr<XrGeneratedDispatchTable>& dispatch_table = loader_instance->DispatchTable();
        if (!loader_instance->ExtensionIsEnabled("XR_MSFT_controller_render_model")) {
            LoaderLogger::LogValidationErrorMessage("VUID-xrLoadControllerRenderModelMSFT-extension-notenabled",
                                                    "xrLoadControllerRenderModelMSFT",
                                                    "The XR_MSFT_controller_render_model extension has not been enabled prior to calling xrLoadControllerRenderModelMSFT");
            return XR_ERROR_FUNCTION_UNSUPPORTED;
        }

        return dispatch_table->LoadControllerRenderModelMSFT(session, modelKey, sizeInput, sizeOutput, buffer);
    } catch (...) {
        LoaderLogger::LogErrorMessage("xrLoadControllerRenderModelMSFT", "xrLoadControllerRenderModelMSFT trampoline encountered an unknown error");
        // NOTE: Most calls only allow XR_SUCCESS as a return code
        return XR_SUCCESS;
    }
}



// ---- XR_MSFT_spatial_anchor extension commands
XRAPI_ATTR XrResult XRAPI_CALL xrCreateSpatialAnchorSpaceMSFT(
    XrSession                                   session,
    XrSpatialAnchorMSFT                         anchor,
    XrSpace*                                    space) {
    try {
        std::unique_lock<std::mutex> secondary_lock(g_session_mutex);
        LoaderInstance *loader_instance = g_session_map[session];
        secondary_lock.unlock();
        if (nullptr == loader_instance) {
            XrLoaderLogObjectInfo bad_object = {};
            bad_object.type = XR_OBJECT_TYPE_SESSION;
            bad_object.handle = reinterpret_cast<uint64_t const&>(session);
            std::vector<XrLoaderLogObjectInfo> loader_objects;
            loader_objects.push_back(bad_object);
            LoaderLogger::LogValidationErrorMessage("VUID-xrCreateSpatialAnchorSpaceMSFT-session-parameter", "xrCreateSpatialAnchorSpaceMSFT",
                                                    "session is not a valid XrSession", loader_objects);
            return XR_ERROR_HANDLE_INVALID;
        }
        const std::unique_ptr<XrGeneratedDispatchTable>& dispatch_table = loader_instance->DispatchTable();
        XrResult result = XR_SUCCESS;
        if (!loader_instance->ExtensionIsEnabled("XR_MSFT_spatial_anchor")) {
            LoaderLogger::LogValidationErrorMessage("VUID-xrCreateSpatialAnchorSpaceMSFT-extension-notenabled",
                                                    "xrCreateSpatialAnchorSpaceMSFT",
                                                    "The XR_MSFT_spatial_anchor extension has not been enabled prior to calling xrCreateSpatialAnchorSpaceMSFT");
            return XR_ERROR_FUNCTION_UNSUPPORTED;
        }

        result = dispatch_table->CreateSpatialAnchorSpaceMSFT(session, anchor, space);
        if (XR_SUCCESS == result && nullptr != space) {
            std::unique_lock<std::mutex> space_lock(g_space_mutex);
            auto exists = g_space_map.find(*space);
            if (exists == g_space_map.end()) {
                g_space_map[*space] = loader_instance;
            }
        }
        return result;
    } catch (std::bad_alloc &) {
        LoaderLogger::LogErrorMessage("xrCreateSpatialAnchorSpaceMSFT", "xrCreateSpatialAnchorSpaceMSFT trampoline failed allocating memory");
        return XR_ERROR_OUT_OF_MEMORY;
    } catch (...) {
        LoaderLogger::LogErrorMessage("xrCreateSpatialAnchorSpaceMSFT", "xrCreateSpatialAnchorSpaceMSFT trampoline encountered an unknown error");
        return XR_ERROR_INITIALIZATION_FAILED;
    }
}


XRAPI_ATTR XrResult XRAPI_CALL xrCreateSpatialAnchorMSFT(
    XrSession                                   session,
    const XrSpatialAnchorCreateInfoMSFT*        anchorCreateInfo,
    XrSpatialAnchorMSFT*                        anchor) {
    try {
        std::unique_lock<std::mutex> secondary_lock(g_session_mutex);
        LoaderInstance *loader_instance = g_session_map[session];
        secondary_lock.unlock();
        if (nullptr == loader_instance) {
            XrLoaderLogObjectInfo bad_object = {};
            bad_object.type = XR_OBJECT_TYPE_SESSION;
            bad_object.handle = reinterpret_cast<uint64_t const&>(session);
            std::vector<XrLoaderLogObjectInfo> loader_objects;
            loader_objects.push_back(bad_object);
            LoaderLogger::LogValidationErrorMessage("VUID-xrCreateSpatialAnchorMSFT-session-parameter", "xrCreateSpatialAnchorMSFT",
                                                    "session is not a valid XrSession", loader_objects);
            return XR_ERROR_HANDLE_INVALID;
        }
        const std::unique_ptr<XrGeneratedDispatchTable>& dispatch_table = loader_instance->DispatchTable();
        XrResult result = XR_SUCCESS;
        if (!loader_instance->ExtensionIsEnabled("XR_MSFT_spatial_anchor")) {
            LoaderLogger::LogValidationErrorMessage("VUID-xrCreateSpatialAnchorMSFT-extension-notenabled",
                                                    "xrCreateSpatialAnchorMSFT",
                                                    "The XR_MSFT_spatial_anchor extension has not been enabled prior to calling xrCreateSpatialAnchorMSFT");
            return XR_ERROR_FUNCTION_UNSUPPORTED;
        }

        result = dispatch_table->CreateSpatialAnchorMSFT(session, anchorCreateInfo, anchor);
        if (XR_SUCCESS == result && nullptr != anchor) {
            std::unique_lock<std::mutex> spatialanchormsft_lock(g_spatialanchormsft_mutex);
            auto exists = g_spatialanchormsft_map.find(*anchor);
            if (exists == g_spatialanchormsft_map.end()) {
                g_spatialanchormsft_map[*anchor] = loader_instance;
            }
        }
        return result;
    } catch (std::bad_alloc &) {
        LoaderLogger::LogErrorMessage("xrCreateSpatialAnchorMSFT", "xrCreateSpatialAnchorMSFT trampoline failed allocating memory");
        return XR_ERROR_OUT_OF_MEMORY;
    } catch (...) {
        LoaderLogger::LogErrorMessage("xrCreateSpatialAnchorMSFT", "xrCreateSpatialAnchorMSFT trampoline encountered an unknown error");
        return XR_ERROR_INITIALIZATION_FAILED;
    }
}


XRAPI_ATTR XrResult XRAPI_CALL xrDestroySpatialAnchorMSFT(
    XrSpatialAnchorMSFT                         anchor) {
    try {
        std::unique_lock<std::mutex> secondary_lock(g_spatialanchormsft_mutex);
        LoaderInstance *loader_instance = g_spatialanchormsft_map[anchor];
        // Destroy the mapping entry for this item if it was valid.
        if (nullptr != loader_instance) {
                g_spatialanchormsft_map.erase(anchor);
        }
        secondary_lock.unlock();
        if (nullptr == loader_instance) {
            XrLoaderLogObjectInfo bad_object = {};
            bad_object.type = XR_OBJECT_TYPE_SPATIAL_ANCHOR_MSFT;
            bad_object.handle = reinterpret_cast<uint64_t const&>(anchor);
            std::vector<XrLoaderLogObjectInfo> loader_objects;
            loader_objects.push_back(bad_object);
            LoaderLogger::LogValidationErrorMessage("VUID-xrDestroySpatialAnchorMSFT-anchor-parameter", "xrDestroySpatialAnchorMSFT",
                                                    "anchor is not a valid XrSpatialAnchorMSFT", loader_objects);
            return XR_ERROR_HANDLE_INVALID;
        }
        const std::unique_ptr<XrGeneratedDispatchTable>& dispatch_table = loader_instance->DispatchTable();
        XrResult result = XR_SUCCESS;
        if (!loader_instance->ExtensionIsEnabled("XR_MSFT_spatial_anchor")) {
            LoaderLogger::LogValidationErrorMessage("VUID-xrDestroySpatialAnchorMSFT-extension-notenabled",
                                                    "xrDestroySpatialAnchorMSFT",
                                                    "The XR_MSFT_spatial_anchor extension has not been enabled prior to calling xrDestroySpatialAnchorMSFT");
            return XR_ERROR_FUNCTION_UNSUPPORTED;
        }

        result = dispatch_table->DestroySpatialAnchorMSFT(anchor);
        return result;
    } catch (...) {
        LoaderLogger::LogErrorMessage("xrDestroySpatialAnchorMSFT", "xrDestroySpatialAnchorMSFT trampoline encountered an unknown error");
        // NOTE: Most calls only allow XR_SUCCESS as a return code
        return XR_SUCCESS;
    }
}



// ---- XR_MSFT_spatial_anchor_storage extension commands
XRAPI_ATTR XrResult XRAPI_CALL xrStoreSpatialAnchorMSFT(
    XrSpatialAnchorMSFT                         anchor,
    XrPath                                      anchorName) {
    try {
        std::unique_lock<std::mutex> secondary_lock(g_spatialanchormsft_mutex);
        LoaderInstance *loader_instance = g_spatialanchormsft_map[anchor];
        secondary_lock.unlock();
        if (nullptr == loader_instance) {
            XrLoaderLogObjectInfo bad_object = {};
            bad_object.type = XR_OBJECT_TYPE_SPATIAL_ANCHOR_MSFT;
            bad_object.handle = reinterpret_cast<uint64_t const&>(anchor);
            std::vector<XrLoaderLogObjectInfo> loader_objects;
            loader_objects.push_back(bad_object);
            LoaderLogger::LogValidationErrorMessage("VUID-xrStoreSpatialAnchorMSFT-anchor-parameter", "xrStoreSpatialAnchorMSFT",
                                                    "anchor is not a valid XrSpatialAnchorMSFT", loader_objects);
            return XR_ERROR_HANDLE_INVALID;
        }
        const std::unique_ptr<XrGeneratedDispatchTable>& dispatch_table = loader_instance->DispatchTable();
        if (!loader_instance->ExtensionIsEnabled("XR_MSFT_spatial_anchor_storage")) {
            LoaderLogger::LogValidationErrorMessage("VUID-xrStoreSpatialAnchorMSFT-extension-notenabled",
                                                    "xrStoreSpatialAnchorMSFT",
                                                    "The XR_MSFT_spatial_anchor_storage extension has not been enabled prior to calling xrStoreSpatialAnchorMSFT");
            return XR_ERROR_FUNCTION_UNSUPPORTED;
        }

        return dispatch_table->StoreSpatialAnchorMSFT(anchor, anchorName);
    } catch (...) {
        LoaderLogger::LogErrorMessage("xrStoreSpatialAnchorMSFT", "xrStoreSpatialAnchorMSFT trampoline encountered an unknown error");
        // NOTE: Most calls only allow XR_SUCCESS as a return code
        return XR_SUCCESS;
    }
}


XRAPI_ATTR XrResult XRAPI_CALL xrEnumerateStoredAnchorsMSFT(
    XrSession                                   session,
    uint32_t                                    anchorCapacityInput,
    uint32_t*                                   anchorCountOutput,
    XrPath*                                     anchorNames) {
    try {
        std::unique_lock<std::mutex> secondary_lock(g_session_mutex);
        LoaderInstance *loader_instance = g_session_map[session];
        secondary_lock.unlock();
        if (nullptr == loader_instance) {
            XrLoaderLogObjectInfo bad_object = {};
            bad_object.type = XR_OBJECT_TYPE_SESSION;
            bad_object.handle = reinterpret_cast<uint64_t const&>(session);
            std::vector<XrLoaderLogObjectInfo> loader_objects;
            loader_objects.push_back(bad_object);
            LoaderLogger::LogValidationErrorMessage("VUID-xrEnumerateStoredAnchorsMSFT-session-parameter", "xrEnumerateStoredAnchorsMSFT",
                                                    "session is not a valid XrSession", loader_objects);
            return XR_ERROR_HANDLE_INVALID;
        }
        const std::unique_ptr<XrGeneratedDispatchTable>& dispatch_table = loader_instance->DispatchTable();
        if (!loader_instance->ExtensionIsEnabled("XR_MSFT_spatial_anchor_storage")) {
            LoaderLogger::LogValidationErrorMessage("VUID-xrEnumerateStoredAnchorsMSFT-extension-notenabled",
                                                    "xrEnumerateStoredAnchorsMSFT",
                                                    "The XR_MSFT_spatial_anchor_storage extension has not been enabled prior to calling xrEnumerateStoredAnchorsMSFT");
            return XR_ERROR_FUNCTION_UNSUPPORTED;
        }

        return dispatch_table->EnumerateStoredAnchorsMSFT(session, anchorCapacityInput, anchorCountOutput, anchorNames);
    } catch (...) {
        LoaderLogger::LogErrorMessage("xrEnumerateStoredAnchorsMSFT", "xrEnumerateStoredAnchorsMSFT trampoline encountered an unknown error");
        // NOTE: Most calls only allow XR_SUCCESS as a return code
        return XR_SUCCESS;
    }
}


XRAPI_ATTR XrResult XRAPI_CALL xrCreateSpatialAnchorFromStoredAnchorNameMSFT(
    XrSession                                   session,
    XrPath                                      anchorName,
    XrSpatialAnchorMSFT*                        anchor) {
    try {
        std::unique_lock<std::mutex> secondary_lock(g_session_mutex);
        LoaderInstance *loader_instance = g_session_map[session];
        secondary_lock.unlock();
        if (nullptr == loader_instance) {
            XrLoaderLogObjectInfo bad_object = {};
            bad_object.type = XR_OBJECT_TYPE_SESSION;
            bad_object.handle = reinterpret_cast<uint64_t const&>(session);
            std::vector<XrLoaderLogObjectInfo> loader_objects;
            loader_objects.push_back(bad_object);
            LoaderLogger::LogValidationErrorMessage("VUID-xrCreateSpatialAnchorFromStoredAnchorNameMSFT-session-parameter", "xrCreateSpatialAnchorFromStoredAnchorNameMSFT",
                                                    "session is not a valid XrSession", loader_objects);
            return XR_ERROR_HANDLE_INVALID;
        }
        const std::unique_ptr<XrGeneratedDispatchTable>& dispatch_table = loader_instance->DispatchTable();
        XrResult result = XR_SUCCESS;
        if (!loader_instance->ExtensionIsEnabled("XR_MSFT_spatial_anchor_storage")) {
            LoaderLogger::LogValidationErrorMessage("VUID-xrCreateSpatialAnchorFromStoredAnchorNameMSFT-extension-notenabled",
                                                    "xrCreateSpatialAnchorFromStoredAnchorNameMSFT",
                                                    "The XR_MSFT_spatial_anchor_storage extension has not been enabled prior to calling xrCreateSpatialAnchorFromStoredAnchorNameMSFT");
            return XR_ERROR_FUNCTION_UNSUPPORTED;
        }

        result = dispatch_table->CreateSpatialAnchorFromStoredAnchorNameMSFT(session, anchorName, anchor);
        if (XR_SUCCESS == result && nullptr != anchor) {
            std::unique_lock<std::mutex> spatialanchormsft_lock(g_spatialanchormsft_mutex);
            auto exists = g_spatialanchormsft_map.find(*anchor);
            if (exists == g_spatialanchormsft_map.end()) {
                g_spatialanchormsft_map[*anchor] = loader_instance;
            }
        }
        return result;
    } catch (std::bad_alloc &) {
        LoaderLogger::LogErrorMessage("xrCreateSpatialAnchorFromStoredAnchorNameMSFT", "xrCreateSpatialAnchorFromStoredAnchorNameMSFT trampoline failed allocating memory");
        return XR_ERROR_OUT_OF_MEMORY;
    } catch (...) {
        LoaderLogger::LogErrorMessage("xrCreateSpatialAnchorFromStoredAnchorNameMSFT", "xrCreateSpatialAnchorFromStoredAnchorNameMSFT trampoline encountered an unknown error");
        return XR_ERROR_INITIALIZATION_FAILED;
    }
}


XRAPI_ATTR XrResult XRAPI_CALL xrDeleteStoredAnchorMSFT(
    XrSession                                   session,
    XrPath                                      anchorName) {
    try {
        std::unique_lock<std::mutex> secondary_lock(g_session_mutex);
        LoaderInstance *loader_instance = g_session_map[session];
        secondary_lock.unlock();
        if (nullptr == loader_instance) {
            XrLoaderLogObjectInfo bad_object = {};
            bad_object.type = XR_OBJECT_TYPE_SESSION;
            bad_object.handle = reinterpret_cast<uint64_t const&>(session);
            std::vector<XrLoaderLogObjectInfo> loader_objects;
            loader_objects.push_back(bad_object);
            LoaderLogger::LogValidationErrorMessage("VUID-xrDeleteStoredAnchorMSFT-session-parameter", "xrDeleteStoredAnchorMSFT",
                                                    "session is not a valid XrSession", loader_objects);
            return XR_ERROR_HANDLE_INVALID;
        }
        const std::unique_ptr<XrGeneratedDispatchTable>& dispatch_table = loader_instance->DispatchTable();
        if (!loader_instance->ExtensionIsEnabled("XR_MSFT_spatial_anchor_storage")) {
            LoaderLogger::LogValidationErrorMessage("VUID-xrDeleteStoredAnchorMSFT-extension-notenabled",
                                                    "xrDeleteStoredAnchorMSFT",
                                                    "The XR_MSFT_spatial_anchor_storage extension has not been enabled prior to calling xrDeleteStoredAnchorMSFT");
            return XR_ERROR_FUNCTION_UNSUPPORTED;
        }

        return dispatch_table->DeleteStoredAnchorMSFT(session, anchorName);
    } catch (...) {
        LoaderLogger::LogErrorMessage("xrDeleteStoredAnchorMSFT", "xrDeleteStoredAnchorMSFT trampoline encountered an unknown error");
        // NOTE: Most calls only allow XR_SUCCESS as a return code
        return XR_SUCCESS;
    }
}



LOADER_EXPORT XRAPI_ATTR XrResult XRAPI_CALL xrGetInstanceProcAddr(XrInstance instance, const char* name,
                                                                   PFN_xrVoidFunction* function) {
    if (nullptr == function) {
        LoaderLogger::LogValidationErrorMessage("VUID-xrGetInstanceProcAddr-function-parameter",
                                                "xrGetInstanceProcAddr", "Invalid Function pointer");
            return XR_ERROR_VALIDATION_FAILURE;
    }
    // Initialize the function to nullptr in case it does not get caught in a known case
    *function = nullptr;
    if (name[0] == 'x' && name[1] == 'r') {
        std::string func_name = &name[2];
        LoaderInstance * const loader_instance = TryLookupLoaderInstance(instance);
        if (loader_instance == nullptr) {
            std::string error_str = "XR_NULL_HANDLE for instance but query for ";
            error_str += name;
            error_str += " requires a valid instance";
            LoaderLogger::LogValidationErrorMessage("VUID-xrGetInstanceProcAddr-instance-parameter",
                                                    "xrGetInstanceProcAddr", error_str);
            return XR_ERROR_HANDLE_INVALID;
        }

        // ---- Core 0.90 commands

        if (func_name == "GetInstanceProcAddr") {
            *function = reinterpret_cast<PFN_xrVoidFunction>(xrGetInstanceProcAddr);
        } else if (func_name == "EnumerateApiLayerProperties") {
            *function = reinterpret_cast<PFN_xrVoidFunction>(xrEnumerateApiLayerProperties);
        } else if (func_name == "EnumerateInstanceExtensionProperties") {
            *function = reinterpret_cast<PFN_xrVoidFunction>(xrEnumerateInstanceExtensionProperties);
        } else if (func_name == "CreateInstance") {
            *function = reinterpret_cast<PFN_xrVoidFunction>(xrCreateInstance);
        } else if (func_name == "DestroyInstance") {
            *function = reinterpret_cast<PFN_xrVoidFunction>(xrDestroyInstance);
        } else if (func_name == "GetInstanceProperties") {
            *function = reinterpret_cast<PFN_xrVoidFunction>(xrGetInstanceProperties);
        } else if (func_name == "PollEvent") {
            *function = reinterpret_cast<PFN_xrVoidFunction>(xrPollEvent);
        } else if (func_name == "ResultToString") {
            *function = reinterpret_cast<PFN_xrVoidFunction>(xrResultToString);
        } else if (func_name == "StructureTypeToString") {
            *function = reinterpret_cast<PFN_xrVoidFunction>(xrStructureTypeToString);
        } else if (func_name == "GetSystem") {
            *function = reinterpret_cast<PFN_xrVoidFunction>(xrGetSystem);
        } else if (func_name == "GetSystemProperties") {
            *function = reinterpret_cast<PFN_xrVoidFunction>(xrGetSystemProperties);
        } else if (func_name == "EnumerateEnvironmentBlendModes") {
            *function = reinterpret_cast<PFN_xrVoidFunction>(xrEnumerateEnvironmentBlendModes);
        } else if (func_name == "CreateSession") {
            *function = reinterpret_cast<PFN_xrVoidFunction>(xrCreateSession);
        } else if (func_name == "DestroySession") {
            *function = reinterpret_cast<PFN_xrVoidFunction>(loader_instance->DispatchTable()->DestroySession);
        } else if (func_name == "EnumerateReferenceSpaces") {
            *function = reinterpret_cast<PFN_xrVoidFunction>(loader_instance->DispatchTable()->EnumerateReferenceSpaces);
        } else if (func_name == "CreateReferenceSpace") {
            *function = reinterpret_cast<PFN_xrVoidFunction>(loader_instance->DispatchTable()->CreateReferenceSpace);
        } else if (func_name == "GetReferenceSpaceBoundsRect") {
            *function = reinterpret_cast<PFN_xrVoidFunction>(loader_instance->DispatchTable()->GetReferenceSpaceBoundsRect);
        } else if (func_name == "CreateActionSpace") {
            *function = reinterpret_cast<PFN_xrVoidFunction>(loader_instance->DispatchTable()->CreateActionSpace);
        } else if (func_name == "LocateSpace") {
            *function = reinterpret_cast<PFN_xrVoidFunction>(loader_instance->DispatchTable()->LocateSpace);
        } else if (func_name == "DestroySpace") {
            *function = reinterpret_cast<PFN_xrVoidFunction>(loader_instance->DispatchTable()->DestroySpace);
        } else if (func_name == "EnumerateViewConfigurations") {
            *function = reinterpret_cast<PFN_xrVoidFunction>(xrEnumerateViewConfigurations);
        } else if (func_name == "GetViewConfigurationProperties") {
            *function = reinterpret_cast<PFN_xrVoidFunction>(xrGetViewConfigurationProperties);
        } else if (func_name == "EnumerateViewConfigurationViews") {
            *function = reinterpret_cast<PFN_xrVoidFunction>(xrEnumerateViewConfigurationViews);
        } else if (func_name == "EnumerateSwapchainFormats") {
            *function = reinterpret_cast<PFN_xrVoidFunction>(loader_instance->DispatchTable()->EnumerateSwapchainFormats);
        } else if (func_name == "CreateSwapchain") {
            *function = reinterpret_cast<PFN_xrVoidFunction>(loader_instance->DispatchTable()->CreateSwapchain);
        } else if (func_name == "DestroySwapchain") {
            *function = reinterpret_cast<PFN_xrVoidFunction>(loader_instance->DispatchTable()->DestroySwapchain);
        } else if (func_name == "EnumerateSwapchainImages") {
            *function = reinterpret_cast<PFN_xrVoidFunction>(loader_instance->DispatchTable()->EnumerateSwapchainImages);
        } else if (func_name == "AcquireSwapchainImage") {
            *function = reinterpret_cast<PFN_xrVoidFunction>(loader_instance->DispatchTable()->AcquireSwapchainImage);
        } else if (func_name == "WaitSwapchainImage") {
            *function = reinterpret_cast<PFN_xrVoidFunction>(loader_instance->DispatchTable()->WaitSwapchainImage);
        } else if (func_name == "ReleaseSwapchainImage") {
            *function = reinterpret_cast<PFN_xrVoidFunction>(loader_instance->DispatchTable()->ReleaseSwapchainImage);
        } else if (func_name == "BeginSession") {
            *function = reinterpret_cast<PFN_xrVoidFunction>(loader_instance->DispatchTable()->BeginSession);
        } else if (func_name == "EndSession") {
            *function = reinterpret_cast<PFN_xrVoidFunction>(loader_instance->DispatchTable()->EndSession);
        } else if (func_name == "WaitFrame") {
            *function = reinterpret_cast<PFN_xrVoidFunction>(loader_instance->DispatchTable()->WaitFrame);
        } else if (func_name == "BeginFrame") {
            *function = reinterpret_cast<PFN_xrVoidFunction>(loader_instance->DispatchTable()->BeginFrame);
        } else if (func_name == "EndFrame") {
            *function = reinterpret_cast<PFN_xrVoidFunction>(loader_instance->DispatchTable()->EndFrame);
        } else if (func_name == "LocateViews") {
            *function = reinterpret_cast<PFN_xrVoidFunction>(loader_instance->DispatchTable()->LocateViews);
        } else if (func_name == "StringToPath") {
            *function = reinterpret_cast<PFN_xrVoidFunction>(xrStringToPath);
        } else if (func_name == "PathToString") {
            *function = reinterpret_cast<PFN_xrVoidFunction>(xrPathToString);
        } else if (func_name == "CreateActionSet") {
            *function = reinterpret_cast<PFN_xrVoidFunction>(loader_instance->DispatchTable()->CreateActionSet);
        } else if (func_name == "DestroyActionSet") {
            *function = reinterpret_cast<PFN_xrVoidFunction>(loader_instance->DispatchTable()->DestroyActionSet);
        } else if (func_name == "CreateAction") {
            *function = reinterpret_cast<PFN_xrVoidFunction>(loader_instance->DispatchTable()->CreateAction);
        } else if (func_name == "DestroyAction") {
            *function = reinterpret_cast<PFN_xrVoidFunction>(loader_instance->DispatchTable()->DestroyAction);
        } else if (func_name == "SetInteractionProfileSuggestedBindings") {
            *function = reinterpret_cast<PFN_xrVoidFunction>(loader_instance->DispatchTable()->SetInteractionProfileSuggestedBindings);
        } else if (func_name == "GetCurrentInteractionProfile") {
            *function = reinterpret_cast<PFN_xrVoidFunction>(loader_instance->DispatchTable()->GetCurrentInteractionProfile);
        } else if (func_name == "GetActionStateBoolean") {
            *function = reinterpret_cast<PFN_xrVoidFunction>(loader_instance->DispatchTable()->GetActionStateBoolean);
        } else if (func_name == "GetActionStateVector1f") {
            *function = reinterpret_cast<PFN_xrVoidFunction>(loader_instance->DispatchTable()->GetActionStateVector1f);
        } else if (func_name == "GetActionStateVector2f") {
            *function = reinterpret_cast<PFN_xrVoidFunction>(loader_instance->DispatchTable()->GetActionStateVector2f);
        } else if (func_name == "GetActionStatePose") {
            *function = reinterpret_cast<PFN_xrVoidFunction>(loader_instance->DispatchTable()->GetActionStatePose);
        } else if (func_name == "SyncActionData") {
            *function = reinterpret_cast<PFN_xrVoidFunction>(loader_instance->DispatchTable()->SyncActionData);
        } else if (func_name == "GetBoundSourcesForAction") {
            *function = reinterpret_cast<PFN_xrVoidFunction>(loader_instance->DispatchTable()->GetBoundSourcesForAction);
        } else if (func_name == "GetInputSourceLocalizedName") {
            *function = reinterpret_cast<PFN_xrVoidFunction>(loader_instance->DispatchTable()->GetInputSourceLocalizedName);
        } else if (func_name == "ApplyHapticFeedback") {
            *function = reinterpret_cast<PFN_xrVoidFunction>(loader_instance->DispatchTable()->ApplyHapticFeedback);
        } else if (func_name == "StopHapticFeedback") {
            *function = reinterpret_cast<PFN_xrVoidFunction>(loader_instance->DispatchTable()->StopHapticFeedback);

        // ---- XR_KHR_android_thread_settings extension commands

#if defined(XR_USE_PLATFORM_ANDROID)
        } else if (func_name == "SetAndroidApplicationThreadKHR") {
            if (loader_instance->ExtensionIsEnabled("XR_KHR_android_thread_settings")) {
                *function = reinterpret_cast<PFN_xrVoidFunction>(loader_instance->DispatchTable()->SetAndroidApplicationThreadKHR);
            }
#endif // defined(XR_USE_PLATFORM_ANDROID)

        // ---- XR_KHR_android_surface_swapchain extension commands

#if defined(XR_USE_PLATFORM_ANDROID)
        } else if (func_name == "CreateSwapchainAndroidSurfaceKHR") {
            if (loader_instance->ExtensionIsEnabled("XR_KHR_android_surface_swapchain")) {
                *function = reinterpret_cast<PFN_xrVoidFunction>(loader_instance->DispatchTable()->CreateSwapchainAndroidSurfaceKHR);
            }
#endif // defined(XR_USE_PLATFORM_ANDROID)

        // ---- XR_KHR_opengl_enable extension commands

#if defined(XR_USE_GRAPHICS_API_OPENGL)
        } else if (func_name == "GetOpenGLGraphicsRequirementsKHR") {
            if (loader_instance->ExtensionIsEnabled("XR_KHR_opengl_enable")) {
                *function = reinterpret_cast<PFN_xrVoidFunction>(xrGetOpenGLGraphicsRequirementsKHR);
            }
#endif // defined(XR_USE_GRAPHICS_API_OPENGL)

        // ---- XR_KHR_opengl_es_enable extension commands

#if defined(XR_USE_GRAPHICS_API_OPENGL_ES)
        } else if (func_name == "GetOpenGLESGraphicsRequirementsKHR") {
            if (loader_instance->ExtensionIsEnabled("XR_KHR_opengl_es_enable")) {
                *function = reinterpret_cast<PFN_xrVoidFunction>(xrGetOpenGLESGraphicsRequirementsKHR);
            }
#endif // defined(XR_USE_GRAPHICS_API_OPENGL_ES)

        // ---- XR_KHR_vulkan_enable extension commands

#if defined(XR_USE_GRAPHICS_API_VULKAN)
        } else if (func_name == "GetVulkanInstanceExtensionsKHR") {
            if (loader_instance->ExtensionIsEnabled("XR_KHR_vulkan_enable")) {
                *function = reinterpret_cast<PFN_xrVoidFunction>(xrGetVulkanInstanceExtensionsKHR);
            }
#endif // defined(XR_USE_GRAPHICS_API_VULKAN)
#if defined(XR_USE_GRAPHICS_API_VULKAN)
        } else if (func_name == "GetVulkanDeviceExtensionsKHR") {
            if (loader_instance->ExtensionIsEnabled("XR_KHR_vulkan_enable")) {
                *function = reinterpret_cast<PFN_xrVoidFunction>(xrGetVulkanDeviceExtensionsKHR);
            }
#endif // defined(XR_USE_GRAPHICS_API_VULKAN)
#if defined(XR_USE_GRAPHICS_API_VULKAN)
        } else if (func_name == "GetVulkanGraphicsDeviceKHR") {
            if (loader_instance->ExtensionIsEnabled("XR_KHR_vulkan_enable")) {
                *function = reinterpret_cast<PFN_xrVoidFunction>(xrGetVulkanGraphicsDeviceKHR);
            }
#endif // defined(XR_USE_GRAPHICS_API_VULKAN)
#if defined(XR_USE_GRAPHICS_API_VULKAN)
        } else if (func_name == "GetVulkanGraphicsRequirementsKHR") {
            if (loader_instance->ExtensionIsEnabled("XR_KHR_vulkan_enable")) {
                *function = reinterpret_cast<PFN_xrVoidFunction>(xrGetVulkanGraphicsRequirementsKHR);
            }
#endif // defined(XR_USE_GRAPHICS_API_VULKAN)

        // ---- XR_KHR_D3D10_enable extension commands

#if defined(XR_USE_GRAPHICS_API_D3D10)
        } else if (func_name == "GetD3D10GraphicsRequirementsKHR") {
            if (loader_instance->ExtensionIsEnabled("XR_KHR_D3D10_enable")) {
                *function = reinterpret_cast<PFN_xrVoidFunction>(xrGetD3D10GraphicsRequirementsKHR);
            }
#endif // defined(XR_USE_GRAPHICS_API_D3D10)

        // ---- XR_KHR_D3D11_enable extension commands

#if defined(XR_USE_GRAPHICS_API_D3D11)
        } else if (func_name == "GetD3D11GraphicsRequirementsKHR") {
            if (loader_instance->ExtensionIsEnabled("XR_KHR_D3D11_enable")) {
                *function = reinterpret_cast<PFN_xrVoidFunction>(xrGetD3D11GraphicsRequirementsKHR);
            }
#endif // defined(XR_USE_GRAPHICS_API_D3D11)

        // ---- XR_KHR_D3D12_enable extension commands

#if defined(XR_USE_GRAPHICS_API_D3D12)
        } else if (func_name == "GetD3D12GraphicsRequirementsKHR") {
            if (loader_instance->ExtensionIsEnabled("XR_KHR_D3D12_enable")) {
                *function = reinterpret_cast<PFN_xrVoidFunction>(xrGetD3D12GraphicsRequirementsKHR);
            }
#endif // defined(XR_USE_GRAPHICS_API_D3D12)

        // ---- XR_KHR_visibility_mask extension commands

        } else if (func_name == "GetVisibilityMaskKHR") {
            if (loader_instance->ExtensionIsEnabled("XR_KHR_visibility_mask")) {
                *function = reinterpret_cast<PFN_xrVoidFunction>(loader_instance->DispatchTable()->GetVisibilityMaskKHR);
            }

        // ---- XR_KHR_win32_convert_performance_counter_time extension commands

#if defined(XR_USE_PLATFORM_WIN32)
        } else if (func_name == "ConvertWin32PerformanceCounterToTimeKHR") {
            if (loader_instance->ExtensionIsEnabled("XR_KHR_win32_convert_performance_counter_time")) {
                *function = reinterpret_cast<PFN_xrVoidFunction>(xrConvertWin32PerformanceCounterToTimeKHR);
            }
#endif // defined(XR_USE_PLATFORM_WIN32)
#if defined(XR_USE_PLATFORM_WIN32)
        } else if (func_name == "ConvertTimeToWin32PerformanceCounterKHR") {
            if (loader_instance->ExtensionIsEnabled("XR_KHR_win32_convert_performance_counter_time")) {
                *function = reinterpret_cast<PFN_xrVoidFunction>(xrConvertTimeToWin32PerformanceCounterKHR);
            }
#endif // defined(XR_USE_PLATFORM_WIN32)

        // ---- XR_KHR_convert_timespec_time extension commands

#if defined(XR_USE_TIMESPEC)
        } else if (func_name == "ConvertTimespecTimeToTimeKHR") {
            if (loader_instance->ExtensionIsEnabled("XR_KHR_convert_timespec_time")) {
                *function = reinterpret_cast<PFN_xrVoidFunction>(xrConvertTimespecTimeToTimeKHR);
            }
#endif // defined(XR_USE_TIMESPEC)
#if defined(XR_USE_TIMESPEC)
        } else if (func_name == "ConvertTimeToTimespecTimeKHR") {
            if (loader_instance->ExtensionIsEnabled("XR_KHR_convert_timespec_time")) {
                *function = reinterpret_cast<PFN_xrVoidFunction>(xrConvertTimeToTimespecTimeKHR);
            }
#endif // defined(XR_USE_TIMESPEC)

        // ---- XR_EXT_debug_utils extension commands

        } else if (func_name == "SetDebugUtilsObjectNameEXT") {
            if (loader_instance->ExtensionIsEnabled("XR_EXT_debug_utils")) {
                *function = reinterpret_cast<PFN_xrVoidFunction>(xrSetDebugUtilsObjectNameEXT);
            }
        } else if (func_name == "CreateDebugUtilsMessengerEXT") {
            if (loader_instance->ExtensionIsEnabled("XR_EXT_debug_utils")) {
                *function = reinterpret_cast<PFN_xrVoidFunction>(xrCreateDebugUtilsMessengerEXT);
            }
        } else if (func_name == "DestroyDebugUtilsMessengerEXT") {
            if (loader_instance->ExtensionIsEnabled("XR_EXT_debug_utils")) {
                *function = reinterpret_cast<PFN_xrVoidFunction>(xrDestroyDebugUtilsMessengerEXT);
            }
        } else if (func_name == "SubmitDebugUtilsMessageEXT") {
            if (loader_instance->ExtensionIsEnabled("XR_EXT_debug_utils")) {
                *function = reinterpret_cast<PFN_xrVoidFunction>(xrSubmitDebugUtilsMessageEXT);
            }
        } else if (func_name == "SessionBeginDebugUtilsLabelRegionEXT") {
            if (loader_instance->ExtensionIsEnabled("XR_EXT_debug_utils")) {
                *function = reinterpret_cast<PFN_xrVoidFunction>(xrSessionBeginDebugUtilsLabelRegionEXT);
            }
        } else if (func_name == "SessionEndDebugUtilsLabelRegionEXT") {
            if (loader_instance->ExtensionIsEnabled("XR_EXT_debug_utils")) {
                *function = reinterpret_cast<PFN_xrVoidFunction>(xrSessionEndDebugUtilsLabelRegionEXT);
            }
        } else if (func_name == "SessionInsertDebugUtilsLabelEXT") {
            if (loader_instance->ExtensionIsEnabled("XR_EXT_debug_utils")) {
                *function = reinterpret_cast<PFN_xrVoidFunction>(xrSessionInsertDebugUtilsLabelEXT);
            }

        // ---- XR_MSFT_secondary_view_configuration extension commands

        } else if (func_name == "SubmitFrameCompositionLayersMSFT") {
            if (loader_instance->ExtensionIsEnabled("XR_MSFT_secondary_view_configuration")) {
                *function = reinterpret_cast<PFN_xrVoidFunction>(loader_instance->DispatchTable()->SubmitFrameCompositionLayersMSFT);
            }

        // ---- XR_MSFT_spatial_perception_bridge extension commands

#if defined(XR_USE_PLATFORM_WIN32)
        } else if (func_name == "CreateSpaceFromSpatialCoordinateSystemMSFT") {
            if (loader_instance->ExtensionIsEnabled("XR_MSFT_spatial_perception_bridge")) {
                *function = reinterpret_cast<PFN_xrVoidFunction>(loader_instance->DispatchTable()->CreateSpaceFromSpatialCoordinateSystemMSFT);
            }
#endif // defined(XR_USE_PLATFORM_WIN32)

        // ---- XR_MSFT_controller_render_model extension commands

        } else if (func_name == "LoadControllerRenderModelMSFT") {
            if (loader_instance->ExtensionIsEnabled("XR_MSFT_controller_render_model")) {
                *function = reinterpret_cast<PFN_xrVoidFunction>(loader_instance->DispatchTable()->LoadControllerRenderModelMSFT);
            }

        // ---- XR_MSFT_spatial_anchor extension commands

        } else if (func_name == "CreateSpatialAnchorSpaceMSFT") {
            if (loader_instance->ExtensionIsEnabled("XR_MSFT_spatial_anchor")) {
                *function = reinterpret_cast<PFN_xrVoidFunction>(loader_instance->DispatchTable()->CreateSpatialAnchorSpaceMSFT);
            }
        } else if (func_name == "CreateSpatialAnchorMSFT") {
            if (loader_instance->ExtensionIsEnabled("XR_MSFT_spatial_anchor")) {
                *function = reinterpret_cast<PFN_xrVoidFunction>(loader_instance->DispatchTable()->CreateSpatialAnchorMSFT);
            }
        } else if (func_name == "DestroySpatialAnchorMSFT") {
            if (loader_instance->ExtensionIsEnabled("XR_MSFT_spatial_anchor")) {
                *function = reinterpret_cast<PFN_xrVoidFunction>(loader_instance->DispatchTable()->DestroySpatialAnchorMSFT);
            }

        // ---- XR_MSFT_spatial_anchor_storage extension commands

        } else if (func_name == "StoreSpatialAnchorMSFT") {
            if (loader_instance->ExtensionIsEnabled("XR_MSFT_spatial_anchor_storage")) {
                *function = reinterpret_cast<PFN_xrVoidFunction>(loader_instance->DispatchTable()->StoreSpatialAnchorMSFT);
            }
        } else if (func_name == "EnumerateStoredAnchorsMSFT") {
            if (loader_instance->ExtensionIsEnabled("XR_MSFT_spatial_anchor_storage")) {
                *function = reinterpret_cast<PFN_xrVoidFunction>(loader_instance->DispatchTable()->EnumerateStoredAnchorsMSFT);
            }
        } else if (func_name == "CreateSpatialAnchorFromStoredAnchorNameMSFT") {
            if (loader_instance->ExtensionIsEnabled("XR_MSFT_spatial_anchor_storage")) {
                *function = reinterpret_cast<PFN_xrVoidFunction>(loader_instance->DispatchTable()->CreateSpatialAnchorFromStoredAnchorNameMSFT);
            }
        } else if (func_name == "DeleteStoredAnchorMSFT") {
            if (loader_instance->ExtensionIsEnabled("XR_MSFT_spatial_anchor_storage")) {
                *function = reinterpret_cast<PFN_xrVoidFunction>(loader_instance->DispatchTable()->DeleteStoredAnchorMSFT);
            }
        }
    }
    if (*function == nullptr) {
        return XR_ERROR_FUNCTION_UNSUPPORTED;
    } else {
        return XR_SUCCESS;
    }
}

// Terminator GetInstanceProcAddr function
XRAPI_ATTR XrResult XRAPI_CALL LoaderXrTermGetInstanceProcAddr(XrInstance instance, const char* name,
                                                               PFN_xrVoidFunction* function) {

    // A few instance commands need to go through a loader terminator.
    // Otherwise, go directly to the runtime version of the command if it exists.
    if (0 == strcmp(name, "xrGetInstanceProcAddr")) {
        *function = reinterpret_cast<PFN_xrVoidFunction>(LoaderXrTermGetInstanceProcAddr);
    } else if (0 == strcmp(name, "xrCreateInstance")) {
        *function = reinterpret_cast<PFN_xrVoidFunction>(LoaderXrTermCreateInstance);
    } else if (0 == strcmp(name, "xrDestroyInstance")) {
        *function = reinterpret_cast<PFN_xrVoidFunction>(LoaderXrTermDestroyInstance);
    } else if (0 == strcmp(name, "xrResultToString")) {
        *function = reinterpret_cast<PFN_xrVoidFunction>(LoaderGenTermXrResultToString);
    } else if (0 == strcmp(name, "xrStructureTypeToString")) {
        *function = reinterpret_cast<PFN_xrVoidFunction>(LoaderGenTermXrStructureTypeToString);
    } else if (0 == strcmp(name, "xrSetDebugUtilsObjectNameEXT")) {
        *function = reinterpret_cast<PFN_xrVoidFunction>(LoaderXrTermSetDebugUtilsObjectNameEXT);
    } else if (0 == strcmp(name, "xrCreateDebugUtilsMessengerEXT")) {
        *function = reinterpret_cast<PFN_xrVoidFunction>(LoaderXrTermCreateDebugUtilsMessengerEXT);
    } else if (0 == strcmp(name, "xrDestroyDebugUtilsMessengerEXT")) {
        *function = reinterpret_cast<PFN_xrVoidFunction>(LoaderXrTermDestroyDebugUtilsMessengerEXT);
    } else if (0 == strcmp(name, "xrSubmitDebugUtilsMessageEXT")) {
        *function = reinterpret_cast<PFN_xrVoidFunction>(LoaderXrTermSubmitDebugUtilsMessageEXT);
    } else if (0 == strcmp(name, "xrCreateApiLayerInstance")) {
        // Special layer version of xrCreateInstance terminator.  If we get called this by a layer,
        // we simply re-direct the information back into the standard xrCreateInstance terminator.
        *function = reinterpret_cast<PFN_xrVoidFunction>(LoaderXrTermCreateApiLayerInstance);
    }
    if (nullptr != *function) {
        return XR_SUCCESS;
    }
    return RuntimeInterface::GetInstanceProcAddr(instance, name, function);
}

// Instance Init Dispatch Table (put all terminators in first)
void LoaderGenInitInstanceDispatchTable(XrInstance instance, std::unique_ptr<XrGeneratedDispatchTable>& table) {

    // ---- Core 0_90 commands
    table->GetInstanceProcAddr = LoaderXrTermGetInstanceProcAddr;
    table->EnumerateApiLayerProperties = nullptr;
    table->EnumerateInstanceExtensionProperties = nullptr;
    table->CreateInstance = LoaderXrTermCreateInstance;
    table->DestroyInstance = LoaderXrTermDestroyInstance;
    RuntimeInterface::GetInstanceProcAddr(instance, "xrGetInstanceProperties", reinterpret_cast<PFN_xrVoidFunction*>(&table->GetInstanceProperties));
    RuntimeInterface::GetInstanceProcAddr(instance, "xrPollEvent", reinterpret_cast<PFN_xrVoidFunction*>(&table->PollEvent));
    RuntimeInterface::GetInstanceProcAddr(instance, "xrResultToString", reinterpret_cast<PFN_xrVoidFunction*>(&table->ResultToString));
    RuntimeInterface::GetInstanceProcAddr(instance, "xrStructureTypeToString", reinterpret_cast<PFN_xrVoidFunction*>(&table->StructureTypeToString));
    RuntimeInterface::GetInstanceProcAddr(instance, "xrGetSystem", reinterpret_cast<PFN_xrVoidFunction*>(&table->GetSystem));
    RuntimeInterface::GetInstanceProcAddr(instance, "xrGetSystemProperties", reinterpret_cast<PFN_xrVoidFunction*>(&table->GetSystemProperties));
    RuntimeInterface::GetInstanceProcAddr(instance, "xrEnumerateEnvironmentBlendModes", reinterpret_cast<PFN_xrVoidFunction*>(&table->EnumerateEnvironmentBlendModes));
    RuntimeInterface::GetInstanceProcAddr(instance, "xrCreateSession", reinterpret_cast<PFN_xrVoidFunction*>(&table->CreateSession));
    RuntimeInterface::GetInstanceProcAddr(instance, "xrDestroySession", reinterpret_cast<PFN_xrVoidFunction*>(&table->DestroySession));
    RuntimeInterface::GetInstanceProcAddr(instance, "xrEnumerateReferenceSpaces", reinterpret_cast<PFN_xrVoidFunction*>(&table->EnumerateReferenceSpaces));
    RuntimeInterface::GetInstanceProcAddr(instance, "xrCreateReferenceSpace", reinterpret_cast<PFN_xrVoidFunction*>(&table->CreateReferenceSpace));
    RuntimeInterface::GetInstanceProcAddr(instance, "xrGetReferenceSpaceBoundsRect", reinterpret_cast<PFN_xrVoidFunction*>(&table->GetReferenceSpaceBoundsRect));
    RuntimeInterface::GetInstanceProcAddr(instance, "xrCreateActionSpace", reinterpret_cast<PFN_xrVoidFunction*>(&table->CreateActionSpace));
    RuntimeInterface::GetInstanceProcAddr(instance, "xrLocateSpace", reinterpret_cast<PFN_xrVoidFunction*>(&table->LocateSpace));
    RuntimeInterface::GetInstanceProcAddr(instance, "xrDestroySpace", reinterpret_cast<PFN_xrVoidFunction*>(&table->DestroySpace));
    RuntimeInterface::GetInstanceProcAddr(instance, "xrEnumerateViewConfigurations", reinterpret_cast<PFN_xrVoidFunction*>(&table->EnumerateViewConfigurations));
    RuntimeInterface::GetInstanceProcAddr(instance, "xrGetViewConfigurationProperties", reinterpret_cast<PFN_xrVoidFunction*>(&table->GetViewConfigurationProperties));
    RuntimeInterface::GetInstanceProcAddr(instance, "xrEnumerateViewConfigurationViews", reinterpret_cast<PFN_xrVoidFunction*>(&table->EnumerateViewConfigurationViews));
    RuntimeInterface::GetInstanceProcAddr(instance, "xrEnumerateSwapchainFormats", reinterpret_cast<PFN_xrVoidFunction*>(&table->EnumerateSwapchainFormats));
    RuntimeInterface::GetInstanceProcAddr(instance, "xrCreateSwapchain", reinterpret_cast<PFN_xrVoidFunction*>(&table->CreateSwapchain));
    RuntimeInterface::GetInstanceProcAddr(instance, "xrDestroySwapchain", reinterpret_cast<PFN_xrVoidFunction*>(&table->DestroySwapchain));
    RuntimeInterface::GetInstanceProcAddr(instance, "xrEnumerateSwapchainImages", reinterpret_cast<PFN_xrVoidFunction*>(&table->EnumerateSwapchainImages));
    RuntimeInterface::GetInstanceProcAddr(instance, "xrAcquireSwapchainImage", reinterpret_cast<PFN_xrVoidFunction*>(&table->AcquireSwapchainImage));
    RuntimeInterface::GetInstanceProcAddr(instance, "xrWaitSwapchainImage", reinterpret_cast<PFN_xrVoidFunction*>(&table->WaitSwapchainImage));
    RuntimeInterface::GetInstanceProcAddr(instance, "xrReleaseSwapchainImage", reinterpret_cast<PFN_xrVoidFunction*>(&table->ReleaseSwapchainImage));
    RuntimeInterface::GetInstanceProcAddr(instance, "xrBeginSession", reinterpret_cast<PFN_xrVoidFunction*>(&table->BeginSession));
    RuntimeInterface::GetInstanceProcAddr(instance, "xrEndSession", reinterpret_cast<PFN_xrVoidFunction*>(&table->EndSession));
    RuntimeInterface::GetInstanceProcAddr(instance, "xrWaitFrame", reinterpret_cast<PFN_xrVoidFunction*>(&table->WaitFrame));
    RuntimeInterface::GetInstanceProcAddr(instance, "xrBeginFrame", reinterpret_cast<PFN_xrVoidFunction*>(&table->BeginFrame));
    RuntimeInterface::GetInstanceProcAddr(instance, "xrEndFrame", reinterpret_cast<PFN_xrVoidFunction*>(&table->EndFrame));
    RuntimeInterface::GetInstanceProcAddr(instance, "xrLocateViews", reinterpret_cast<PFN_xrVoidFunction*>(&table->LocateViews));
    RuntimeInterface::GetInstanceProcAddr(instance, "xrStringToPath", reinterpret_cast<PFN_xrVoidFunction*>(&table->StringToPath));
    RuntimeInterface::GetInstanceProcAddr(instance, "xrPathToString", reinterpret_cast<PFN_xrVoidFunction*>(&table->PathToString));
    RuntimeInterface::GetInstanceProcAddr(instance, "xrCreateActionSet", reinterpret_cast<PFN_xrVoidFunction*>(&table->CreateActionSet));
    RuntimeInterface::GetInstanceProcAddr(instance, "xrDestroyActionSet", reinterpret_cast<PFN_xrVoidFunction*>(&table->DestroyActionSet));
    RuntimeInterface::GetInstanceProcAddr(instance, "xrCreateAction", reinterpret_cast<PFN_xrVoidFunction*>(&table->CreateAction));
    RuntimeInterface::GetInstanceProcAddr(instance, "xrDestroyAction", reinterpret_cast<PFN_xrVoidFunction*>(&table->DestroyAction));
    RuntimeInterface::GetInstanceProcAddr(instance, "xrSetInteractionProfileSuggestedBindings", reinterpret_cast<PFN_xrVoidFunction*>(&table->SetInteractionProfileSuggestedBindings));
    RuntimeInterface::GetInstanceProcAddr(instance, "xrGetCurrentInteractionProfile", reinterpret_cast<PFN_xrVoidFunction*>(&table->GetCurrentInteractionProfile));
    RuntimeInterface::GetInstanceProcAddr(instance, "xrGetActionStateBoolean", reinterpret_cast<PFN_xrVoidFunction*>(&table->GetActionStateBoolean));
    RuntimeInterface::GetInstanceProcAddr(instance, "xrGetActionStateVector1f", reinterpret_cast<PFN_xrVoidFunction*>(&table->GetActionStateVector1f));
    RuntimeInterface::GetInstanceProcAddr(instance, "xrGetActionStateVector2f", reinterpret_cast<PFN_xrVoidFunction*>(&table->GetActionStateVector2f));
    RuntimeInterface::GetInstanceProcAddr(instance, "xrGetActionStatePose", reinterpret_cast<PFN_xrVoidFunction*>(&table->GetActionStatePose));
    RuntimeInterface::GetInstanceProcAddr(instance, "xrSyncActionData", reinterpret_cast<PFN_xrVoidFunction*>(&table->SyncActionData));
    RuntimeInterface::GetInstanceProcAddr(instance, "xrGetBoundSourcesForAction", reinterpret_cast<PFN_xrVoidFunction*>(&table->GetBoundSourcesForAction));
    RuntimeInterface::GetInstanceProcAddr(instance, "xrGetInputSourceLocalizedName", reinterpret_cast<PFN_xrVoidFunction*>(&table->GetInputSourceLocalizedName));
    RuntimeInterface::GetInstanceProcAddr(instance, "xrApplyHapticFeedback", reinterpret_cast<PFN_xrVoidFunction*>(&table->ApplyHapticFeedback));
    RuntimeInterface::GetInstanceProcAddr(instance, "xrStopHapticFeedback", reinterpret_cast<PFN_xrVoidFunction*>(&table->StopHapticFeedback));

    // ---- XR_KHR_android_thread_settings extension commands
#if defined(XR_USE_PLATFORM_ANDROID)
    RuntimeInterface::GetInstanceProcAddr(instance, "xrSetAndroidApplicationThreadKHR", reinterpret_cast<PFN_xrVoidFunction*>(&table->SetAndroidApplicationThreadKHR));
#endif // defined(XR_USE_PLATFORM_ANDROID)

    // ---- XR_KHR_android_surface_swapchain extension commands
#if defined(XR_USE_PLATFORM_ANDROID)
    RuntimeInterface::GetInstanceProcAddr(instance, "xrCreateSwapchainAndroidSurfaceKHR", reinterpret_cast<PFN_xrVoidFunction*>(&table->CreateSwapchainAndroidSurfaceKHR));
#endif // defined(XR_USE_PLATFORM_ANDROID)

    // ---- XR_KHR_opengl_enable extension commands
#if defined(XR_USE_GRAPHICS_API_OPENGL)
    RuntimeInterface::GetInstanceProcAddr(instance, "xrGetOpenGLGraphicsRequirementsKHR", reinterpret_cast<PFN_xrVoidFunction*>(&table->GetOpenGLGraphicsRequirementsKHR));
#endif // defined(XR_USE_GRAPHICS_API_OPENGL)

    // ---- XR_KHR_opengl_es_enable extension commands
#if defined(XR_USE_GRAPHICS_API_OPENGL_ES)
    RuntimeInterface::GetInstanceProcAddr(instance, "xrGetOpenGLESGraphicsRequirementsKHR", reinterpret_cast<PFN_xrVoidFunction*>(&table->GetOpenGLESGraphicsRequirementsKHR));
#endif // defined(XR_USE_GRAPHICS_API_OPENGL_ES)

    // ---- XR_KHR_vulkan_enable extension commands
#if defined(XR_USE_GRAPHICS_API_VULKAN)
    RuntimeInterface::GetInstanceProcAddr(instance, "xrGetVulkanInstanceExtensionsKHR", reinterpret_cast<PFN_xrVoidFunction*>(&table->GetVulkanInstanceExtensionsKHR));
#endif // defined(XR_USE_GRAPHICS_API_VULKAN)
#if defined(XR_USE_GRAPHICS_API_VULKAN)
    RuntimeInterface::GetInstanceProcAddr(instance, "xrGetVulkanDeviceExtensionsKHR", reinterpret_cast<PFN_xrVoidFunction*>(&table->GetVulkanDeviceExtensionsKHR));
#endif // defined(XR_USE_GRAPHICS_API_VULKAN)
#if defined(XR_USE_GRAPHICS_API_VULKAN)
    RuntimeInterface::GetInstanceProcAddr(instance, "xrGetVulkanGraphicsDeviceKHR", reinterpret_cast<PFN_xrVoidFunction*>(&table->GetVulkanGraphicsDeviceKHR));
#endif // defined(XR_USE_GRAPHICS_API_VULKAN)
#if defined(XR_USE_GRAPHICS_API_VULKAN)
    RuntimeInterface::GetInstanceProcAddr(instance, "xrGetVulkanGraphicsRequirementsKHR", reinterpret_cast<PFN_xrVoidFunction*>(&table->GetVulkanGraphicsRequirementsKHR));
#endif // defined(XR_USE_GRAPHICS_API_VULKAN)

    // ---- XR_KHR_D3D10_enable extension commands
#if defined(XR_USE_GRAPHICS_API_D3D10)
    RuntimeInterface::GetInstanceProcAddr(instance, "xrGetD3D10GraphicsRequirementsKHR", reinterpret_cast<PFN_xrVoidFunction*>(&table->GetD3D10GraphicsRequirementsKHR));
#endif // defined(XR_USE_GRAPHICS_API_D3D10)

    // ---- XR_KHR_D3D11_enable extension commands
#if defined(XR_USE_GRAPHICS_API_D3D11)
    RuntimeInterface::GetInstanceProcAddr(instance, "xrGetD3D11GraphicsRequirementsKHR", reinterpret_cast<PFN_xrVoidFunction*>(&table->GetD3D11GraphicsRequirementsKHR));
#endif // defined(XR_USE_GRAPHICS_API_D3D11)

    // ---- XR_KHR_D3D12_enable extension commands
#if defined(XR_USE_GRAPHICS_API_D3D12)
    RuntimeInterface::GetInstanceProcAddr(instance, "xrGetD3D12GraphicsRequirementsKHR", reinterpret_cast<PFN_xrVoidFunction*>(&table->GetD3D12GraphicsRequirementsKHR));
#endif // defined(XR_USE_GRAPHICS_API_D3D12)

    // ---- XR_KHR_visibility_mask extension commands
    RuntimeInterface::GetInstanceProcAddr(instance, "xrGetVisibilityMaskKHR", reinterpret_cast<PFN_xrVoidFunction*>(&table->GetVisibilityMaskKHR));

    // ---- XR_KHR_win32_convert_performance_counter_time extension commands
#if defined(XR_USE_PLATFORM_WIN32)
    RuntimeInterface::GetInstanceProcAddr(instance, "xrConvertWin32PerformanceCounterToTimeKHR", reinterpret_cast<PFN_xrVoidFunction*>(&table->ConvertWin32PerformanceCounterToTimeKHR));
#endif // defined(XR_USE_PLATFORM_WIN32)
#if defined(XR_USE_PLATFORM_WIN32)
    RuntimeInterface::GetInstanceProcAddr(instance, "xrConvertTimeToWin32PerformanceCounterKHR", reinterpret_cast<PFN_xrVoidFunction*>(&table->ConvertTimeToWin32PerformanceCounterKHR));
#endif // defined(XR_USE_PLATFORM_WIN32)

    // ---- XR_KHR_convert_timespec_time extension commands
#if defined(XR_USE_TIMESPEC)
    RuntimeInterface::GetInstanceProcAddr(instance, "xrConvertTimespecTimeToTimeKHR", reinterpret_cast<PFN_xrVoidFunction*>(&table->ConvertTimespecTimeToTimeKHR));
#endif // defined(XR_USE_TIMESPEC)
#if defined(XR_USE_TIMESPEC)
    RuntimeInterface::GetInstanceProcAddr(instance, "xrConvertTimeToTimespecTimeKHR", reinterpret_cast<PFN_xrVoidFunction*>(&table->ConvertTimeToTimespecTimeKHR));
#endif // defined(XR_USE_TIMESPEC)

    // ---- XR_EXT_debug_utils extension commands
    table->SetDebugUtilsObjectNameEXT = LoaderXrTermSetDebugUtilsObjectNameEXT;
    table->CreateDebugUtilsMessengerEXT = LoaderXrTermCreateDebugUtilsMessengerEXT;
    table->DestroyDebugUtilsMessengerEXT = LoaderXrTermDestroyDebugUtilsMessengerEXT;
    table->SubmitDebugUtilsMessageEXT = LoaderXrTermSubmitDebugUtilsMessageEXT;
    RuntimeInterface::GetInstanceProcAddr(instance, "xrSessionBeginDebugUtilsLabelRegionEXT", reinterpret_cast<PFN_xrVoidFunction*>(&table->SessionBeginDebugUtilsLabelRegionEXT));
    RuntimeInterface::GetInstanceProcAddr(instance, "xrSessionEndDebugUtilsLabelRegionEXT", reinterpret_cast<PFN_xrVoidFunction*>(&table->SessionEndDebugUtilsLabelRegionEXT));
    RuntimeInterface::GetInstanceProcAddr(instance, "xrSessionInsertDebugUtilsLabelEXT", reinterpret_cast<PFN_xrVoidFunction*>(&table->SessionInsertDebugUtilsLabelEXT));

    // ---- XR_MSFT_secondary_view_configuration extension commands
    RuntimeInterface::GetInstanceProcAddr(instance, "xrSubmitFrameCompositionLayersMSFT", reinterpret_cast<PFN_xrVoidFunction*>(&table->SubmitFrameCompositionLayersMSFT));

    // ---- XR_MSFT_spatial_perception_bridge extension commands
#if defined(XR_USE_PLATFORM_WIN32)
    RuntimeInterface::GetInstanceProcAddr(instance, "xrCreateSpaceFromSpatialCoordinateSystemMSFT", reinterpret_cast<PFN_xrVoidFunction*>(&table->CreateSpaceFromSpatialCoordinateSystemMSFT));
#endif // defined(XR_USE_PLATFORM_WIN32)

    // ---- XR_MSFT_controller_render_model extension commands
    RuntimeInterface::GetInstanceProcAddr(instance, "xrLoadControllerRenderModelMSFT", reinterpret_cast<PFN_xrVoidFunction*>(&table->LoadControllerRenderModelMSFT));

    // ---- XR_MSFT_spatial_anchor extension commands
    RuntimeInterface::GetInstanceProcAddr(instance, "xrCreateSpatialAnchorSpaceMSFT", reinterpret_cast<PFN_xrVoidFunction*>(&table->CreateSpatialAnchorSpaceMSFT));
    RuntimeInterface::GetInstanceProcAddr(instance, "xrCreateSpatialAnchorMSFT", reinterpret_cast<PFN_xrVoidFunction*>(&table->CreateSpatialAnchorMSFT));
    RuntimeInterface::GetInstanceProcAddr(instance, "xrDestroySpatialAnchorMSFT", reinterpret_cast<PFN_xrVoidFunction*>(&table->DestroySpatialAnchorMSFT));

    // ---- XR_MSFT_spatial_anchor_storage extension commands
    RuntimeInterface::GetInstanceProcAddr(instance, "xrStoreSpatialAnchorMSFT", reinterpret_cast<PFN_xrVoidFunction*>(&table->StoreSpatialAnchorMSFT));
    RuntimeInterface::GetInstanceProcAddr(instance, "xrEnumerateStoredAnchorsMSFT", reinterpret_cast<PFN_xrVoidFunction*>(&table->EnumerateStoredAnchorsMSFT));
    RuntimeInterface::GetInstanceProcAddr(instance, "xrCreateSpatialAnchorFromStoredAnchorNameMSFT", reinterpret_cast<PFN_xrVoidFunction*>(&table->CreateSpatialAnchorFromStoredAnchorNameMSFT));
    RuntimeInterface::GetInstanceProcAddr(instance, "xrDeleteStoredAnchorMSFT", reinterpret_cast<PFN_xrVoidFunction*>(&table->DeleteStoredAnchorMSFT));
}

// Instance Update Dispatch Table with an API Layer Interface
void ApiLayerInterface::GenUpdateInstanceDispatchTable(XrInstance instance, std::unique_ptr<XrGeneratedDispatchTable>& table) {
    PFN_xrVoidFunction cur_func_ptr;

    // ---- Core 0_90 commands
    table->GetInstanceProcAddr = _get_instant_proc_addr;
    _get_instant_proc_addr(instance, "xrCreateInstance", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->CreateInstance = reinterpret_cast<PFN_xrCreateInstance>(cur_func_ptr);
    }
    _get_instant_proc_addr(instance, "xrDestroyInstance", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->DestroyInstance = reinterpret_cast<PFN_xrDestroyInstance>(cur_func_ptr);
    }
    _get_instant_proc_addr(instance, "xrGetInstanceProperties", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->GetInstanceProperties = reinterpret_cast<PFN_xrGetInstanceProperties>(cur_func_ptr);
    }
    _get_instant_proc_addr(instance, "xrPollEvent", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->PollEvent = reinterpret_cast<PFN_xrPollEvent>(cur_func_ptr);
    }
    _get_instant_proc_addr(instance, "xrResultToString", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->ResultToString = reinterpret_cast<PFN_xrResultToString>(cur_func_ptr);
    }
    _get_instant_proc_addr(instance, "xrStructureTypeToString", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->StructureTypeToString = reinterpret_cast<PFN_xrStructureTypeToString>(cur_func_ptr);
    }
    _get_instant_proc_addr(instance, "xrGetSystem", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->GetSystem = reinterpret_cast<PFN_xrGetSystem>(cur_func_ptr);
    }
    _get_instant_proc_addr(instance, "xrGetSystemProperties", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->GetSystemProperties = reinterpret_cast<PFN_xrGetSystemProperties>(cur_func_ptr);
    }
    _get_instant_proc_addr(instance, "xrEnumerateEnvironmentBlendModes", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->EnumerateEnvironmentBlendModes = reinterpret_cast<PFN_xrEnumerateEnvironmentBlendModes>(cur_func_ptr);
    }
    _get_instant_proc_addr(instance, "xrCreateSession", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->CreateSession = reinterpret_cast<PFN_xrCreateSession>(cur_func_ptr);
    }
    _get_instant_proc_addr(instance, "xrDestroySession", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->DestroySession = reinterpret_cast<PFN_xrDestroySession>(cur_func_ptr);
    }
    _get_instant_proc_addr(instance, "xrEnumerateReferenceSpaces", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->EnumerateReferenceSpaces = reinterpret_cast<PFN_xrEnumerateReferenceSpaces>(cur_func_ptr);
    }
    _get_instant_proc_addr(instance, "xrCreateReferenceSpace", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->CreateReferenceSpace = reinterpret_cast<PFN_xrCreateReferenceSpace>(cur_func_ptr);
    }
    _get_instant_proc_addr(instance, "xrGetReferenceSpaceBoundsRect", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->GetReferenceSpaceBoundsRect = reinterpret_cast<PFN_xrGetReferenceSpaceBoundsRect>(cur_func_ptr);
    }
    _get_instant_proc_addr(instance, "xrCreateActionSpace", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->CreateActionSpace = reinterpret_cast<PFN_xrCreateActionSpace>(cur_func_ptr);
    }
    _get_instant_proc_addr(instance, "xrLocateSpace", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->LocateSpace = reinterpret_cast<PFN_xrLocateSpace>(cur_func_ptr);
    }
    _get_instant_proc_addr(instance, "xrDestroySpace", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->DestroySpace = reinterpret_cast<PFN_xrDestroySpace>(cur_func_ptr);
    }
    _get_instant_proc_addr(instance, "xrEnumerateViewConfigurations", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->EnumerateViewConfigurations = reinterpret_cast<PFN_xrEnumerateViewConfigurations>(cur_func_ptr);
    }
    _get_instant_proc_addr(instance, "xrGetViewConfigurationProperties", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->GetViewConfigurationProperties = reinterpret_cast<PFN_xrGetViewConfigurationProperties>(cur_func_ptr);
    }
    _get_instant_proc_addr(instance, "xrEnumerateViewConfigurationViews", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->EnumerateViewConfigurationViews = reinterpret_cast<PFN_xrEnumerateViewConfigurationViews>(cur_func_ptr);
    }
    _get_instant_proc_addr(instance, "xrEnumerateSwapchainFormats", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->EnumerateSwapchainFormats = reinterpret_cast<PFN_xrEnumerateSwapchainFormats>(cur_func_ptr);
    }
    _get_instant_proc_addr(instance, "xrCreateSwapchain", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->CreateSwapchain = reinterpret_cast<PFN_xrCreateSwapchain>(cur_func_ptr);
    }
    _get_instant_proc_addr(instance, "xrDestroySwapchain", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->DestroySwapchain = reinterpret_cast<PFN_xrDestroySwapchain>(cur_func_ptr);
    }
    _get_instant_proc_addr(instance, "xrEnumerateSwapchainImages", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->EnumerateSwapchainImages = reinterpret_cast<PFN_xrEnumerateSwapchainImages>(cur_func_ptr);
    }
    _get_instant_proc_addr(instance, "xrAcquireSwapchainImage", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->AcquireSwapchainImage = reinterpret_cast<PFN_xrAcquireSwapchainImage>(cur_func_ptr);
    }
    _get_instant_proc_addr(instance, "xrWaitSwapchainImage", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->WaitSwapchainImage = reinterpret_cast<PFN_xrWaitSwapchainImage>(cur_func_ptr);
    }
    _get_instant_proc_addr(instance, "xrReleaseSwapchainImage", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->ReleaseSwapchainImage = reinterpret_cast<PFN_xrReleaseSwapchainImage>(cur_func_ptr);
    }
    _get_instant_proc_addr(instance, "xrBeginSession", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->BeginSession = reinterpret_cast<PFN_xrBeginSession>(cur_func_ptr);
    }
    _get_instant_proc_addr(instance, "xrEndSession", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->EndSession = reinterpret_cast<PFN_xrEndSession>(cur_func_ptr);
    }
    _get_instant_proc_addr(instance, "xrWaitFrame", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->WaitFrame = reinterpret_cast<PFN_xrWaitFrame>(cur_func_ptr);
    }
    _get_instant_proc_addr(instance, "xrBeginFrame", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->BeginFrame = reinterpret_cast<PFN_xrBeginFrame>(cur_func_ptr);
    }
    _get_instant_proc_addr(instance, "xrEndFrame", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->EndFrame = reinterpret_cast<PFN_xrEndFrame>(cur_func_ptr);
    }
    _get_instant_proc_addr(instance, "xrLocateViews", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->LocateViews = reinterpret_cast<PFN_xrLocateViews>(cur_func_ptr);
    }
    _get_instant_proc_addr(instance, "xrStringToPath", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->StringToPath = reinterpret_cast<PFN_xrStringToPath>(cur_func_ptr);
    }
    _get_instant_proc_addr(instance, "xrPathToString", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->PathToString = reinterpret_cast<PFN_xrPathToString>(cur_func_ptr);
    }
    _get_instant_proc_addr(instance, "xrCreateActionSet", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->CreateActionSet = reinterpret_cast<PFN_xrCreateActionSet>(cur_func_ptr);
    }
    _get_instant_proc_addr(instance, "xrDestroyActionSet", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->DestroyActionSet = reinterpret_cast<PFN_xrDestroyActionSet>(cur_func_ptr);
    }
    _get_instant_proc_addr(instance, "xrCreateAction", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->CreateAction = reinterpret_cast<PFN_xrCreateAction>(cur_func_ptr);
    }
    _get_instant_proc_addr(instance, "xrDestroyAction", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->DestroyAction = reinterpret_cast<PFN_xrDestroyAction>(cur_func_ptr);
    }
    _get_instant_proc_addr(instance, "xrSetInteractionProfileSuggestedBindings", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->SetInteractionProfileSuggestedBindings = reinterpret_cast<PFN_xrSetInteractionProfileSuggestedBindings>(cur_func_ptr);
    }
    _get_instant_proc_addr(instance, "xrGetCurrentInteractionProfile", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->GetCurrentInteractionProfile = reinterpret_cast<PFN_xrGetCurrentInteractionProfile>(cur_func_ptr);
    }
    _get_instant_proc_addr(instance, "xrGetActionStateBoolean", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->GetActionStateBoolean = reinterpret_cast<PFN_xrGetActionStateBoolean>(cur_func_ptr);
    }
    _get_instant_proc_addr(instance, "xrGetActionStateVector1f", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->GetActionStateVector1f = reinterpret_cast<PFN_xrGetActionStateVector1f>(cur_func_ptr);
    }
    _get_instant_proc_addr(instance, "xrGetActionStateVector2f", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->GetActionStateVector2f = reinterpret_cast<PFN_xrGetActionStateVector2f>(cur_func_ptr);
    }
    _get_instant_proc_addr(instance, "xrGetActionStatePose", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->GetActionStatePose = reinterpret_cast<PFN_xrGetActionStatePose>(cur_func_ptr);
    }
    _get_instant_proc_addr(instance, "xrSyncActionData", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->SyncActionData = reinterpret_cast<PFN_xrSyncActionData>(cur_func_ptr);
    }
    _get_instant_proc_addr(instance, "xrGetBoundSourcesForAction", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->GetBoundSourcesForAction = reinterpret_cast<PFN_xrGetBoundSourcesForAction>(cur_func_ptr);
    }
    _get_instant_proc_addr(instance, "xrGetInputSourceLocalizedName", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->GetInputSourceLocalizedName = reinterpret_cast<PFN_xrGetInputSourceLocalizedName>(cur_func_ptr);
    }
    _get_instant_proc_addr(instance, "xrApplyHapticFeedback", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->ApplyHapticFeedback = reinterpret_cast<PFN_xrApplyHapticFeedback>(cur_func_ptr);
    }
    _get_instant_proc_addr(instance, "xrStopHapticFeedback", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->StopHapticFeedback = reinterpret_cast<PFN_xrStopHapticFeedback>(cur_func_ptr);
    }

    // ---- XR_KHR_android_thread_settings extension commands
#if defined(XR_USE_PLATFORM_ANDROID)
    _get_instant_proc_addr(instance, "xrSetAndroidApplicationThreadKHR", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->SetAndroidApplicationThreadKHR = reinterpret_cast<PFN_xrSetAndroidApplicationThreadKHR>(cur_func_ptr);
    }
#endif // defined(XR_USE_PLATFORM_ANDROID)

    // ---- XR_KHR_android_surface_swapchain extension commands
#if defined(XR_USE_PLATFORM_ANDROID)
    _get_instant_proc_addr(instance, "xrCreateSwapchainAndroidSurfaceKHR", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->CreateSwapchainAndroidSurfaceKHR = reinterpret_cast<PFN_xrCreateSwapchainAndroidSurfaceKHR>(cur_func_ptr);
    }
#endif // defined(XR_USE_PLATFORM_ANDROID)

    // ---- XR_KHR_opengl_enable extension commands
#if defined(XR_USE_GRAPHICS_API_OPENGL)
    _get_instant_proc_addr(instance, "xrGetOpenGLGraphicsRequirementsKHR", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->GetOpenGLGraphicsRequirementsKHR = reinterpret_cast<PFN_xrGetOpenGLGraphicsRequirementsKHR>(cur_func_ptr);
    }
#endif // defined(XR_USE_GRAPHICS_API_OPENGL)

    // ---- XR_KHR_opengl_es_enable extension commands
#if defined(XR_USE_GRAPHICS_API_OPENGL_ES)
    _get_instant_proc_addr(instance, "xrGetOpenGLESGraphicsRequirementsKHR", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->GetOpenGLESGraphicsRequirementsKHR = reinterpret_cast<PFN_xrGetOpenGLESGraphicsRequirementsKHR>(cur_func_ptr);
    }
#endif // defined(XR_USE_GRAPHICS_API_OPENGL_ES)

    // ---- XR_KHR_vulkan_enable extension commands
#if defined(XR_USE_GRAPHICS_API_VULKAN)
    _get_instant_proc_addr(instance, "xrGetVulkanInstanceExtensionsKHR", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->GetVulkanInstanceExtensionsKHR = reinterpret_cast<PFN_xrGetVulkanInstanceExtensionsKHR>(cur_func_ptr);
    }
#endif // defined(XR_USE_GRAPHICS_API_VULKAN)
#if defined(XR_USE_GRAPHICS_API_VULKAN)
    _get_instant_proc_addr(instance, "xrGetVulkanDeviceExtensionsKHR", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->GetVulkanDeviceExtensionsKHR = reinterpret_cast<PFN_xrGetVulkanDeviceExtensionsKHR>(cur_func_ptr);
    }
#endif // defined(XR_USE_GRAPHICS_API_VULKAN)
#if defined(XR_USE_GRAPHICS_API_VULKAN)
    _get_instant_proc_addr(instance, "xrGetVulkanGraphicsDeviceKHR", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->GetVulkanGraphicsDeviceKHR = reinterpret_cast<PFN_xrGetVulkanGraphicsDeviceKHR>(cur_func_ptr);
    }
#endif // defined(XR_USE_GRAPHICS_API_VULKAN)
#if defined(XR_USE_GRAPHICS_API_VULKAN)
    _get_instant_proc_addr(instance, "xrGetVulkanGraphicsRequirementsKHR", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->GetVulkanGraphicsRequirementsKHR = reinterpret_cast<PFN_xrGetVulkanGraphicsRequirementsKHR>(cur_func_ptr);
    }
#endif // defined(XR_USE_GRAPHICS_API_VULKAN)

    // ---- XR_KHR_D3D10_enable extension commands
#if defined(XR_USE_GRAPHICS_API_D3D10)
    _get_instant_proc_addr(instance, "xrGetD3D10GraphicsRequirementsKHR", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->GetD3D10GraphicsRequirementsKHR = reinterpret_cast<PFN_xrGetD3D10GraphicsRequirementsKHR>(cur_func_ptr);
    }
#endif // defined(XR_USE_GRAPHICS_API_D3D10)

    // ---- XR_KHR_D3D11_enable extension commands
#if defined(XR_USE_GRAPHICS_API_D3D11)
    _get_instant_proc_addr(instance, "xrGetD3D11GraphicsRequirementsKHR", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->GetD3D11GraphicsRequirementsKHR = reinterpret_cast<PFN_xrGetD3D11GraphicsRequirementsKHR>(cur_func_ptr);
    }
#endif // defined(XR_USE_GRAPHICS_API_D3D11)

    // ---- XR_KHR_D3D12_enable extension commands
#if defined(XR_USE_GRAPHICS_API_D3D12)
    _get_instant_proc_addr(instance, "xrGetD3D12GraphicsRequirementsKHR", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->GetD3D12GraphicsRequirementsKHR = reinterpret_cast<PFN_xrGetD3D12GraphicsRequirementsKHR>(cur_func_ptr);
    }
#endif // defined(XR_USE_GRAPHICS_API_D3D12)

    // ---- XR_KHR_visibility_mask extension commands
    _get_instant_proc_addr(instance, "xrGetVisibilityMaskKHR", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->GetVisibilityMaskKHR = reinterpret_cast<PFN_xrGetVisibilityMaskKHR>(cur_func_ptr);
    }

    // ---- XR_KHR_win32_convert_performance_counter_time extension commands
#if defined(XR_USE_PLATFORM_WIN32)
    _get_instant_proc_addr(instance, "xrConvertWin32PerformanceCounterToTimeKHR", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->ConvertWin32PerformanceCounterToTimeKHR = reinterpret_cast<PFN_xrConvertWin32PerformanceCounterToTimeKHR>(cur_func_ptr);
    }
#endif // defined(XR_USE_PLATFORM_WIN32)
#if defined(XR_USE_PLATFORM_WIN32)
    _get_instant_proc_addr(instance, "xrConvertTimeToWin32PerformanceCounterKHR", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->ConvertTimeToWin32PerformanceCounterKHR = reinterpret_cast<PFN_xrConvertTimeToWin32PerformanceCounterKHR>(cur_func_ptr);
    }
#endif // defined(XR_USE_PLATFORM_WIN32)

    // ---- XR_KHR_convert_timespec_time extension commands
#if defined(XR_USE_TIMESPEC)
    _get_instant_proc_addr(instance, "xrConvertTimespecTimeToTimeKHR", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->ConvertTimespecTimeToTimeKHR = reinterpret_cast<PFN_xrConvertTimespecTimeToTimeKHR>(cur_func_ptr);
    }
#endif // defined(XR_USE_TIMESPEC)
#if defined(XR_USE_TIMESPEC)
    _get_instant_proc_addr(instance, "xrConvertTimeToTimespecTimeKHR", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->ConvertTimeToTimespecTimeKHR = reinterpret_cast<PFN_xrConvertTimeToTimespecTimeKHR>(cur_func_ptr);
    }
#endif // defined(XR_USE_TIMESPEC)

    // ---- XR_EXT_debug_utils extension commands
    _get_instant_proc_addr(instance, "xrSetDebugUtilsObjectNameEXT", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->SetDebugUtilsObjectNameEXT = reinterpret_cast<PFN_xrSetDebugUtilsObjectNameEXT>(cur_func_ptr);
    }
    _get_instant_proc_addr(instance, "xrCreateDebugUtilsMessengerEXT", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->CreateDebugUtilsMessengerEXT = reinterpret_cast<PFN_xrCreateDebugUtilsMessengerEXT>(cur_func_ptr);
    }
    _get_instant_proc_addr(instance, "xrDestroyDebugUtilsMessengerEXT", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->DestroyDebugUtilsMessengerEXT = reinterpret_cast<PFN_xrDestroyDebugUtilsMessengerEXT>(cur_func_ptr);
    }
    _get_instant_proc_addr(instance, "xrSubmitDebugUtilsMessageEXT", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->SubmitDebugUtilsMessageEXT = reinterpret_cast<PFN_xrSubmitDebugUtilsMessageEXT>(cur_func_ptr);
    }
    _get_instant_proc_addr(instance, "xrSessionBeginDebugUtilsLabelRegionEXT", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->SessionBeginDebugUtilsLabelRegionEXT = reinterpret_cast<PFN_xrSessionBeginDebugUtilsLabelRegionEXT>(cur_func_ptr);
    }
    _get_instant_proc_addr(instance, "xrSessionEndDebugUtilsLabelRegionEXT", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->SessionEndDebugUtilsLabelRegionEXT = reinterpret_cast<PFN_xrSessionEndDebugUtilsLabelRegionEXT>(cur_func_ptr);
    }
    _get_instant_proc_addr(instance, "xrSessionInsertDebugUtilsLabelEXT", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->SessionInsertDebugUtilsLabelEXT = reinterpret_cast<PFN_xrSessionInsertDebugUtilsLabelEXT>(cur_func_ptr);
    }

    // ---- XR_MSFT_secondary_view_configuration extension commands
    _get_instant_proc_addr(instance, "xrSubmitFrameCompositionLayersMSFT", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->SubmitFrameCompositionLayersMSFT = reinterpret_cast<PFN_xrSubmitFrameCompositionLayersMSFT>(cur_func_ptr);
    }

    // ---- XR_MSFT_spatial_perception_bridge extension commands
#if defined(XR_USE_PLATFORM_WIN32)
    _get_instant_proc_addr(instance, "xrCreateSpaceFromSpatialCoordinateSystemMSFT", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->CreateSpaceFromSpatialCoordinateSystemMSFT = reinterpret_cast<PFN_xrCreateSpaceFromSpatialCoordinateSystemMSFT>(cur_func_ptr);
    }
#endif // defined(XR_USE_PLATFORM_WIN32)

    // ---- XR_MSFT_controller_render_model extension commands
    _get_instant_proc_addr(instance, "xrLoadControllerRenderModelMSFT", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->LoadControllerRenderModelMSFT = reinterpret_cast<PFN_xrLoadControllerRenderModelMSFT>(cur_func_ptr);
    }

    // ---- XR_MSFT_spatial_anchor extension commands
    _get_instant_proc_addr(instance, "xrCreateSpatialAnchorSpaceMSFT", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->CreateSpatialAnchorSpaceMSFT = reinterpret_cast<PFN_xrCreateSpatialAnchorSpaceMSFT>(cur_func_ptr);
    }
    _get_instant_proc_addr(instance, "xrCreateSpatialAnchorMSFT", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->CreateSpatialAnchorMSFT = reinterpret_cast<PFN_xrCreateSpatialAnchorMSFT>(cur_func_ptr);
    }
    _get_instant_proc_addr(instance, "xrDestroySpatialAnchorMSFT", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->DestroySpatialAnchorMSFT = reinterpret_cast<PFN_xrDestroySpatialAnchorMSFT>(cur_func_ptr);
    }

    // ---- XR_MSFT_spatial_anchor_storage extension commands
    _get_instant_proc_addr(instance, "xrStoreSpatialAnchorMSFT", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->StoreSpatialAnchorMSFT = reinterpret_cast<PFN_xrStoreSpatialAnchorMSFT>(cur_func_ptr);
    }
    _get_instant_proc_addr(instance, "xrEnumerateStoredAnchorsMSFT", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->EnumerateStoredAnchorsMSFT = reinterpret_cast<PFN_xrEnumerateStoredAnchorsMSFT>(cur_func_ptr);
    }
    _get_instant_proc_addr(instance, "xrCreateSpatialAnchorFromStoredAnchorNameMSFT", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->CreateSpatialAnchorFromStoredAnchorNameMSFT = reinterpret_cast<PFN_xrCreateSpatialAnchorFromStoredAnchorNameMSFT>(cur_func_ptr);
    }
    _get_instant_proc_addr(instance, "xrDeleteStoredAnchorMSFT", &cur_func_ptr);
    if (nullptr != cur_func_ptr) {
        table->DeleteStoredAnchorMSFT = reinterpret_cast<PFN_xrDeleteStoredAnchorMSFT>(cur_func_ptr);
    }
}
#ifdef __cplusplus
} // extern "C"
#endif

