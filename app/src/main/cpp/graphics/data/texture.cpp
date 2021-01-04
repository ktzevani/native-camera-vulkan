#include <graphics/data/texture.hpp>

#define STB_IMAGE_IMPLEMENTATION

#include <stb/stb_image.h>
#include <android_native_app_glue.h>

namespace graphics{ namespace data{

    texture::texture(AAssetManager *a_ass_mgr, const std::string &a_filename)
    {
        AAsset* file = AAssetManager_open(a_ass_mgr, a_filename.c_str(), AASSET_MODE_BUFFER);

        if(!file)
            throw std::runtime_error{"Unknown error. Couldn't open texture file."};

        auto file_data = std::vector<stbi_uc>(AAsset_getLength(file));

        if(AAsset_read(file, file_data.data(), file_data.size()) != file_data.size())
        {
            AAsset_close(file);
            throw std::runtime_error{"Unknown error. Couldn't read from texture file."};
        }

        AAsset_close(file);

        int tex_width, tex_height, tex_channels;

        auto cvt_data = stbi_load_from_memory(file_data.data(), file_data.size(), &tex_height, &tex_width,
            &tex_channels, STBI_rgb_alpha);

        if(!cvt_data)
            throw std::runtime_error{"Could not convert loaded texture."};

        m_extent = {static_cast<uint32_t>(tex_width), static_cast<uint32_t>(tex_height), 1};

        m_data.resize(tex_width * tex_height * 4);

        memcpy(m_data.data(), cvt_data, m_data.size());

        stbi_image_free(cvt_data);
    }

}}