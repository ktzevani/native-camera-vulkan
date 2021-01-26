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

#ifndef NCV_RESOURCES_BUFFER_HPP
#define NCV_RESOURCES_BUFFER_HPP

#include <graphics/resources/base.hpp>
#include <graphics/resources/types.hpp>

namespace graphics{ namespace resources
{
    class buffer_base : public base
    {
    public:

        buffer_base(const vk::PhysicalDevice& a_gpu, const vk::UniqueDevice& a_device)
            : base{a_gpu, a_device}
        {}

        virtual ~buffer_base() { destroy_resources(); }

        vk::Buffer& get() { return m_buffer; }

    protected:

        virtual void destroy_resources() noexcept;

        vk::DeviceMemory m_memory = nullptr;
        vk::Buffer m_buffer = nullptr;
    };

    template<typename Policy, typename DataFormat>
    class buffer : public buffer_base
    {};

    template<typename DataFormat>
    class buffer<external, DataFormat> {};

    template<typename DataFormat>
    class buffer<host, DataFormat> : public buffer_base
    {
    public:
        buffer(const vk::PhysicalDevice& a_gpu, const vk::UniqueDevice& a_device,
            vk::BufferUsageFlags a_usage, vk::SharingMode a_sharing, const std::vector<DataFormat>& a_data);
        void update(const std::vector<DataFormat>& a_data);
    };

    template<>
    class buffer<device> : public buffer_base
    {
    public:
        buffer(const vk::PhysicalDevice& a_gpu, const vk::UniqueDevice& a_device,
            vk::BufferUsageFlags a_usage, vk::SharingMode a_sharing, uint32_t a_size);
    };

    template<typename DataFormat>
    class buffer<device_upload, DataFormat> : public buffer_base
    {
    public:
        buffer(const vk::PhysicalDevice& a_gpu, const vk::UniqueDevice& a_device,
            vk::BufferUsageFlags a_usage, vk::SharingMode a_sharing, const std::vector<DataFormat>& a_data);
        ~buffer() { destroy_resources(); }
        void update_staging(const std::vector<DataFormat>& a_data);
        vk::Buffer& get_staging() { return m_staging_buffer; }
        vk::DeviceSize size_staging() { return m_staging_size; }
    private:

        void destroy_resources() noexcept override;

        vk::DeviceSize m_staging_size = 0;
        vk::DeviceMemory m_staging_memory = nullptr;
        vk::Buffer m_staging_buffer = nullptr;
    };

