#include <graphics/resources/image.hpp>
#include <android_native_app_glue.h>

using namespace ::vk;

namespace graphics{ namespace resources{

    void image_base::destroy_resources() noexcept
    {
        if(!!m_img_view)
            m_device.destroyImageView(m_img_view);
        if(!!m_image)
            m_device.destroyImage(m_image);
        if(!!m_memory)
            m_device.freeMemory(m_memory);
        m_img_view = nullptr;
        m_image = nullptr;
        m_memory = nullptr;
    }

    image<external, void>::image(const PhysicalDevice &a_gpu, const UniqueDevice &a_device,
        AHardwareBuffer *a_buffer) : image_base{a_gpu, a_device}
    {
        AHardwareBuffer_Desc buffer_desc;
        AHardwareBuffer_describe(a_buffer, &buffer_desc);

        m_data_size = buffer_desc.width * buffer_desc.height * buffer_desc.layers;

        AndroidHardwareBufferFormatPropertiesANDROID format_info;
        AndroidHardwareBufferPropertiesANDROID properties_info;

        properties_info.pNext = &format_info;

        auto res = m_device.getAndroidHardwareBufferPropertiesANDROID(a_buffer, &properties_info);

        if(res != Result::eSuccess)
            throw std::runtime_error{"Error code: " + to_string(res) + ". Failed getting buffer properties."};

        ExternalMemoryImageCreateInfo ext_mem_info;

        ext_mem_info.handleTypes = ExternalMemoryHandleTypeFlagBitsKHR::eAndroidHardwareBufferANDROID;

        ExternalFormatANDROID external_format;
        SamplerYcbcrConversionCreateInfo conv_info;

        if(format_info.format == Format::eUndefined)
        {
            external_format.externalFormat = format_info.externalFormat;
            conv_info.pNext = &external_format;
            conv_info.format = Format::eUndefined;
        }
        else
        {
            conv_info.pNext = &external_format;
            conv_info.format = format_info.format;
        }

        conv_info.ycbcrModel = format_info.suggestedYcbcrModel;
        conv_info.ycbcrRange = format_info.suggestedYcbcrRange;
        conv_info.components = format_info.samplerYcbcrConversionComponents;
        conv_info.xChromaOffset = format_info.suggestedXChromaOffset;
        conv_info.yChromaOffset = format_info.suggestedYChromaOffset;
        conv_info.chromaFilter = Filter::eNearest;
        conv_info.forceExplicitReconstruction = false;

        m_conversion = m_device.createSamplerYcbcrConversion(conv_info);

        SamplerYcbcrConversionInfo conv_sampler_info;

        conv_sampler_info.conversion = m_conversion;

        SamplerCreateInfo sampler_info;

        sampler_info.pNext = &conv_sampler_info;
        sampler_info.magFilter = Filter::eNearest;
        sampler_info.minFilter = Filter::eNearest;
        sampler_info.mipmapMode = SamplerMipmapMode::eNearest;
        sampler_info.addressModeU = SamplerAddressMode::eClampToEdge;
        sampler_info.addressModeV = SamplerAddressMode::eClampToEdge;
        sampler_info.addressModeW = SamplerAddressMode::eClampToEdge;
        sampler_info.mipLodBias = 0.0f;
        sampler_info.anisotropyEnable = false;
        sampler_info.maxAnisotropy = 1.0f;
        sampler_info.compareEnable = false;
        sampler_info.compareOp = CompareOp::eNever;
        sampler_info.minLod = 0.0f;
        sampler_info.maxLod = 0.0f;
        sampler_info.borderColor = BorderColor::eFloatOpaqueWhite;
        sampler_info.unnormalizedCoordinates = false;

        m_sampler = m_device.createSampler(sampler_info);
    }

    void image<external, void>::destroy_resources() noexcept
    {
        if(!!m_sampler)
            m_device.destroySampler(m_sampler);
        if(!!m_conversion)
            m_device.destroySamplerYcbcrConversion(m_conversion);
    }

