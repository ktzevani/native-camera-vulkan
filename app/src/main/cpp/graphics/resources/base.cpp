#include <graphics/resources/base.hpp>

//using namespace ::std;
using namespace ::vk;

namespace graphics{ namespace resources{

    base::base(const PhysicalDevice& a_gpu, const UniqueDevice& a_device)
        : m_device{a_device.get()}
    {
        m_mem_props = a_gpu.getMemoryProperties();
    }

    int base::get_memory_index(uint32_t a_memory_type_bits, memory_location a_location) noexcept
    {
        int mem_index {0};

        MemoryPropertyFlags mem_flags = MemoryPropertyFlags{0};

        switch(a_location)
        {
            case memory_location::device:
                mem_flags = MemoryPropertyFlagBits::eDeviceLocal;
                break;
            case memory_location::host:
                mem_flags = MemoryPropertyFlagBits::eHostVisible | MemoryPropertyFlagBits::eHostCoherent;
                break;
            default:
                break;
        }

        if(a_location == memory_location::external)
        {
            for(auto& type : m_mem_props.memoryTypes)
            {
                if((a_memory_type_bits & (1 << mem_index)) &&
                   ((type.propertyFlags & mem_flags) == MemoryPropertyFlags{0}))
                    break;
                mem_index++;
            }
        }
        else
        {
            for(auto& type : m_mem_props.memoryTypes)
            {
                if((a_memory_type_bits & (1 << mem_index)) &&
                   (type.propertyFlags & mem_flags))
                    break;
                mem_index++;
            }
        }

        return mem_index;
    }

}}
