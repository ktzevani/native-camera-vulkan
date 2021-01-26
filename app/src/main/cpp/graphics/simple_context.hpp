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

#ifndef NCV_GRAPHICS_SIMPLE_CONTEXT_HPP
#define NCV_GRAPHICS_SIMPLE_CONTEXT_HPP

#include <graphics/vulkan_context.hpp>
#include <any>

class android_app;

namespace graphics
{
    class simple_context : public vulkan_context
    {
    public:

        explicit simple_context(const std::string& a_app_name);
        ~simple_context();
        void initialize_graphics(android_app* a_app);
        void render_frame(const std::any& a_params);

    private:

#ifdef NCV_VULKAN_VALIDATION_ENABLED
        const bool m_enable_validation = true;
#else
        const bool m_enable_validation = false;
#endif

        std::vector<vk::PhysicalDevice> m_devices;

        vk::PhysicalDevice m_gpu;
        vk::PhysicalDeviceProperties m_gpu_props;
        vk::PhysicalDeviceFeatures m_gpu_features;
        vk::PhysicalDeviceMemoryProperties m_gpu_mem_props;

        ::vk_util::version m_api_version;
        ::vk_util::version m_driver_version;

        vk::UniqueInstance m_instance;
        vk::UniqueSurfaceKHR m_surface;
        vk::UniqueDevice m_device;
        vk::UniqueSemaphore m_semaphore;
        vk::UniqueSemaphore m_render_semaphore;
        vk::UniqueSwapchainKHR m_swap_chain;
        vk::UniqueCommandPool m_cmd_pool;

        std::vector<vk::CommandBuffer> m_cmd_buffers;
        std::vector<vk::Image> m_images;

        vk::Queue m_pres_queue;
        int32_t m_selected_queue_family_index = -1;

        vk::DebugUtilsMessengerEXT m_debug_msg;

        android_app* m_app = nullptr;
    };
}

#endif //NCV_GRAPHICS_SIMPLE_CONTEXT_HPP
