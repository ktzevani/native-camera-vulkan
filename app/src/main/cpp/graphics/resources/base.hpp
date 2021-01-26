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

#ifndef NCV_RESOURCES_BASE_HPP
#define NCV_RESOURCES_BASE_HPP

#include <vulkan_hpp/vulkan.hpp>

namespace graphics{ namespace resources{

    class base
    {
    public:

        enum class memory_location
        {
            device,
            host,
            external
        };

        base(const vk::PhysicalDevice& a_gpu, const vk::UniqueDevice& a_device);

        size_t data_size() { return m_data_size; }

    protected:

        int get_memory_index(uint32_t a_memory_type_bits, memory_location a_location) noexcept;

        vk::Device m_device = nullptr;
        vk::PhysicalDeviceMemoryProperties m_mem_props;
        size_t m_data_size = 0;
        vk::DeviceSize m_size = 0;
    };

}}

#endif //NCV_RESOURCES_BASE_HPP
