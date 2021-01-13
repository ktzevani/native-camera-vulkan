#ifndef NCV_GRAPHICS_COMPLETE_CONTEXT_HPP
#define NCV_GRAPHICS_COMPLETE_CONTEXT_HPP

#include <graphics/vulkan_context.hpp>
#include <metadata/version.hpp>
#include <graphics/data/types.hpp>
#include <graphics/resources/types.hpp>

#include <map>
#include <any>

class android_app;

namespace graphics
{
    class pipeline;

    class complex_context : public vulkan_context
    {
    public:

        typedef resources::buffer<resources::device_upload, data::index_format> index_data;
        typedef resources::buffer<resources::host, data::model_view_projection> uniform_data;
        typedef resources::buffer<resources::device_upload, data::vertex_format> vertex_data;
        typedef resources::image<resources::external> camera_data;
        typedef resources::image<resources::device> depth_data;
        typedef resources::image<resources::device_upload, data::stbi_uc> texture_data;

        struct descriptor_configuration
        {
            std::array<vk::DescriptorBufferInfo, 1> buffer_infos;
            std::array<vk::DescriptorImageInfo, 1> image_infos;
            std::array<vk::WriteDescriptorSet, 2> writes;
        };

        explicit complex_context(const std::string &a_app_name);
        ~complex_context();
        void initialize_graphics(android_app *a_app, AHardwareBuffer* a_buffer = nullptr);
        void render_frame(const std::any &a_params, AHardwareBuffer* a_buffer = nullptr);

    protected:

        template<typename T>
        void log_requested_instance_info() const;

        template<typename T>
        void log_physical_devices_info() const;

        template<typename T>
        void log_primary_device_info() const;

        template<typename T>
        void log_requested_logical_device_info() const;

        template<typename T>
        void log_surface_info(const vk::SurfaceCapabilitiesKHR& a_surf_caps,
            const std::vector<vk::SurfaceFormatKHR>& a_surf_fmts,
            const std::vector<vk::PresentModeKHR>& a_pres_modes) const;

        void reset_surface(ANativeWindow* a_window);

        void select_device_and_qfamily();

        void create_logical_device();

        void create_render_pass();

        void create_graphics_pipeline();

        void create_data_buffers(AHardwareBuffer* a_buffer);

        void reset_swapchain();

        void reset_framebuffer_and_zbuffer();

        void reset_sync_and_cmd_resources();

        void reset_ubos_samplers_and_descriptors();

        void reset_camera();

        void release_rendering_resources();

    private:

        std::vector<const char*> m_requested_gl_extensions = {
            VK_KHR_SURFACE_EXTENSION_NAME,
            VK_KHR_ANDROID_SURFACE_EXTENSION_NAME,
            VK_KHR_EXTERNAL_MEMORY_CAPABILITIES_EXTENSION_NAME,
            VK_KHR_EXTERNAL_SEMAPHORE_CAPABILITIES_EXTENSION_NAME,
            VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME
        };

        std::vector<const char*> m_requested_gl_layers = {
            "VK_LAYER_KHRONOS_validation"
        };

        std::vector<const char*> m_requested_dev_extensions = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME,
            VK_KHR_MAINTENANCE1_EXTENSION_NAME,
            VK_KHR_BIND_MEMORY_2_EXTENSION_NAME,
            VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME,
            VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME,
            VK_EXT_QUEUE_FAMILY_FOREIGN_EXTENSION_NAME,
            VK_KHR_EXTERNAL_MEMORY_EXTENSION_NAME,
            VK_KHR_EXTERNAL_SEMAPHORE_EXTENSION_NAME,
            VK_KHR_EXTERNAL_SEMAPHORE_FD_EXTENSION_NAME,
            VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME,
            VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_EXTENSION_NAME
        };

        bool is_initialized = false;

#ifdef NCV_VULKAN_VALIDATION_ENABLED
        const bool m_enable_validation = true;
#else
        const bool m_enable_validation = false;
#endif

        std::vector<vk::PhysicalDevice> m_devices;

        vk::PhysicalDevice m_gpu;

        vk::UniqueInstance m_instance;
        vk::UniqueSurfaceKHR m_surface;
        vk::UniqueDevice m_device;
        vk::UniqueSwapchainKHR m_swap_chain;
        vk::UniqueCommandPool m_cmd_pool;

        std::vector<vk::CommandBuffer> m_cmd_buffers;
        std::vector<vk::Fence> m_cmd_fences;
        std::vector<vk::Image> m_images;
        std::vector<vk::ImageView> m_swapchain_img_views;
        std::vector<vk::Framebuffer> m_framebuffers;
        std::vector<vk::Semaphore> m_proc_semaphores;
        std::vector<vk::Semaphore> m_pres_semaphores;

