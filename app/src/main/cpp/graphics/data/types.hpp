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

#ifndef NCV_DATA_TYPES_HPP
#define NCV_DATA_TYPES_HPP

#include <cstdint>

namespace graphics { namespace data {
  typedef uint16_t index_format;
  class model_view_projection;
  class vertex_format;
  class texture;
  typedef unsigned char stbi_uc;
}}

#endif //NCV_DATA_TYPES_HPP
