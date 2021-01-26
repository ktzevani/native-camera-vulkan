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

#include <graphics/simple_context.hpp>

#include <android_native_app_glue.h>
#include <metadata/version.hpp>
#include <glm/glm.hpp>

using namespace ::std;
using namespace ::vk;
using namespace ::glm;
using namespace ::utilities;
using namespace ::vk_util;
using namespace ::metadata;

namespace graphics
{
    simple_context::simple_context(const string &a_app_name) : vulkan_context{}
    {
        // Create an instance and query for physical devices

        vector<const char*> requested_gl_extensions = {
            VK_KHR_SURFACE_EXTENSION_NAME,
            VK_KHR_ANDROID_SURFACE_EXTENSION_NAME
        };

        if(m_enable_validation)
            requested_gl_extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

        vector<const char*> requested_gl_layers = {
            "VK_LAYER_KHRONOS_validation"
        };

        ApplicationInfo app_info;

        app_info.pApplicationName = a_app_name.c_str();
        app_info.applicationVersion = PROJECT_VERSION.packed_version();
        app_info.pEngineName = "AndroidVulkanEngine";
        app_info.engineVersion = PROJECT_VERSION.packed_version();
        app_info.apiVersion = VK_API_VERSION_1_1;

        InstanceCreateInfo instance_info;

        instance_info.flags = static_cast<InstanceCreateFlags>(0);
        instance_info.pApplicationInfo = &app_info;
        if(m_enable_validation)
        {
            instance_info.enabledLayerCount = requested_gl_layers.size();
            instance_info.ppEnabledLayerNames = requested_gl_layers.data();
        }
        else
        {
            instance_info.enabledLayerCount = 0;
            instance_info.ppEnabledLayerNames = nullptr;
        }
        instance_info.enabledExtensionCount = requested_gl_extensions.size();
        instance_info.ppEnabledExtensionNames = requested_gl_extensions.data();

        m_instance = createInstanceUnique(instance_info);

        VULKAN_HPP_DEFAULT_DISPATCHER.init(m_instance.get());

        if(m_enable_validation)
        {
            DebugUtilsMessengerCreateInfoEXT msg_info;

            msg_info.messageSeverity = DebugUtilsMessageSeverityFlagBitsEXT::eError |
                                       DebugUtilsMessageSeverityFlagBitsEXT::eInfo |
                                       DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
                                       DebugUtilsMessageSeverityFlagBitsEXT::eVerbose;

            msg_info.messageType = DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
                                   DebugUtilsMessageTypeFlagBitsEXT::ePerformance |
                                   DebugUtilsMessageTypeFlagBitsEXT::eValidation;

            msg_info.pfnUserCallback = reinterpret_cast<PFN_vkDebugUtilsMessengerCallbackEXT>(message_callback);
            m_debug_msg = m_instance->createDebugUtilsMessengerEXT(msg_info);
        }

        m_devices = m_instance->enumeratePhysicalDevices();

        if constexpr(__ncv_logging_enabled)
            _log_android(log_level::debug)
                << "Vulkan Context Created.\n# of Vulkan Devices: " << m_devices.size();
    }

    simple_context::~simple_context()
    {
        if(m_cmd_buffers.size() > 0)
        {
            m_device->waitIdle();
            m_device->freeCommandBuffers(m_cmd_pool.get(), m_cmd_buffers);
            m_cmd_buffers.clear();
            m_device->destroyCommandPool(m_cmd_pool.release());
            m_device->destroySwapchainKHR(m_swap_chain.release());
            m_instance->destroySurfaceKHR(m_surface.release());
            m_device->destroySemaphore(m_render_semaphore.release());
            m_device->destroySemaphore(m_semaphore.release());
            if(m_enable_validation && !!m_debug_msg)
                m_instance->destroyDebugUtilsMessengerEXT(m_debug_msg);
            m_device->waitIdle();
            m_device.release().destroy();
            m_instance.release().destroy();
        }
    }

