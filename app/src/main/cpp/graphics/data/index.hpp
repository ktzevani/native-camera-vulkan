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

#ifndef NCV_INDEX_HPP
#define NCV_INDEX_HPP

#include <graphics/data/types.hpp>
#include <vector>

namespace graphics { namespace data {
    static const std::vector<index_format> index_set = std::vector<index_format>
    {
        0, 1, 2, 2, 3, 0,
        7, 5, 4, 7, 6, 5,
        8, 9, 10, 10, 11, 8,
        12, 13, 14, 14, 15, 12,
        16, 17, 18, 18, 19, 16,
        20, 21, 22, 22, 23, 20
    };
}}

#endif //NCV_INDEX_HPP
