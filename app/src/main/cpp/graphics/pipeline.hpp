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

#ifndef NCV_GRAPHICS_PIPELINE_HPP
#define NCV_GRAPHICS_PIPELINE_HPP

#include <vulkan_hpp/vulkan.hpp>

class AAssetManager;

namespace graphics
{
    class pipeline
    {
    public:
        
        struct shaders_info
        {
            const std::string vert_filename;
            const std::string frag_filename;
        };

        struct parameters
        {
            AAssetManager* ass_mgr;
            vk::Device device;
            vk::Extent2D surface_extent;
            vk::RenderPass render_pass;
            vk::Sampler immut_sampler;
        };

        pipeline(const parameters& a_params, const shaders_info& a_shaders_info);

        ~pipeline();

        vk::Pipeline& get();
        vk::DescriptorSetLayout& get_desc_set();
        vk::PipelineLayout& get_layout();

    private:

        void destroy_resources() noexcept;

        parameters m_params;
        shaders_info m_shaders_info;

        vk::ShaderModule m_vertex_shader = nullptr;
        vk::ShaderModule m_fragment_shader = nullptr;
        vk::DescriptorSetLayout m_desc_set_layout = nullptr;
        vk::PipelineLayout m_pipeline_layout = nullptr;
        vk::Pipeline m_graphics_pipeline = nullptr;
    };

}

#endif //NCV_GRAPHICS_PIPELINE_HPP