    template<typename DataFormat>
    buffer<host, DataFormat>::buffer(const vk::PhysicalDevice &a_gpu, const vk::UniqueDevice &a_device,
        vk::BufferUsageFlags a_usage, vk::SharingMode a_sharing, const std::vector<DataFormat> &a_data)
        : buffer_base{a_gpu, a_device}
    {
        using namespace ::vk;
        using std::exception;
        using std::runtime_error;

        BufferCreateInfo device_buffer_info;

        m_data_size = a_data.size() * sizeof(DataFormat);

        device_buffer_info.usage = a_usage;
        device_buffer_info.size = m_data_size;
        device_buffer_info.sharingMode = a_sharing;

        m_buffer = m_device.createBuffer(device_buffer_info);

        auto mem_reqs = m_device.getBufferMemoryRequirements(m_buffer);

        m_size = mem_reqs.size;

        MemoryAllocateInfo mem_info;

        mem_info.memoryTypeIndex = get_memory_index(mem_reqs.memoryTypeBits, memory_location::host);
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

        uint8_t* pt;
        MemoryMapFlags map_flags {0};

        auto result = m_device.mapMemory(m_memory, 0, mem_reqs.size, map_flags,
            reinterpret_cast<void**>(&pt));

        if(result == Result::eSuccess)
        {
            memcpy(pt, a_data.data(), m_size);
        }
        else
        {
            destroy_resources();
            throw runtime_error{"Result is: " + to_string(result) + ". Could not copy host data."};
        }

        m_device.unmapMemory(m_memory);

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

    template<typename DataFormat>
    inline void buffer<host, DataFormat>::update(const std::vector<DataFormat> &a_data)
    {
        if(a_data.size() * sizeof(DataFormat) != m_data_size)
            throw std::runtime_error{"Data size differs. Cannot update buffer."};

        uint8_t* pt;
        vk::MemoryMapFlags map_flags {0};

        auto result = m_device.mapMemory(m_memory, 0, m_size, map_flags, reinterpret_cast<void**>(&pt));

        if(result == vk::Result::eSuccess)
            memcpy(pt, a_data.data(), m_size);
        else
            throw std::runtime_error{"Result is: " + to_string(result) + ". Could not copy host data."};

        m_device.unmapMemory(m_memory);
    }

    template<typename DataFormat>
    buffer<device_upload, DataFormat>::buffer(const vk::PhysicalDevice &a_gpu, const vk::UniqueDevice &a_device,
        vk::BufferUsageFlags a_usage, vk::SharingMode a_sharing, const std::vector<DataFormat> &a_data)
        : buffer_base{a_gpu, a_device}
    {
        using namespace ::vk;
        using std::exception;
        using std::runtime_error;

        BufferCreateInfo staging_buffer_info;

        m_data_size = a_data.size() * sizeof(DataFormat);

        staging_buffer_info.usage = BufferUsageFlagBits::eTransferSrc;
        staging_buffer_info.size = m_data_size;
        staging_buffer_info.sharingMode = a_sharing;

        m_staging_buffer = m_device.createBuffer(staging_buffer_info);

        auto mem_reqs = m_device.getBufferMemoryRequirements(m_staging_buffer);

        m_staging_size = mem_reqs.size;

        MemoryAllocateInfo mem_info;

        mem_info.memoryTypeIndex = get_memory_index(mem_reqs.memoryTypeBits, memory_location::host);
        mem_info.allocationSize = mem_reqs.size;

        try
        {
            m_staging_memory = m_device.allocateMemory(mem_info);
        }
        catch(exception const &e)
        {
            destroy_resources();
            throw e;
        }

        uint8_t* pt;
        MemoryMapFlags map_flags {0};

        auto result = m_device.mapMemory(m_staging_memory, 0, m_staging_size, map_flags,
             reinterpret_cast<void**>(&pt));

        if(result == Result::eSuccess)
        {
            memcpy(pt, a_data.data(), m_staging_size);
        }
        else
        {
            destroy_resources();
            throw runtime_error{"Result is: " + to_string(result) + ". Could not copy host data."};
        }

        m_device.unmapMemory(m_staging_memory);

        try
        {
            m_device.bindBufferMemory(m_staging_buffer, m_staging_memory, 0);
        }
        catch(exception const &e)
        {
            destroy_resources();
            throw e;
        }

        BufferCreateInfo device_buffer_info;

        device_buffer_info.usage = BufferUsageFlagBits::eTransferDst | a_usage;
        device_buffer_info.size = m_data_size;
        device_buffer_info.sharingMode = a_sharing;

        try
        {
            m_buffer = m_device.createBuffer(device_buffer_info);
        }
        catch (exception const &e)
        {
            destroy_resources();
            throw e;
        }

        mem_reqs = m_device.getBufferMemoryRequirements(m_buffer);

        m_size = mem_reqs.size;

        mem_info.memoryTypeIndex = get_memory_index(mem_reqs.memoryTypeBits, memory_location::device);
        mem_info.allocationSize = mem_reqs.size;

        try
        {
            m_memory = m_device.allocateMemory(mem_info);
            m_device.bindBufferMemory(m_buffer, m_memory,0);
        }
        catch (exception const &e)
        {
            destroy_resources();
            throw e;
        }
    }

    template<typename DataFormat>
    inline void buffer<device_upload, DataFormat>::destroy_resources() noexcept
    {
        if(!!m_staging_buffer)
            m_device.destroyBuffer(m_staging_buffer);
        if(!!m_staging_memory)
            m_device.freeMemory(m_staging_memory);
    }

    template<typename DataFormat>
    inline void buffer<device_upload, DataFormat>::update_staging(const std::vector<DataFormat> &a_data)
    {
        if(a_data.size() * sizeof(DataFormat) != m_data_size)
            throw std::runtime_error{"Data size differs. Cannot update buffer."};

        uint8_t* pt;
        vk::MemoryMapFlags map_flags {0};

        auto result = m_device.mapMemory(m_staging_memory, 0, m_staging_size, map_flags,
            reinterpret_cast<void**>(&pt));

        if(result == vk::Result::eSuccess)
            memcpy(pt, a_data.data(), m_staging_size);
        else
            throw std::runtime_error{"Result is: " + to_string(result) + ". Could not copy host data."};

        m_device.unmapMemory(m_staging_memory);
    }
}}

#endif //NCV_RESOURCES_BUFFER_HPP