        uint32_t m_proc_si {0};

        vk::RenderPass m_render_pass = nullptr;

        vk::Queue m_pres_queue;
        int32_t m_qfam_index = -1;

        vk::Extent2D m_surface_extent;

        std::shared_ptr<pipeline> m_graphics_pipeline = nullptr;
        std::shared_ptr<vertex_data> m_vertex_data = nullptr;
        std::shared_ptr<index_data> m_index_data = nullptr;

        std::vector<vk::Sampler> m_samplers;
        std::shared_ptr<camera_data> m_camera_image = nullptr;
        std::shared_ptr<depth_data> m_depth_buffer = nullptr;

        std::shared_ptr<data::texture> m_stbi_data = nullptr;
        vk::Extent3D m_stbi_extent;
        std::shared_ptr<texture_data> m_texture_data = nullptr;

        vk::UniqueDescriptorPool m_desc_pool;
        std::vector<vk::DescriptorSet> m_desc_sets;
        std::vector<descriptor_configuration> m_desc_configs;

        std::vector<data::model_view_projection> m_mvp_data;
        std::vector<std::shared_ptr<uniform_data>> m_uniform_data;

        vk::DebugUtilsMessengerEXT m_debug_msg;

        std::chrono::time_point<std::chrono::steady_clock> m_ref_time;

