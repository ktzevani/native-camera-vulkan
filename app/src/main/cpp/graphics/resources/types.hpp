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

#ifndef NCV_RESOURCES_TYPES_HPP
#define NCV_RESOURCES_TYPES_HPP

namespace graphics{ namespace resources
{
    struct host;
    struct device_upload;
    struct device;
    struct external;

    template<typename T, typename S = void>
    class buffer;

    template<typename T, typename S = void>
    class image;
}}

#endif //NCV_RESOURCES_TYPES_HPP
