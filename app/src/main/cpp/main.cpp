/*
 * Copyright 2020 Konstantinos Tzevanidis
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <metadata/version.hpp>
#include <core/android_permissions.hpp>
#include <core/event_loop.hpp>
#include <graphics/simple_context.hpp>
#include <graphics/complex_context.hpp>
#include <engine/vulkan.hpp>
#include <utilities/log.hpp>

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

#ifdef NCV_USE_VULKAN_SIMPLE
typedef ::engine::vulkan<::graphics::simple_context> android_engine;
#elif NCV_USE_VULKAN_COMPLETE
typedef ::engine::vulkan<::graphics::complex_context> android_engine;
#endif

constexpr const char* PACKAGE_NAME = "com.ktzevani.nativecameravulkan";
constexpr const char* PROJECT_NAME = "native-camera-vulkan";

::utilities::log_level __ncv_log_level = ::utilities::log_level::all;

void android_main(android_app* a_Application)
{
    using ::metadata::PROJECT_VERSION;
    using ::utilities::log_level;

    if constexpr(std::is_same<android_engine, ::engine::vulkan<::graphics::complex_context>>())
    {
        ::core::android_permissions permissions;
        permissions.request_camera_permission(a_Application);
    }

    if constexpr(__ncv_logging_enabled)
        _log_android(log_level::info) << "Version is: " << PROJECT_VERSION.full_version();

    auto engine = std::make_shared<android_engine>(a_Application, std::string(PACKAGE_NAME),
        std::string(PROJECT_NAME), PROJECT_VERSION.packed_version());

    ::core::event_loop<android_engine>{a_Application, engine}.run();
}