        android_app* m_app = nullptr;
    };

    template<typename T = void>
    inline void complex_context::log_requested_instance_info() const
    {
        using namespace ::utilities;
        using ::vk_util::version;

        auto debug = log_level::debug;
        auto logger = ::vk_util::logger{box_width, col_width + 15, debug};
        auto cc = align::ccenter;

        logger.box_top();
        logger.write_line("REQUESTED INSTANCE INFORMATION", cc);
        logger.box_separator();
        logger.write_line("Requested Extensions", cc);
        logger.write_line("‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾", cc);
        for (auto &extension : m_requested_gl_extensions)
            logger.write_line((std::string("    ") + extension).c_str());
        logger.box_separator();
        logger.write_line("Requested Layers", cc);
        logger.write_line("‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾", cc);
        for (auto &layer : m_requested_gl_layers)
            logger.write_line((std::string("    ") + layer).c_str());
        logger.box_separator();
        logger.write_line("Instance Properties", cc);
        logger.write_line("‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾", cc);
        logger.write_line_2col("    Engine Name:", "AndroidVulkanEngine");
        logger.write_line_2col("    Engine Version:", (::metadata::PROJECT_VERSION.full_version()).c_str());
        logger.write_line_2col("    Vulkan API Version:", version(VK_API_VERSION_1_2).to_string().c_str());
        logger.box_bottom();
    }

    template<typename T = void>
    inline void complex_context::log_physical_devices_info() const
    {
        using namespace ::utilities;
        using namespace ::vk_util;

        auto debug = log_level::debug;
        auto logger = ::vk_util::logger{box_width, col_width, debug};
        auto cc = align::ccenter;
        std::stringstream sstr;
        auto sort_extension = [](const auto& lhs, const auto& rhs)
        { return std::string(lhs.extensionName) < std::string(rhs.extensionName); };

        logger.box_top();
        logger.write_line("PHYSICAL DEVICES INFORMATION", cc);

        uint32_t devi = 0;

        for(auto& device : m_devices)
        {
            logger.box_separator();
            auto extensions = device.enumerateDeviceExtensionProperties();
            sort(extensions.begin(), extensions.end(), sort_extension);
            sstr << "Device Index: " << devi++;
            logger.write_line(sstr.str().c_str(), cc);
            logger.box_separator();
            logger.write_line("Available Extensions", cc);
            logger.write_line("‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾", cc);
            for(auto& extension : extensions)
                logger.write_line_2col(("    " + std::string(extension.extensionName)).c_str(),
                                       ("v" + version(extension.specVersion).to_string()).c_str());
            logger.box_separator();
            logger.write_line("Supported Queue Families", cc);
            logger.write_line("‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾", cc);

            auto queue_props = device.getQueueFamilyProperties();
            uint32_t qi {0};

            for(auto& qprops : queue_props)
            {
                bool pflag = device.getSurfaceSupportKHR(qi,m_surface.get());
                sstr.str(""); sstr << "  Queue Family: " << qi++;
                logger.write_line(sstr.str().c_str());
                sstr.str(""); sstr << "    Flags: " <<to_string(qprops.queueFlags);
                logger.write_line(sstr.str().c_str());
                sstr.str(""); sstr << "    Count: " << qprops.queueCount;
                logger.write_line(sstr.str().c_str());
                sstr.str(""); sstr << "    Presentation: " << (pflag ? "True" : "False");
                logger.write_line(sstr.str().c_str());
            }

            logger.box_separator();
            logger.write_line("Supported Depth Formats", cc);
            logger.write_line("‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾", cc);

            using vk::Format;

            std::vector<Format> formats = {
                    Format::eD16Unorm,
                    Format::eD16UnormS8Uint,
                    Format::eD24UnormS8Uint,
                    Format::eD32Sfloat,
                    Format::eD32SfloatS8Uint
            };

            for(auto& format : formats)
            {
                auto props = device.getFormatProperties(format);
                sstr.str(""); sstr << "  " << to_string(format);
                logger.write_line(sstr.str().c_str());
                logger.write_multiline("    Linear: ", split_to_vector(props.linearTilingFeatures));
                logger.write_multiline("    Optimal: ", split_to_vector(props.optimalTilingFeatures));
            }

            auto features = device.getFeatures();

            vk::PhysicalDeviceSamplerYcbcrConversionFeatures ycbcr_features;
            vk::PhysicalDeviceFeatures2KHR aug_features;

            aug_features.pNext = &ycbcr_features;
            device.getFeatures2KHR(&aug_features);

            logger.box_separator();
            logger.write_line("Supported Features", cc);
            logger.write_line("‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾", cc);
            sstr.str(""); sstr << "  Sampler Anisotropy: ";
            logger.write_line_2col(sstr.str().c_str(), features.samplerAnisotropy ? "True" : "False");
            sstr.str(""); sstr << "  Sampler YCBCR Conversion: ";
            logger.write_line_2col(sstr.str().c_str(), ycbcr_features.samplerYcbcrConversion ? "True" : "False");
        }

        if(this->m_devices.size() < 1)
        {
            logger.box_separator();
            logger.write_line("<No Physical Devices Found>", cc);
        }

        logger.box_bottom();
    }

    template<typename T = void>
    inline void complex_context::log_primary_device_info() const
    {
        using namespace ::utilities;
        using namespace ::vk_util;

        auto debug = log_level::debug;
        auto logger = ::vk_util::logger{box_width, col_width, debug};
        auto cc = align::ccenter;
        auto gpu_props = m_gpu.getProperties();
        version api_version = gpu_props.apiVersion;
        version driver_version = gpu_props.driverVersion;
        auto gpu_mem_props = m_gpu.getMemoryProperties();
        std::stringstream sstr;

        logger.box_top();
        logger.write_line("PRIMARY DEVICE INFORMATION", cc);
        logger.box_separator();
        logger.write_line("General", cc);
        logger.write_line("‾‾‾‾‾‾‾", cc);
        sstr.str(""); sstr << "    API Version: " << api_version.to_string();
        logger.write_line(sstr.str().c_str());
        sstr.str(""); sstr << "    Driver Version: " << driver_version.to_string();
        logger.write_line(sstr.str().c_str());
        sstr.str(""); sstr << "    Device Name: " << gpu_props.deviceName;
        logger.write_line(sstr.str().c_str());
        sstr.str(""); sstr << "    Device Type: " << to_string(gpu_props.deviceType);
        logger.write_line(sstr.str().c_str());
        sstr.str(""); sstr << "    Memory Heaps: " << gpu_mem_props.memoryHeapCount;
        logger.box_separator();
        logger.write_line("Memory Information", cc);
        logger.write_line("‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾", cc);
        uint32_t hi {0};

        for(auto it = gpu_mem_props.memoryHeaps.begin(); it->size > 0; ++it, ++hi);

        std::vector<std::map<vk::MemoryPropertyFlags, bool>> mem_types(hi);

        sstr.str(""); sstr << "  Available heaps: " << hi;
        logger.write_line(sstr.str().c_str());
        logger.write_line("");

        for(auto& mtype : gpu_mem_props.memoryTypes)
            mem_types[mtype.heapIndex][mtype.propertyFlags] = true;

        hi = 0;

        for(auto& heap : gpu_mem_props.memoryHeaps)
        {
            sstr.str(""); sstr << "  Heap " << hi++;
            logger.write_line(sstr.str().c_str());
            sstr.str(""); sstr << "    Size: " << readable_size(heap.size);
            logger.write_line(sstr.str().c_str());
            sstr.str(""); sstr << "    Flags: " <<to_string(heap.flags);
            logger.write_line(sstr.str().c_str());
            sstr.str(""); sstr << "    Supported Types: ";
            logger.write_line(sstr.str().c_str());
            for(auto& mask : mem_types[hi-1]) {
                if(mask.first)
                    logger.write_multiline("    - ", split_to_vector(mask.first));
                else
                {
                    sstr.str(""); sstr << "    - <Unknown>";
                    logger.write_line(sstr.str().c_str());
                }
            }
            if(hi == mem_types.size())
                break;
        }

        logger.box_separator();
        logger.write_line("Device Limitations", cc);
        logger.write_line("‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾", cc);
        sstr.str(""); sstr << "  Maximun allocated objects: " << gpu_props.limits.maxMemoryAllocationCount;
        logger.write_line(sstr.str().c_str());
        sstr.str(""); sstr << "  Maximun sampler anisotropy: " << gpu_props.limits.maxSamplerAnisotropy;
        logger.write_line(sstr.str().c_str());
        sstr.str(""); sstr << "  Maximun samplers allocations: " << gpu_props.limits.maxSamplerAllocationCount;
        logger.write_line(sstr.str().c_str());
        logger.box_bottom();
    }

    template<typename T = void>
    inline void complex_context::log_requested_logical_device_info() const
    {
        auto debug = ::utilities::log_level::debug;
        auto logger = ::vk_util::logger{box_width, col_width, debug};
        auto cc = align::ccenter;
        std::stringstream sstr;
        logger.box_top();
        logger.write_line("REQUESTED LOGICAL DEVICE INFORMATION", cc);
        logger.box_separator();
        logger.write_line("Requested Extensions", cc);
        logger.write_line("‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾", cc);
        for(auto& extension : m_requested_dev_extensions)
            logger.write_line((std::string("    ") + extension).c_str());
        logger.box_separator();
        logger.write_line("Instantiated Queues", cc);
        logger.write_line("‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾", cc);
        sstr.str(""); sstr << "  Queue Family Index: " << m_qfam_index;
        logger.write_line(sstr.str().c_str());
        sstr.str(""); sstr << "  Number of Queues: " << 1;
        logger.write_line(sstr.str().c_str());
        sstr.str(""); sstr << "  Priorities: { " << 1.0f << " }";
        logger.write_line(sstr.str().c_str());
        logger.box_bottom();
    }

    template<typename T = void>
    inline void complex_context::log_surface_info(const vk::SurfaceCapabilitiesKHR& a_surf_caps,
        const std::vector<vk::SurfaceFormatKHR>& a_surf_fmts,
        const std::vector<vk::PresentModeKHR>& a_pres_modes) const
    {
        using ::vk_util::split_to_vector;

        auto debug = ::utilities::log_level::debug;
        auto logger = ::vk_util::logger{box_width, col_width+10, debug};
        auto cc = align::ccenter;
        std::stringstream sstr;
        logger.box_top();
        logger.write_line("SURFACE INFORMATION", cc);
        logger.box_separator();
        logger.write_line("Images Information", cc);
        logger.write_line("‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾", cc);
        sstr.str(""); sstr << "  Min Image Count: " << a_surf_caps.minImageCount;
        logger.write_line(sstr.str().c_str());
        sstr.str(""); sstr << "  Max Image Count: " << a_surf_caps.maxImageCount;
        logger.write_line(sstr.str().c_str());
        sstr.str(""); sstr << "  Min Image Extend: (" << a_surf_caps.minImageExtent.width
                           << ", " << a_surf_caps.minImageExtent.height << ")";
        logger.write_line(sstr.str().c_str());
        sstr.str(""); sstr << "  Max Image Extend: (" << a_surf_caps.maxImageExtent.width
                           << ", " << a_surf_caps.maxImageExtent.height << ")";
        logger.write_line(sstr.str().c_str());
        sstr.str(""); sstr << "  Current Image Extend: (" << a_surf_caps.currentExtent.width
                           << ", " << a_surf_caps.currentExtent.height << ")";
        logger.write_line(sstr.str().c_str());
        logger.box_separator();
        logger.write_line("Formats Supported", cc);
        logger.write_line("‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾", cc);
        for(auto& format : a_surf_fmts)
        {
            sstr.str(""); sstr << "    " <<to_string(format.format);
            logger.write_line_2col(sstr.str().c_str(),to_string(format.colorSpace).c_str());
        }
        logger.box_separator();
        logger.write_line("Surface Capabilities", cc);
        logger.write_line("‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾", cc);
        logger.write_multiline("  Usage: ", split_to_vector(a_surf_caps.supportedUsageFlags));
        logger.write_multiline("  Pre-transforms: ", split_to_vector(a_surf_caps.supportedTransforms));
        sstr.str(""); sstr << "  Current Pre-transform: " << to_string(a_surf_caps.currentTransform);
        logger.write_line(sstr.str().c_str());
        logger.box_separator();
        logger.write_line("Presentation Modes", cc);
        logger.write_line("‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾", cc);
        for (auto &mode : a_pres_modes)
        {
            sstr.str(""); sstr << "    " << to_string(mode);
            logger.write_line(sstr.str().c_str());
        }
        logger.box_bottom();
    }
}

#endif //NCV_GRAPHICS_COMPLETE_CONTEXT_HPP
