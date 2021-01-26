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

#ifndef NCV_RESOURCES_IMAGE_HPP
#define NCV_RESOURCES_IMAGE_HPP

#include <graphics/resources/base.hpp>
#include <graphics/resources/types.hpp>

namespace graphics{ namespace resources{

    class image_base : public base
    {
    public:

        image_base(const vk::PhysicalDevice& a_gpu, const vk::UniqueDevice& a_device)
            : base{a_gpu, a_device}
        {}

        virtual ~image_base()  { destroy_resources(); }

        vk::Image& get() { return m_image; }
        vk::ImageView& get_img_view() { return m_img_view; }

    protected:

        virtual void destroy_resources() noexcept;

        vk::DeviceMemory m_memory = nullptr;
        vk::Image m_image = nullptr;
        vk::ImageView m_img_view = nullptr;
    };

    template<typename Policy, typename ImageDataFormat>
    class image : public image_base
    {};

    template<>
    class image<external> : public image_base
    {
    public:
        image(const vk::PhysicalDevice& a_gpu, const vk::UniqueDevice& a_device, AHardwareBuffer* a_buffer);
        ~image() { destroy_resources(); }
        void update(vk::ImageUsageFlags a_usage, vk::SharingMode a_sharing, AHardwareBuffer* a_buffer);
        vk::Sampler& get_sampler() { return m_sampler; }
    private:

        void destroy_resources() noexcept override;

        vk::Sampler m_sampler = nullptr;
        vk::SamplerYcbcrConversion m_conversion = nullptr;
    };

    template<typename ImageDataFormat>
    class image<host, ImageDataFormat> {};

    template<>
    class image<device> : public image_base
    {
    public:
        image(const vk::PhysicalDevice& a_gpu, const vk::UniqueDevice& a_device,
            vk::ImageUsageFlags a_usage, vk::SharingMode a_sharing, const vk::Extent2D& a_extent);
    };

    template<typename ImageDataFormat>
    class image<device_upload, ImageDataFormat> : public image_base
    {
    public:
        image(const vk::PhysicalDevice& a_gpu, const vk::UniqueDevice& a_device,
            vk::ImageUsageFlags a_usage, vk::SharingMode a_sharing, vk::Extent3D& a_extent,
            vk::Format a_format, const std::vector<ImageDataFormat>& a_data);
        ~image() { destroy_resources(); }
        void update_staging(const std::vector<ImageDataFormat>& a_data);
        vk::Buffer& get_staging() { return m_staging_buffer; }
        vk::DeviceSize size_staging() { return m_staging_size; }
    private:

        void destroy_resources() noexcept override;

        vk::DeviceSize m_staging_size = 0;
        vk::DeviceMemory m_staging_memory = nullptr;
        vk::Buffer m_staging_buffer = nullptr;
    };

    template<typename ImageDataFormat>
    image<device_upload, ImageDataFormat>::image(const vk::PhysicalDevice &a_gpu,
        const vk::UniqueDevice &a_device, vk::ImageUsageFlags a_usage, vk::SharingMode a_sharing,
        vk::Extent3D &a_extent, vk::Format a_format, const std::vector<ImageDataFormat> &a_data)
        : image_base{a_gpu, a_device}
    {
        using namespace ::vk;

        int num_channels;

        switch (a_format)
        {
            case vk::Format::eR8G8B8A8Srgb:
            case vk::Format::eB8G8R8A8Srgb:
                num_channels = 4;
                break;
            case vk::Format::eR8G8B8Srgb:
            case vk::Format::eB8G8R8Srgb:
                num_channels = 3;
                break;
            default:
                num_channels = -1;
        }

        if(num_channels == -1)
            throw std::runtime_error{"Format is not supported."};

        m_data_size = a_extent.width * a_extent.height * num_channels;

        BufferCreateInfo staging_buffer_info;

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
        catch (std::exception const &e)
        {
            destroy_resources();
            throw e;
        }

        uint8_t* pt;
        MemoryMapFlags map_flags {0};

        auto mresult = m_device.mapMemory(m_staging_memory, 0, mem_reqs.size, map_flags,
            reinterpret_cast<void**>(&pt));

        if(mresult == Result::eSuccess)
        {
            memcpy(pt, a_data.data(), m_staging_size);
        }
        else
        {
            destroy_resources();
            throw std::runtime_error{"Result is: " + to_string(mresult) +
                ". Could not copy image data to staging buffer."};
        }

        m_device.unmapMemory(m_staging_memory);

        try
        {
            m_device.bindBufferMemory(m_staging_buffer, m_staging_memory, 0);
        }
        catch (std::exception const &e)
        {
            destroy_resources();
            throw e;
        }

        ImageCreateInfo image_info;

        //image_info.flags
        image_info.imageType = ImageType::e2D;
        image_info.format = a_format;
        image_info.extent.width = a_extent.width;
        image_info.extent.height = a_extent.height;
        image_info.extent.depth = a_extent.depth;
        image_info.mipLevels = 1;
        image_info.arrayLayers = 1;
        image_info.samples = SampleCountFlagBits::e1;
        image_info.tiling = ImageTiling::eOptimal;
        image_info.usage = ImageUsageFlagBits::eTransferDst | a_usage;
        image_info.sharingMode = a_sharing;
        image_info.queueFamilyIndexCount = 0;
        image_info.pQueueFamilyIndices = nullptr;
        image_info.initialLayout = ImageLayout::eUndefined;

        m_image = m_device.createImage(image_info);

        mem_reqs = m_device.getImageMemoryRequirements(m_image);

        mem_info.memoryTypeIndex = get_memory_index(mem_reqs.memoryTypeBits, memory_location::device);
        mem_info.allocationSize = mem_reqs.size;

        m_size = mem_reqs.size;

        ImageViewCreateInfo img_view_info;

        img_view_info.image = m_image;
        img_view_info.viewType = ImageViewType::e2D;
        img_view_info.format = a_format;
        //img_view_info.components
        img_view_info.subresourceRange.aspectMask = ImageAspectFlagBits::eColor;
        img_view_info.subresourceRange.baseMipLevel = 0;
        img_view_info.subresourceRange.levelCount = 1;
        img_view_info.subresourceRange.baseArrayLayer = 0;
        img_view_info.subresourceRange.layerCount = 1;

        try
        {
            m_memory = m_device.allocateMemory(mem_info);
            m_device.bindImageMemory(m_image, m_memory, 0);
            m_img_view = m_device.createImageView(img_view_info);
        }
        catch(std::exception const &e)
        {
            destroy_resources();
            throw e;
        }
    }

    template<typename ImageDataFormat>
    void image<device_upload, ImageDataFormat>::destroy_resources() noexcept
    {
        if(!!m_staging_buffer)
            m_device.destroyBuffer(m_staging_buffer);
        if(!!m_staging_memory)
            m_device.freeMemory(m_staging_memory);
    }

    template<typename ImageDataFormat>
    void image<device_upload, ImageDataFormat>::update_staging(const std::vector<ImageDataFormat> &a_data)
    {
        if(a_data.size() * sizeof(ImageDataFormat) != m_data_size)
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

#endif //NCV_RESOURCES_IMAGE_HPP
