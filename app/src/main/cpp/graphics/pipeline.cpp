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

#include <graphics/pipeline.hpp>
#include <graphics/data/vertex.hpp>
#include <android_native_app_glue.h>
#include <utilities/log.hpp>

using namespace ::std;
using namespace ::vk;
using namespace ::utilities;

namespace graphics
{
    pipeline::pipeline(const parameters &a_params, const shaders_info &a_shaders_info)
        : m_params{a_params}, m_shaders_info{a_shaders_info}
    {
        AAsset* file = AAssetManager_open(m_params.ass_mgr,
            ("shaders/" + m_shaders_info.vert_filename + ".spv").c_str(), AASSET_MODE_BUFFER);

        if(!file)
            throw runtime_error{"Unknown error. Couldn't open shader file."};

        vector<char> file_contents(AAsset_getLength(file));

        if(AAsset_read(file, file_contents.data(), file_contents.size()) != file_contents.size())
        {
            AAsset_close(file);
            throw runtime_error{"Unknown error. Couldn't load shader file contents."};
        }

        ShaderModuleCreateInfo shader_info;

        shader_info.codeSize = file_contents.size();
        shader_info.pCode = reinterpret_cast<const uint32_t*>(file_contents.data());

        m_vertex_shader = m_params.device.createShaderModule(shader_info);

        AAsset_close(file);

        file = AAssetManager_open(m_params.ass_mgr,
            ("shaders/" + m_shaders_info.frag_filename + ".spv").c_str(), AASSET_MODE_BUFFER);

        if(!file)
            throw runtime_error{"Unknown error. Couldn't open shader file."};

        file_contents.resize(AAsset_getLength(file));

        if(AAsset_read(file, file_contents.data(), file_contents.size()) != file_contents.size())
        {
            AAsset_close(file);
            throw runtime_error{"Unknown error. Couldn't load shader file contents."};
        }

        shader_info.codeSize = file_contents.size();
        shader_info.pCode = reinterpret_cast<const uint32_t*>(file_contents.data());

        m_fragment_shader = m_params.device.createShaderModule(shader_info);

        AAsset_close(file);

        if constexpr(__ncv_logging_enabled)
            _log_android(log_level::info) << "Shaders loaded with success, shader modules created.";

        // Shaders stage creation info

        PipelineShaderStageCreateInfo vertex_shader_stage_info;
        PipelineShaderStageCreateInfo fragment_shader_stage_info;

        vertex_shader_stage_info.stage = ShaderStageFlagBits::eVertex;
        vertex_shader_stage_info.module = m_vertex_shader;
        vertex_shader_stage_info.pName = "main";

        fragment_shader_stage_info.stage = ShaderStageFlagBits::eFragment;
        fragment_shader_stage_info.module = m_fragment_shader;
        fragment_shader_stage_info.pName = "main";

        vector<PipelineShaderStageCreateInfo> shader_stages_info;

        shader_stages_info.push_back(vertex_shader_stage_info);
        shader_stages_info.push_back(fragment_shader_stage_info);

        // Acquire vertex-related information (Input and Assembly)

        vector<VertexInputBindingDescription> bind_descs;
        vector<VertexInputAttributeDescription> attr_descs;

        VertexInputBindingDescription vertex_bind_desc;

        vertex_bind_desc.binding = 0;
        vertex_bind_desc.stride = sizeof(data::vertex_format);
        vertex_bind_desc.inputRate = VertexInputRate::eVertex;

        bind_descs.push_back(vertex_bind_desc);

        VertexInputAttributeDescription vertex_geom_desc;

        vertex_geom_desc.location = 0;
        vertex_geom_desc.binding = vertex_bind_desc.binding;
        vertex_geom_desc.format = Format::eR32G32B32A32Sfloat;
        vertex_geom_desc.offset = offsetof(data::vertex_format, position);

        VertexInputAttributeDescription vertex_color_desc;

        vertex_color_desc.location = 1;
        vertex_color_desc.binding = vertex_bind_desc.binding;
        vertex_color_desc.format = Format::eR32G32B32A32Sfloat;
        vertex_color_desc.offset = offsetof(data::vertex_format, color);

        VertexInputAttributeDescription vertex_tex_coords;

        vertex_tex_coords.location = 2;
        vertex_tex_coords.binding = vertex_bind_desc.binding;
        vertex_tex_coords.format = Format::eR32G32Sfloat;
        vertex_tex_coords.offset = offsetof(data::vertex_format, tex_coords);

        attr_descs.push_back(vertex_geom_desc);
        attr_descs.push_back(vertex_color_desc);
        attr_descs.push_back(vertex_tex_coords);

        PipelineVertexInputStateCreateInfo vertex_input_info;

        vertex_input_info.vertexBindingDescriptionCount = bind_descs.size();
        vertex_input_info.vertexAttributeDescriptionCount = attr_descs.size();
        vertex_input_info.pVertexBindingDescriptions = bind_descs.data();
        vertex_input_info.pVertexAttributeDescriptions = attr_descs.data();

        PipelineInputAssemblyStateCreateInfo vertex_assembly_info;

        vertex_assembly_info.topology = PrimitiveTopology::eTriangleList;
        vertex_assembly_info.primitiveRestartEnable = false;

        // Setting viewport and scissor information

        Viewport viewport;

        viewport.x = viewport.y = .0f;
        viewport.width = static_cast<float>(m_params.surface_extent.width);
        viewport.height = static_cast<float>(m_params.surface_extent.height);
        viewport.minDepth = .0f;
        viewport.maxDepth = 1.f;

        Rect2D scissor;

        scissor.offset = Offset2D{0, 0};
        scissor.extent = m_params.surface_extent;

        PipelineViewportStateCreateInfo viewport_info;

        viewport_info.viewportCount = 1;
        viewport_info.pViewports = &viewport;
        viewport_info.scissorCount = 1;
        viewport_info.pScissors = &scissor;

        // Setting up rasterizer

        PipelineRasterizationStateCreateInfo rasterizer_info;

        rasterizer_info.depthClampEnable = false;
        rasterizer_info.rasterizerDiscardEnable = false;
        rasterizer_info.polygonMode = PolygonMode::eFill;
        rasterizer_info.lineWidth = 1.f;
        rasterizer_info.cullMode = CullModeFlagBits::eBack;
        rasterizer_info.frontFace = FrontFace::eClockwise;
        rasterizer_info.depthBiasEnable = false;
        rasterizer_info.depthBiasConstantFactor = .0f;
        rasterizer_info.depthBiasClamp = .0f;
        rasterizer_info.depthBiasSlopeFactor = .0f;

        // Setup multi-sampling

        PipelineMultisampleStateCreateInfo multisampling_info;

        multisampling_info.sampleShadingEnable = false;
        multisampling_info.rasterizationSamples = SampleCountFlagBits::e1;
        multisampling_info.minSampleShading = 1.f;
        multisampling_info.pSampleMask = nullptr;
        multisampling_info.alphaToCoverageEnable = false;
        multisampling_info.alphaToOneEnable = false;

        // Setup depth and stencil testing

        PipelineDepthStencilStateCreateInfo depth_stencil_info;

        depth_stencil_info.depthTestEnable = true;
        depth_stencil_info.depthWriteEnable = true;
        depth_stencil_info.depthCompareOp = CompareOp::eLess;
        depth_stencil_info.depthBoundsTestEnable = false;
        depth_stencil_info.minDepthBounds = 0.0f;
        depth_stencil_info.maxDepthBounds = 1.0f;
        depth_stencil_info.stencilTestEnable = false;
        //depth_stencil_info.front = {};
        //depth_stencil_info.back = {};

        // Setup color blending state

        PipelineColorBlendAttachmentState color_blend_attachment;

        color_blend_attachment.colorWriteMask = ColorComponentFlagBits::eR | ColorComponentFlagBits::eG |
                                                ColorComponentFlagBits::eB | ColorComponentFlagBits::eA;
        color_blend_attachment.blendEnable = false;
        color_blend_attachment.srcColorBlendFactor = BlendFactor::eOne;
        color_blend_attachment.dstColorBlendFactor = BlendFactor::eZero;
        color_blend_attachment.colorBlendOp = BlendOp::eAdd;
        color_blend_attachment.srcAlphaBlendFactor = BlendFactor::eOne;
        color_blend_attachment.dstAlphaBlendFactor = BlendFactor::eZero;
        color_blend_attachment.alphaBlendOp = BlendOp::eAdd;

        PipelineColorBlendStateCreateInfo color_blend_info;

        color_blend_info.logicOpEnable = false;
        color_blend_info.logicOp = LogicOp::eCopy;
        color_blend_info.attachmentCount = 1;
        color_blend_info.pAttachments = &color_blend_attachment;
        color_blend_info.blendConstants[0.0f] = .0f;
        color_blend_info.blendConstants[1.0f] = .0f;
        color_blend_info.blendConstants[2.0f] = .0f;
        color_blend_info.blendConstants[3.0f] = .0f;

        // Setup dynamic state for pipeline

        // NO dynamic state is defined so just pass nullptr to pipeline creation

        // Setup pipeline layout

        DescriptorSetLayoutBinding ubo_layout_binding;

        ubo_layout_binding.binding = 0;
        ubo_layout_binding.descriptorType = DescriptorType::eUniformBuffer;
        ubo_layout_binding.descriptorCount = 1;
        ubo_layout_binding.stageFlags = ShaderStageFlagBits::eVertex;
        ubo_layout_binding.pImmutableSamplers = nullptr;

        array<DescriptorSetLayoutBinding, 1> sampler_binding;

        if(a_params.immut_sampler)
        {
            sampler_binding[0].binding = 1;
            sampler_binding[0].descriptorCount = 1;
            sampler_binding[0].descriptorType = DescriptorType::eCombinedImageSampler;
            sampler_binding[0].stageFlags = ShaderStageFlagBits::eFragment;
            sampler_binding[0].pImmutableSamplers = &a_params.immut_sampler;
        }
        else
        {
            sampler_binding[0].binding = 1;
            sampler_binding[0].descriptorCount = 1;
            sampler_binding[0].descriptorType = DescriptorType::eCombinedImageSampler;
            sampler_binding[0].stageFlags = ShaderStageFlagBits::eFragment;
            sampler_binding[0].pImmutableSamplers = nullptr;
        }

        array<DescriptorSetLayoutBinding, 2> desc_bindings = {
                ubo_layout_binding, sampler_binding[0]
        };

        DescriptorSetLayoutCreateInfo desc_set_layout_info;

        desc_set_layout_info.bindingCount = desc_bindings.size();
        desc_set_layout_info.pBindings = desc_bindings.data();

        m_desc_set_layout = m_params.device.createDescriptorSetLayout(desc_set_layout_info);

        PipelineLayoutCreateInfo pipeline_layout_info;

        pipeline_layout_info.setLayoutCount = 1;
        pipeline_layout_info.pSetLayouts = &m_desc_set_layout;
        pipeline_layout_info.pushConstantRangeCount = 0;
        pipeline_layout_info.pPushConstantRanges = nullptr;

        //-- Instantiate graphics pipeline with all of the above

        // Create pipeline layout

        m_pipeline_layout = m_params.device.createPipelineLayout(pipeline_layout_info);

        // Create graphics pipeline

        GraphicsPipelineCreateInfo graphics_pipeline_info;

        graphics_pipeline_info.stageCount = shader_stages_info.size();
        graphics_pipeline_info.pStages = shader_stages_info.data();
        graphics_pipeline_info.pVertexInputState = &vertex_input_info;
        graphics_pipeline_info.pInputAssemblyState = &vertex_assembly_info;
        graphics_pipeline_info.pViewportState = &viewport_info;
        graphics_pipeline_info.pRasterizationState = &rasterizer_info;
        graphics_pipeline_info.pMultisampleState = &multisampling_info;
        graphics_pipeline_info.pDepthStencilState = &depth_stencil_info;
        graphics_pipeline_info.pColorBlendState = &color_blend_info;
        graphics_pipeline_info.pDynamicState = nullptr;
        graphics_pipeline_info.layout = m_pipeline_layout;
        graphics_pipeline_info.renderPass = m_params.render_pass;
        graphics_pipeline_info.subpass = 0;
        graphics_pipeline_info.basePipelineHandle = nullptr;
        graphics_pipeline_info.basePipelineIndex = -1;

        auto call_result = m_params.device.createGraphicsPipeline(nullptr, graphics_pipeline_info);

        if(call_result.result != Result::eSuccess)
        {
            destroy_resources();
            throw runtime_error{"Result is: " + to_string(call_result.result) +
                " Couldn't create graphics pipeline."};
        }

        m_graphics_pipeline = call_result.value;
    }

    pipeline::~pipeline()
    {
        destroy_resources();
    }

    Pipeline& pipeline::get()
    {
        return m_graphics_pipeline;
    }

    DescriptorSetLayout& pipeline::get_desc_set()
    {
        return m_desc_set_layout;
    }

    PipelineLayout& pipeline::get_layout()
    {
        return m_pipeline_layout;
    }

    void pipeline::destroy_resources() noexcept
    {
        m_params.device.destroyDescriptorSetLayout(m_desc_set_layout);
        m_params.device.destroyPipelineLayout(m_pipeline_layout);
        m_params.device.destroyPipeline(m_graphics_pipeline);
        m_params.device.destroyShaderModule(m_fragment_shader);
        m_params.device.destroyShaderModule(m_vertex_shader);
    }
}