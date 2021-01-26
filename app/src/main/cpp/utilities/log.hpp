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

#ifndef NCV_LOG_HPP
#define NCV_LOG_HPP

#include <thread>
#include <vector>
#include <iostream>
#include <sstream>
#include <fstream>
#include <android/log.h>

#ifdef NCV_LOGGING_ENABLED
constexpr bool __ncv_logging_enabled = true;
#else
constexpr bool __ncv_logging_enabled = false;
#endif

constexpr const char* TAG = "NativeActivity";

namespace utilities
{
    enum class log_level
    {
        silent,
        unknown,
        fatal,
        error,
        warning,
        info,
        debug,
        verbose,
        trace_0,
        trace_1,
        trace_2,
        all
    };
}

extern utilities::log_level __ncv_log_level;

namespace utilities
{
    struct log_2_android_console;

    struct log_2_clog;

    struct log_2_cerr;

    struct log_2_cout;

    struct log_2_file;

    class log_base
    {
    public:

        explicit log_base(log_level a_loglevel = log_level::error) : m_level{a_loglevel}
        {
            m_buffer << "Thread ID: " << std::hex << std::this_thread::get_id() << std::dec << " - ";

            switch(a_loglevel)
            {
                case log_level::unknown: m_buffer << "UNKNOWN: "; break;
                case log_level::fatal:   m_buffer << "FATAL: ";   break;
                case log_level::error:   m_buffer << "ERROR: ";   break;
                case log_level::warning: m_buffer << "WARNING: "; break;
                case log_level::info:    m_buffer << "INFO: ";    break;
                case log_level::debug:   m_buffer << "DEBUG: ";   break;
                case log_level::verbose: m_buffer << "VERBOSE: "; break;
                case log_level::trace_0: m_buffer << "TRACE-0: "; break;
                case log_level::trace_1: m_buffer << "TRACE-1: "; break;
                case log_level::trace_2: m_buffer << "TRACE-2: "; break;
                case log_level::all:     m_buffer << "ALL: ";     break;
                default:                                          break;
            }
        }

        template<typename T>
        log_base& operator<<(T const& a_val)
        {
            m_buffer << a_val;
            return *this;
        }

        template<typename T>
        log_base& operator<<(std::vector<T> const& a_val)
        {
            m_buffer << '[';
            if(!a_val.empty())
                copy(a_val.begin(), a_val.end(), std::ostream_iterator<T>(m_buffer, ", "));
            m_buffer << ']';
            return *this;
        }

        virtual ~log_base()
        {}

    protected:

        std::ostringstream m_buffer;
        log_level m_level;
    };

    template<class BasePolicy>
    class log : public log_base
    {};

    template<>
    class log<log_2_clog> : public log_base
    {
    public:

        log(log_level a_level = log_level::error) : log_base{a_level}
        {}

        ~log()
        {
            m_buffer << '\n';
            if(m_level != log_level::silent)
                std::clog << m_buffer.str();
        }
    };

    template<>
    class log<log_2_cerr> : public log_base
    {
    public:

        log(log_level a_level = log_level::error) : log_base{a_level}
        {}

        ~log()
        {
            m_buffer << '\n';
            if(m_level != log_level::silent)
                std::cerr << m_buffer.str();
        }
    };

    template<>
    class log<log_2_cout> : public log_base
    {
    public:

        log(log_level a_level = log_level::error) : log_base{a_level}
        {}

        ~log()
        {
            m_buffer << '\n';
            if(m_level != log_level::silent)
                std::cout << m_buffer.str();
        }
    };

    template<>
    class log<log_2_file> : public log_base
    {
    public:

        log(std::ofstream& a_filestream, log_level a_level = log_level::error)
            : log_base{a_level}, m_filestream{a_filestream}
        {}

        ~log()
        {
            m_buffer << '\n';
            if(m_level != log_level::silent)
                m_filestream << m_buffer.str();
        }

    private:

        std::ofstream& m_filestream;
    };

    template<>
    class log<log_2_android_console> : public log_base
    {
    public:
        
        log(log_level a_level = log_level::error) : log_base{a_level}
        {}
        
        ~log()
        {
            m_buffer << "\n";
            switch(m_level)
            {
                case log_level::error:
                    __android_log_print(ANDROID_LOG_ERROR, TAG, "%s", m_buffer.str().c_str());
                    break;
                case log_level::warning:
                    __android_log_print(ANDROID_LOG_WARN, TAG, "%s", m_buffer.str().c_str());
                    break;
                case log_level::info:
                    __android_log_print(ANDROID_LOG_INFO, TAG, "%s", m_buffer.str().c_str());
                    break;
                case log_level::debug:
                    __android_log_print(ANDROID_LOG_DEBUG, TAG, "%s", m_buffer.str().c_str());
                    break;
                case log_level::verbose:
                    __android_log_print(ANDROID_LOG_VERBOSE, TAG, "%s", m_buffer.str().c_str());
                    break;
                default:
                    break;
            }
        }
    };

    template<typename T>
    inline static log<T> log_to_sarg(log_level a_level)
    {
        if(static_cast<int>(a_level) <= static_cast<int>(::__ncv_log_level))
            return log<T>(a_level);
        else
            return log<T>(log_level::silent);
    }

    template<typename T1, typename T2>
    inline static log<T1> log_to_marg(log_level a_level, T2& a_arg)
    {
        if(static_cast<int>(a_level) <= static_cast<int>(::__ncv_log_level))
            return log<T1>(a_arg, a_level);
        else
            return log<T1>(a_arg, log_level::silent);
    }

    inline static log<log_2_android_console> _log_android(log_level a_level)
    {
        return log_to_sarg<log_2_android_console>(a_level);
    }

    inline static log<log_2_file> _log_file(log_level a_level, std::ofstream& a_fout)
    {
        return log_to_marg<log_2_file, std::ofstream>(a_level, a_fout);
    }

    inline static log<log_2_clog> _log_clog(log_level a_level)
    {
        return log_to_sarg<log_2_clog>(a_level);
    }

    inline static log<log_2_cerr> _log_cerr(log_level a_level)
    {
        return log_to_sarg<log_2_cerr>(a_level);
    }

    inline static log<log_2_cout> _log_cout(log_level a_level)
    {
        return log_to_sarg<log_2_cout>(a_level);
    }
}

#endif //NCV_LOG_HPP