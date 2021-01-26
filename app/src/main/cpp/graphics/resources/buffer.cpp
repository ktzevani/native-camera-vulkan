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

#include <graphics/resources/buffer.hpp>

using namespace ::std;
using namespace ::vk;

namespace graphics{ namespace resources{

    void buffer_base::destroy_resources() noexcept
    {
        if(!!m_buffer)
            m_device.destroyBuffer(m_buffer);
        if(!!m_memory)
            m_device.freeMemory(m_memory);
    }

    buffer<device>::buffer(const PhysicalDevice &a_gpu, const UniqueDevice &a_device,
        BufferUsageFlags a_usage, SharingMode a_sharing, uint32_t a_size)
        : buffer_base{a_gpu, a_device}
    {
        BufferCreateInfo device_buffer_info;

        m_data_size = a_size;

        device_buffer_info.usage = a_usage;
        device_buffer_info.size = m_data_size;
        device_buffer_info.sharingMode = a_sharing;

        m_buffer = m_device.createBuffer(device_buffer_info);

        auto mem_reqs = m_device.getBufferMemoryRequirements(m_buffer);

        m_size = mem_reqs.size;

        MemoryAllocateInfo mem_info;

        mem_info.memoryTypeIndex = get_memory_index(mem_reqs.memoryTypeBits, memory_location::device);
        mem_info.allocationSize = mem_reqs.size;

        try
        {
            m_memory = m_device.allocateMemory(mem_info);
        }
        catch(exception const &e)
        {
            destroy_resources();
            throw e;
        }

        try
        {
            m_device.bindBufferMemory(m_buffer, m_memory, 0);
        }
        catch(exception const &e)
        {
            destroy_resources();
            throw e;
        }
    }
}}
