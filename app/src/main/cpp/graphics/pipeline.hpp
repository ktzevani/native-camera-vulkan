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