    void simple_context::initialize_graphics(android_app *a_app)
    {
        m_app = a_app;

        if(m_cmd_buffers.size() > 0)
        {
            m_device->destroySwapchainKHR(m_swap_chain.release());
            m_instance->destroySurfaceKHR(m_surface.release());
            m_device->waitIdle();
            m_device->freeCommandBuffers(m_cmd_pool.get(), m_cmd_buffers);
            m_cmd_buffers.clear();
            m_device->destroyCommandPool(m_cmd_pool.release());
        }

        // Create a presentation surface

        SurfaceCapabilitiesKHR surf_caps;
        AndroidSurfaceCreateInfoKHR surf_info;

        surf_info.flags = static_cast<AndroidSurfaceCreateFlagsKHR>(0);
        surf_info.window = a_app->window;

        m_surface.reset(m_instance->createAndroidSurfaceKHR(surf_info));

        if(m_selected_queue_family_index < 0)
        {
            // List device information and select an appropriate device based on queue family
            // support and presentation capabilities

            if constexpr(__ncv_logging_enabled)
                _log_android(log_level::debug) << "**** Devices Information ****";

            int32_t selected_device_index = -1;

            for (uint32_t i = 0; i < m_devices.size(); ++i)
            {
                // List supported extensions

                auto dev_exts = m_devices[i].enumerateDeviceExtensionProperties();

                if constexpr(__ncv_logging_enabled)
                {
                    _log_android(log_level::debug) << "\tDevice Index: " << i;

                    for (auto &extension : dev_exts)
                        _log_android(log_level::debug) << "\t\t" << extension.extensionName << " ("
                           << version(extension.specVersion).to_string() << ")";
                }

                // List supported queue families and select a family that supports graphics and
                // compute

                if constexpr(__ncv_logging_enabled)
                    _log_android(log_level::debug) << "Queues:\n";

                vector<QueueFamilyProperties> queue_props = m_devices[i].getQueueFamilyProperties();

                for (size_t j = 0; j < queue_props.size(); ++j)
                {
                    const auto &queue_family_props = queue_props[j];
                    bool presentation_flag = m_devices[i].getSurfaceSupportKHR(j, m_surface.get());

                    if constexpr(__ncv_logging_enabled)
                    {
                        _log_android(log_level::debug) << "\tQueue Family: " << j;
                        _log_android(log_level::debug) << "\t\tQueue Family Flags: "
                            << to_string(queue_family_props.queueFlags);
                        _log_android(log_level::debug) << "\t\tQueue Count: " << queue_family_props.queueCount;
                        _log_android(log_level::debug) << "\t\tSupports Presentation: "
                            << (presentation_flag ? "True" : "False");
                    }

                    if ((queue_family_props.queueFlags & QueueFlagBits::eGraphics) &&
                        (queue_family_props.queueFlags & QueueFlagBits::eCompute) &&
                        presentation_flag && m_selected_queue_family_index < 0) {
                        m_selected_queue_family_index = j;
                    }
                }

                if (m_selected_queue_family_index > -1 && selected_device_index < 0)
                    selected_device_index = i;
            }

            // Set a primary physical device and gather its information

            if constexpr(__ncv_logging_enabled)
            {
                if (selected_device_index == -1)
                    throw runtime_error{"No Appropriate Device Found (One that supports both graphics and compute)."};
                else
                    _log_android(log_level::debug) << "Device " << selected_device_index << "(queue.fam:"
                       << m_selected_queue_family_index << ") is selected as primary.";
            }

            m_gpu = m_devices[selected_device_index];
            m_gpu_props = m_gpu.getProperties();
            m_api_version = m_gpu_props.apiVersion;
            m_driver_version = m_gpu_props.driverVersion;
            m_gpu_features = m_gpu.getFeatures();
            m_gpu_mem_props = m_gpu.getMemoryProperties();

            if constexpr(__ncv_logging_enabled)
            {
                _log_android(log_level::debug) << "**** Primary Device Information ****";
                _log_android(log_level::debug) << "API Version:    " << m_api_version.to_string();
                _log_android(log_level::debug) << "Driver Version: " << m_driver_version.to_string();
                _log_android(log_level::debug) << "Device Name:    " << m_gpu_props.deviceName;
                _log_android(log_level::debug) << "Device Type:    " << to_string(m_gpu_props.deviceType);
                _log_android(log_level::debug) << "Memory Heaps:   " << m_gpu_mem_props.memoryHeapCount;

                for (uint32_t i = 0; i < m_gpu_mem_props.memoryHeapCount; ++i)
                {
                    const auto &heap = m_gpu_mem_props.memoryHeaps[i];
                    _log_android(log_level::debug) << "\tHeap " << i << to_string(heap.flags)
                        << " size " << readable_size(heap.size);
                }
            }

            // Create logical device

            vector<DeviceQueueCreateInfo> queue_infos = {DeviceQueueCreateInfo()};
            vector<float> queue_priorities = {1.0f};

            queue_infos[0].queueFamilyIndex = m_selected_queue_family_index;
            queue_infos[0].queueCount = 1;
            queue_infos[0].pQueuePriorities = queue_priorities.data();
            queue_infos[0].flags = static_cast<DeviceQueueCreateFlags>(0);

            vector<const char *> requested_dev_extensions = {
                VK_KHR_SWAPCHAIN_EXTENSION_NAME
            };

            DeviceCreateInfo dev_info;

            dev_info.flags = static_cast<DeviceCreateFlags>(0);
            dev_info.queueCreateInfoCount = 1;
            dev_info.pQueueCreateInfos = queue_infos.data();
            dev_info.enabledLayerCount = 0;
            dev_info.ppEnabledLayerNames = nullptr;
            dev_info.enabledExtensionCount = requested_dev_extensions.size();
            dev_info.ppEnabledExtensionNames = requested_dev_extensions.data();
            dev_info.pEnabledFeatures = nullptr;

            m_device = m_gpu.createDeviceUnique(dev_info);

            VULKAN_HPP_DEFAULT_DISPATCHER.init(m_device.get());

            if constexpr(__ncv_logging_enabled)
                _log_android(log_level::debug) << "Logical device created with success.";

            // Acquiring Presentation Queue

            m_pres_queue = m_device->getQueue(m_selected_queue_family_index, 0);

            // Creating a semaphore for queue sync

            SemaphoreCreateInfo semaphore_info;

            semaphore_info.flags = static_cast<SemaphoreCreateFlags>(0);

            m_semaphore = m_device->createSemaphoreUnique(semaphore_info);
            m_render_semaphore = m_device->createSemaphoreUnique(semaphore_info);

            if constexpr(__ncv_logging_enabled)
                _log_android(log_level::debug) << "Semaphore created with success.";

            // Acquire surface capabilities and available swap chain property limits

            surf_caps = m_gpu.getSurfaceCapabilitiesKHR(m_surface.get());
            vector<SurfaceFormatKHR> surf_fmts = m_gpu.getSurfaceFormatsKHR(m_surface.get());
            vector<PresentModeKHR> pres_modes = m_gpu.getSurfacePresentModesKHR(m_surface.get());

            if constexpr(__ncv_logging_enabled)
            {
                _log_android(log_level::debug) << "**** Swap chain Properties ****";
                _log_android(log_level::debug) << "\tMin Image Count: " << surf_caps.minImageCount;
                _log_android(log_level::debug) << "\tMax Image Count: " << surf_caps.maxImageCount;
                _log_android(log_level::debug) << "\tMin Image Extend: (" << surf_caps.minImageExtent.width
                    << ", " << surf_caps.minImageExtent.height << ")";
                _log_android(log_level::debug) << "\tMax Image Extend: (" << surf_caps.maxImageExtent.width
                    << ", " << surf_caps.maxImageExtent.height << ")";
                _log_android(log_level::debug) << "\tCurrent Image Extend: (" << surf_caps.currentExtent.width
                    << ", " << surf_caps.currentExtent.height << ")";
                _log_android(log_level::debug) << "\tFormats Supported: ";

                for (auto &format : surf_fmts)
                    _log_android(log_level::debug) << "\t\t" << to_string(format.format) << " - "
                        << to_string(format.colorSpace);

                _log_android(log_level::debug) << "\tSupported Usage: "
                    << to_string(surf_caps.supportedUsageFlags);
                _log_android(log_level::debug) << "\tSupported Pre-transforms: "
                    << to_string(surf_caps.supportedTransforms);
                _log_android(log_level::debug) << "\tCurrent Pre-transform: "
                    << to_string(surf_caps.currentTransform);
                _log_android(log_level::debug) << "\tPresentation Modes Supported: ";

                for (auto &mode : pres_modes)
                    _log_android(log_level::debug) << "\t\t" << to_string(mode);
            }
        }
        else
            surf_caps = m_gpu.getSurfaceCapabilitiesKHR(m_surface.get());

        // Create swap chain

        SwapchainCreateInfoKHR swap_chain_info;

        swap_chain_info.flags = static_cast<SwapchainCreateFlagsKHR>(0);
        swap_chain_info.surface = m_surface.get();
        swap_chain_info.minImageCount = 3;
        swap_chain_info.imageFormat = Format::eR8G8B8A8Unorm;
        swap_chain_info.imageColorSpace = ColorSpaceKHR::eSrgbNonlinear;
        swap_chain_info.imageExtent = surf_caps.currentExtent;
        swap_chain_info.imageArrayLayers = 1;
        swap_chain_info.imageUsage = ImageUsageFlagBits::eColorAttachment | ImageUsageFlagBits::eTransferDst;
        swap_chain_info.imageSharingMode = SharingMode::eExclusive;
        swap_chain_info.queueFamilyIndexCount = 0;
        swap_chain_info.pQueueFamilyIndices = nullptr;
        swap_chain_info.preTransform = surf_caps.currentTransform;
        swap_chain_info.compositeAlpha = CompositeAlphaFlagBitsKHR::eInherit;
        swap_chain_info.presentMode = PresentModeKHR::eMailbox;
        swap_chain_info.clipped = true;
        swap_chain_info.oldSwapchain = nullptr;

        m_swap_chain.reset(m_device->createSwapchainKHR(swap_chain_info));

        if constexpr(__ncv_logging_enabled)
            _log_android(log_level::debug) << "Swap chain create with success.";

        m_images = m_device->getSwapchainImagesKHR(m_swap_chain.get());

        // Create command pool

        CommandPoolCreateInfo pool_info;

        pool_info.flags = CommandPoolCreateFlagBits::eResetCommandBuffer;
        pool_info.queueFamilyIndex = m_selected_queue_family_index;

        m_cmd_pool.reset(m_device->createCommandPool(pool_info));

        // Allocate command buffers one for each chain image

        CommandBufferAllocateInfo cmd_buf_info;

        cmd_buf_info.commandPool = m_cmd_pool.get();
        cmd_buf_info.level = CommandBufferLevel::ePrimary;
        cmd_buf_info.commandBufferCount = m_images.size();

        m_cmd_buffers = m_device->allocateCommandBuffers(cmd_buf_info);

        if constexpr(__ncv_logging_enabled)
            _log_android(log_level::debug) << "Command buffers created with success.";
    }

