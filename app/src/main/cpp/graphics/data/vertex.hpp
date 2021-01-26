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

#ifndef NCV_VERTEX_HPP
#define NCV_VERTEX_HPP

#include <glm/glm.hpp>
#include <vector>

namespace graphics{ namespace data{
    struct vertex_format
    {
        glm::vec4 position;
        glm::vec4 color;
        glm::vec2 tex_coords;
    };

    static const std::vector<vertex_format> vertex_set = std::vector<vertex_format>
    {
        {{-1.0f, 1.0f,  1.0f, 1.0f}, {1.f,  0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
        {{1.0f, 1.0f,  1.0f, 1.0f}, {0.0f, 0.0f, 1.0f, 1.0f}, {0.0f, 0.0f}},
        {{1.0f, -1.0f, 1.0f, 1.0f}, {0.0f, 1.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
        {{-1.0f, -1.0f, 1.0f, 1.0f}, {1.0f, 1.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},

        {{-1.0f, 1.0f,  -1.0f, 1.0f}, {1.f,  0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
        {{1.0f, 1.0f,  -1.0f, 1.0f}, {0.0f, 0.0f, 1.0f, 1.0f}, {1.0f, 0.0f}},
        {{1.0f, -1.0f, -1.0f, 1.0f}, {0.0f, 1.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
        {{-1.0f, -1.0f, -1.0f, 1.0f}, {1.0f, 1.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},

        {{1.0f, 1.0f,  1.0f, 1.0f}, {0.0f, 0.0f, 1.0f, 1.0f}, {1.0f, 0.0f}},
        {{1.0f, 1.0f,  -1.0f, 1.0f}, {0.0f, 0.0f, 1.0f, 1.0f}, {0.0f, 0.0f}},
        {{1.0f, -1.0f, -1.0f, 1.0f}, {0.0f, 1.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
        {{1.0f, -1.0f, 1.0f, 1.0f}, {0.0f, 1.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},

        {{-1.0f, 1.0f,  -1.0f, 1.0f}, {1.f,  0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
        {{-1.0f, 1.0f,  1.0f, 1.0f}, {1.f,  0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
        {{-1.0f, -1.0f, 1.0f, 1.0f}, {1.0f, 1.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
        {{-1.0f, -1.0f, -1.0f, 1.0f}, {1.0f, 1.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},

        {{-1.0f, 1.0f,  -1.0f, 1.0f}, {1.f,  0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
        {{1.0f, 1.0f,  -1.0f, 1.0f}, {0.0f, 0.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},
        {{1.0f, 1.0f,  1.0f, 1.0f}, {0.0f, 0.0f, 1.0f, 1.0f}, {1.0f, 1.0f}},
        {{-1.0f, 1.0f,  1.0f, 1.0f}, {1.f,  0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},

        {{-1.0f, -1.0f, 1.0f, 1.0f}, {1.0f, 1.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
        {{1.0f, -1.0f, 1.0f, 1.0f}, {0.0f, 1.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
        {{1.0f, -1.0f, -1.0f, 1.0f}, {0.0f, 1.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
        {{-1.0f, -1.0f, -1.0f, 1.0f}, {1.0f, 1.0f, 0.0f, 1.0f}, {1.0f, 0.0f}}
    };
}}

#endif //NCV_VERTEX_HPP
