#include <graphics/vulkan_context.hpp>

using namespace ::std;
using namespace ::vk;
using namespace ::utilities;

namespace graphics
{
    vulkan_context::vulkan_context()
    {
        // Setup dynamic dispatcher

        m_libvulkan = dlopen("libvulkan.so", RTLD_NOW | RTLD_LOCAL);

        if constexpr(__ncv_logging_enabled)
        {
            if(!m_libvulkan)
                _log_android(log_level::error) << "Couldn't load shared vulkan library.";
            else
                _log_android(log_level::info) << "Vulkan library loaded with success.";
        }

        if(!m_libvulkan)
            throw runtime_error{string("Error is: ") + to_string(Result::eErrorInitializationFailed)
                                + string(". No Appropriate Device Found.")};

        PFN_vkGetInstanceProcAddr getInstanceProcAddr =
            reinterpret_cast<PFN_vkGetInstanceProcAddr>(dlsym(m_libvulkan, "vkGetInstanceProcAddr"));

        VULKAN_HPP_DEFAULT_DISPATCHER.init(getInstanceProcAddr);

        // Get global information about available extensions and layers

        m_avail_extensions = enumerateInstanceExtensionProperties(nullptr);
        m_avail_layers = enumerateInstanceLayerProperties();

        auto sort_extension = [](const auto& lhs, const auto& rhs)
        { return string(lhs.extensionName) < string(rhs.extensionName); };
        auto sort_layer = [](const auto& lhs, const auto& rhs)
        { return string(lhs.layerName) < string(rhs.layerName); };

        sort(m_avail_extensions.begin(),m_avail_extensions.end(), sort_extension);
        sort(m_avail_layers.begin(),m_avail_layers.end(), sort_layer);

        for (const auto &layer_prop : m_avail_layers)
        {
            m_avail_layer_extensions.push_back(
                enumerateInstanceExtensionProperties(string(layer_prop.layerName)));
            sort(m_avail_layer_extensions.back().begin(), m_avail_layer_extensions.back().end(),
                 sort_extension);
        }

        if constexpr(__ncv_logging_enabled)
            log_system_info();
    }

    vulkan_context::~vulkan_context()
    {
        if(m_libvulkan != nullptr)
            dlclose(m_libvulkan);
    }

}