    void simple_context::render_frame(const std::any& a_params)
    {
        vec4 a_rgba = any_cast<vec4>(a_params);

        auto img_idx = m_device->acquireNextImageKHR(m_swap_chain.get(), UINT64_MAX, m_semaphore.get());

        CommandBufferBeginInfo begin_info;

        begin_info.flags = CommandBufferUsageFlagBits::eSimultaneousUse;
        begin_info.pInheritanceInfo = nullptr;

        ClearColorValue clear_color;
        memcpy(&clear_color.float32, &a_rgba, sizeof(clear_color.float32));
        
        ImageSubresourceRange img_subrange;

        img_subrange.aspectMask = ImageAspectFlagBits::eColor;
        img_subrange.baseMipLevel = 0;
        img_subrange.levelCount = 1;
        img_subrange.baseArrayLayer = 0;
        img_subrange.layerCount = 1;

        ImageMemoryBarrier pres_to_clear;

        pres_to_clear.srcAccessMask = AccessFlagBits::eColorAttachmentRead;
        pres_to_clear.dstAccessMask = AccessFlagBits::eColorAttachmentWrite;
        pres_to_clear.oldLayout = ImageLayout::eUndefined;
        pres_to_clear.newLayout = ImageLayout::eTransferDstOptimal;
        pres_to_clear.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        pres_to_clear.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        pres_to_clear.image = m_images[img_idx.value];
        pres_to_clear.subresourceRange = img_subrange;

        ImageMemoryBarrier clear_to_pres;

        clear_to_pres.srcAccessMask =  AccessFlagBits::eColorAttachmentWrite;
        clear_to_pres.dstAccessMask = AccessFlagBits::eColorAttachmentRead;
        clear_to_pres.oldLayout = ImageLayout::eTransferDstOptimal;
        clear_to_pres.newLayout = ImageLayout::ePresentSrcKHR;
        clear_to_pres.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        clear_to_pres.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        clear_to_pres.image = m_images[img_idx.value];
        clear_to_pres.subresourceRange = img_subrange;

        m_device->waitIdle();
        m_cmd_buffers[img_idx.value].reset(CommandBufferResetFlagBits::eReleaseResources);
        m_cmd_buffers[img_idx.value].begin(begin_info);
        m_cmd_buffers[img_idx.value].pipelineBarrier(PipelineStageFlagBits::eColorAttachmentOutput,
            PipelineStageFlagBits::eColorAttachmentOutput, static_cast<DependencyFlags>(0), 0, nullptr,
            0, nullptr, 1, &pres_to_clear);
        m_cmd_buffers[img_idx.value].clearColorImage(m_images[img_idx.value],
            ImageLayout::eTransferDstOptimal, clear_color, img_subrange);
        m_cmd_buffers[img_idx.value].pipelineBarrier(PipelineStageFlagBits::eColorAttachmentOutput,
            PipelineStageFlagBits::eColorAttachmentOutput, static_cast<DependencyFlags>(0), 0, nullptr,
            0, nullptr, 1, &clear_to_pres);
        m_cmd_buffers[img_idx.value].end();

        SubmitInfo submit_info;
        auto wdst_mask = PipelineStageFlags(PipelineStageFlagBits::eColorAttachmentOutput);

        submit_info.waitSemaphoreCount = 1;
        submit_info.pWaitSemaphores = &(m_semaphore.get());
        submit_info.pWaitDstStageMask = &wdst_mask;
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &m_cmd_buffers[img_idx.value];
        submit_info.signalSemaphoreCount = 1;
        submit_info.pSignalSemaphores = &m_render_semaphore.get();

        m_pres_queue.submit(submit_info);

        PresentInfoKHR pres_info;

        pres_info.waitSemaphoreCount = 1;
        pres_info.pWaitSemaphores = &m_render_semaphore.get();
        pres_info.swapchainCount = 1;
        pres_info.pSwapchains = &m_swap_chain.get();
        pres_info.pImageIndices = &img_idx.value;

        auto result = m_pres_queue.presentKHR(pres_info);

        if(result == Result::eSuboptimalKHR)
            initialize_graphics(m_app);
        else if(result != Result::eSuccess)
            throw runtime_error{to_string(result) + string(". Failed Rendering Frame.")};
    }
}