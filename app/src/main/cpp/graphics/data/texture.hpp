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

#ifndef NCV_TEXTURE_HPP
#define NCV_TEXTURE_HPP

#include <string>
#include <vector>

class AAssetManager;

namespace graphics{ namespace data{

    class texture
    {
    public:
        typedef unsigned char stbi_uc;
        typedef struct
        {
            uint32_t width;
            uint32_t height;
            uint32_t depth;
        } extent3d;

        texture(AAssetManager* a_ass_mgr, const std::string& a_filename);
        std::vector<stbi_uc>& get_data() { return m_data; }
        extent3d get_extent() { return m_extent; }
    private:
        std::vector<stbi_uc> m_data;
        extent3d m_extent;
    };

}}

#endif //NCV_TEXTURE_HPP
