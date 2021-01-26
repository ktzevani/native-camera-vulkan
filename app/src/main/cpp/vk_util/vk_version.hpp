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

#ifndef NCV_VK_VERSION_HPP
#define NCV_VK_VERSION_HPP

#include <sstream>

namespace vk_util
{
    struct version
    {
        version() : major{0}, minor{0}, patch{0}
        {}

        version(uint32_t version) : ::vk_util::version{}
        {
            *this = version;
        }

        version& operator=(uint32_t version)
        {
            memcpy(this, &version, sizeof(uint32_t));
            return *this;
        }

        operator uint32_t() const
        {
            uint32_t result;
            memcpy(&result, this, sizeof(uint32_t));
            return result;
        }

        bool operator>=(const version &other) const
        {
            return (operator uint32_t()) >= (other.operator uint32_t());
        }

        std::string to_string() const
        {
            std::stringstream buffer;
            buffer << major << "." << minor << "." << patch;
            return buffer.str();
        }

        const uint32_t patch: 12;
        const uint32_t minor: 10;
        const uint32_t major: 10;
    };

}

#endif //NCV_VK_VERSION_HPP