    void image<external, void>::update(ImageUsageFlags a_usage, SharingMode a_sharing,
        AHardwareBuffer *a_buffer)
    {
        // No exception handling is required inside this body because resources are freed as soon as
        // control enters this function member.

        m_device.waitIdle();
        image_base::destroy_resources();

        AHardwareBuffer_Desc buffer_desc;
        AHardwareBuffer_describe(a_buffer, &buffer_desc);

        if(buffer_desc.width * buffer_desc.height * buffer_desc.layers != m_data_size)
            throw std::runtime_error{"Data size differs. Cannot update image."};

        AndroidHardwareBufferFormatPropertiesANDROID format_info;
        AndroidHardwareBufferPropertiesANDROID properties_info;

        properties_info.pNext = &format_info;

        auto res = m_device.getAndroidHardwareBufferPropertiesANDROID(a_buffer, &properties_info);

        if(res != Result::eSuccess)
            throw std::runtime_error{"Result is: " + to_string(res) + "Couldn't get external buffer properties."};

        ExternalMemoryImageCreateInfo ext_mem_info;

        ext_mem_info.handleTypes = ExternalMemoryHandleTypeFlagBitsKHR::eAndroidHardwareBufferANDROID;

        ExternalFormatANDROID external_format;

        external_format.pNext = &ext_mem_info;

        ImageCreateInfo image_info;

        if(format_info.format == Format::eUndefined)
        {
            external_format.externalFormat = format_info.externalFormat;
            image_info.pNext = &external_format;
            image_info.format = Format::eUndefined;
        }
        else
        {
            image_info.pNext = &external_format;
            image_info.format = format_info.format;
        }

        image_info.flags = ImageCreateFlags{0};
        image_info.imageType = ImageType::e2D;
        image_info.extent = Extent3D{buffer_desc.width, buffer_desc.height, 1};
        image_info.mipLevels = 1;
        image_info.arrayLayers = buffer_desc.layers;
        image_info.samples = SampleCountFlagBits::e1;
        image_info.tiling = ImageTiling::eOptimal;
        image_info.usage = a_usage;
        image_info.sharingMode = a_sharing;
        image_info.queueFamilyIndexCount = 0;
        image_info.pQueueFamilyIndices = nullptr;
        image_info.initialLayout = ImageLayout::eUndefined;

        m_image = m_device.createImage(image_info);

        ImportAndroidHardwareBufferInfoANDROID import_info;

        import_info.buffer = a_buffer;

        MemoryDedicatedAllocateInfo mem_ded_info;

        mem_ded_info.pNext = &import_info;
        mem_ded_info.image = m_image;
        mem_ded_info.buffer = nullptr;

        MemoryAllocateInfo mem_info;

        mem_info.pNext = &mem_ded_info;
        mem_info.allocationSize = properties_info.allocationSize;

        m_size = properties_info.allocationSize;

        mem_info.memoryTypeIndex = get_memory_index(properties_info.memoryTypeBits,
            memory_location::external);

        m_memory = m_device.allocateMemory(mem_info);

        BindImageMemoryInfo bind_info;

        bind_info.image = m_image;
        bind_info.memory = m_memory;
        bind_info.memoryOffset = 0;

        m_device.bindImageMemory2KHR(bind_info);

        ImageMemoryRequirementsInfo2 mem_reqs_info;

        mem_reqs_info.image = m_image;

        MemoryDedicatedRequirements ded_mem_reqs;
        MemoryRequirements2 mem_reqs2;

        mem_reqs2.pNext = &ded_mem_reqs;

        m_device.getImageMemoryRequirements2KHR(&mem_reqs_info, &mem_reqs2);

        if(!ded_mem_reqs.prefersDedicatedAllocation || !ded_mem_reqs.requiresDedicatedAllocation)
            return;

        SamplerYcbcrConversionInfo conv_sampler_info;

        conv_sampler_info.conversion = m_conversion;

        ImageViewCreateInfo img_view_info;

        img_view_info.format = format_info.format;
        img_view_info.pNext = &conv_sampler_info;
        img_view_info.image = m_image;
        img_view_info.viewType = ImageViewType::e2D;
        img_view_info.components = {
                VK_COMPONENT_SWIZZLE_IDENTITY,
                VK_COMPONENT_SWIZZLE_IDENTITY,
                VK_COMPONENT_SWIZZLE_IDENTITY,
                VK_COMPONENT_SWIZZLE_IDENTITY
        };
        img_view_info.subresourceRange.aspectMask = ImageAspectFlagBits::eColor;
        img_view_info.subresourceRange.baseMipLevel = 0;
        img_view_info.subresourceRange.levelCount = 1;
        img_view_info.subresourceRange.baseArrayLayer = 0;
        img_view_info.subresourceRange.layerCount = 1;

        m_img_view = m_device.createImageView(img_view_info);
    }
    
    image<device>::image(const PhysicalDevice &a_gpu, const UniqueDevice &a_device,
        ImageUsageFlags a_usage, SharingMode a_sharing, const Extent2D &a_extent)
        : image_base{a_gpu, a_device}
    {
        ImageCreateInfo image_info;

        //image_info.flags
        image_info.imageType = ImageType::e2D;
        image_info.format = Format::eD32Sfloat;
        image_info.extent = Extent3D(a_extent, 1);
        image_info.mipLevels = 1;
        image_info.arrayLayers = 1;
        image_info.samples = SampleCountFlagBits::e1;
        image_info.tiling = ImageTiling::eOptimal;
        image_info.usage = a_usage;
        image_info.sharingMode = a_sharing;
        image_info.queueFamilyIndexCount = 0;
        image_info.pQueueFamilyIndices = nullptr;
        image_info.initialLayout = ImageLayout::eUndefined;

        m_image = m_device.createImage(image_info);

        auto mem_reqs = m_device.getImageMemoryRequirements(m_image);

        MemoryAllocateInfo mem_info;

        mem_info.memoryTypeIndex = get_memory_index(mem_reqs.memoryTypeBits, memory_location::device);
        mem_info.allocationSize = mem_reqs.size;

        m_size = mem_reqs.size;
        m_data_size = static_cast<size_t>(m_size);

        ImageViewCreateInfo img_view_info;

        img_view_info.image = m_image;
        img_view_info.viewType = ImageViewType::e2D;
        img_view_info.format = Format::eD32Sfloat;
        //img_view_info.components
        img_view_info.subresourceRange.aspectMask = ImageAspectFlagBits::eDepth;
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
}}