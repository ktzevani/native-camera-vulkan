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