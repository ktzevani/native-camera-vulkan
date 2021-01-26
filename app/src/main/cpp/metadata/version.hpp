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

#ifndef NCV_VERSION_HPP
#define NCV_VERSION_HPP

#include <string>
#include <sstream>

extern const int PROJECT_VERSION_MAJOR;
extern const int PROJECT_VERSION_MINOR;
extern const int PROJECT_VERSION_PATCH;
extern const int PROJECT_VERSION_BUILD_NUMBER;
extern const int PROJECT_BUILD_NUMBER;

namespace metadata
{
    struct
    {
        const int major = PROJECT_VERSION_MAJOR;
        const int minor = PROJECT_VERSION_MINOR;
        const int patch = PROJECT_VERSION_PATCH;
        const int build_number = PROJECT_VERSION_BUILD_NUMBER;
        const int total_builds = PROJECT_BUILD_NUMBER;

        std::string full_version() const
        {
            std::ostringstream sstr;
            sstr << this->major << "."
                 << this->minor << "."
                 << this->patch << "-"
                 << this->build_number << " (build."
                 << this->total_builds << ")";
            return sstr.str();
        }

        std::string semver() const
        {
            std::ostringstream sstr;
            sstr << this->major << "."
                 << this->minor << "."
                 << this->patch;
            return sstr.str();
        }

        uint32_t packed_version() const
        {
            return static_cast<uint32_t>(
                   PROJECT_VERSION_MAJOR << 22 |
                   PROJECT_VERSION_MINOR << 12 |
                  (PROJECT_VERSION_PATCH & 0xfff));
        }

    } PROJECT_VERSION;
}

#endif //NCV_VERSION_HPP
