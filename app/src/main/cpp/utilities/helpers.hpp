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

#ifndef NCV_HELPERS_HPP
#define NCV_HELPERS_HPP

#include <string>

namespace utilities
{
    template<typename Char_t, typename Traits = std::char_traits<Char_t>>
    class center_helper
    {
    public:

        explicit center_helper(std::basic_string<Char_t, Traits> a_str) : m_str(a_str)
        {}

        template<typename _Char_t, typename _Traits>
        friend std::basic_ostream<_Char_t, _Traits> &operator<<(std::basic_ostream<_Char_t, _Traits>& a_s,
            const center_helper<_Char_t, _Traits>& a_c);

    private:
        std::basic_string<Char_t, Traits> m_str;
    };

    template<typename Char_t, typename Traits>
    inline std::basic_ostream<Char_t, Traits> &operator<<(std::basic_ostream<Char_t, Traits>& a_s,
        const center_helper<Char_t, Traits>& a_c)
    {
        auto w = a_s.width();

        if (w > a_c.m_str.length())
        {
            auto left = (w + a_c.m_str.length()) / 2;
            a_s.width(left);
            a_s << a_c.m_str;
            a_s.width(w - left);
            a_s << "";
        }
        else
            a_s << a_c.m_str;

        return a_s;
    }

    template<typename Char_t, typename Traits = std::char_traits<Char_t>>
    inline center_helper<Char_t, Traits> centered(std::basic_string<Char_t, Traits> str)
    {
        return center_helper<Char_t, Traits>(str);
    }

    static center_helper<std::string::value_type, std::string::traits_type>
    inline centered(const std::string& str)
    {
        return center_helper<std::string::value_type, std::string::traits_type>(str);
    }
}

#endif //NCV_HELPERS_HPP
