#ifndef NCV_RESOURCES_BASE_HPP
#define NCV_RESOURCES_BASE_HPP

#include <vulkan_hpp/vulkan.hpp>

namespace graphics{ namespace resources{

    class base
    {
    public:

        enum class memory_location
        {
            device,
            host,
            external
        };

        base(const vk::PhysicalDevice& a_gpu, const vk::UniqueDevice& a_device);

        vk::DeviceSize size() { return m_size; }

    protected:

        int get_memory_index(uint32_t a_memory_type_bits, memory_location a_location) noexcept;

        vk::Device m_device = nullptr;
        vk::PhysicalDeviceMemoryProperties m_mem_props;
        size_t m_data_size = 0;
        vk::DeviceSize m_size = 0;
    };

}}

#endif //NCV_RESOURCES_BASE_HPP
