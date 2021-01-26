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

#ifndef NCV_EVENT_LOOP_HPP
#define NCV_EVENT_LOOP_HPP

#include <android_native_app_glue.h>
#include <utilities/log.hpp>

namespace core
{
    template <typename T>
    class event_loop
    {
    public:

        event_loop(android_app* a_app, std::shared_ptr<T>& a_engine) :
            m_app{a_app}, m_engine{a_engine}
        {
            if constexpr(__ncv_logging_enabled)
                _log_android(::utilities::log_level::info) << "Event loop is created.";
        }

        ~event_loop()
        {
            if constexpr(__ncv_logging_enabled)
                _log_android(::utilities::log_level::info) << "Destroying event loop...";
        }

        void run()
        {
            int32_t result;
            android_poll_source* source;

            m_app->onAppCmd = T::app_cmd_handler;
            m_app->onInputEvent = T::app_input_handler;

            while(true)
            {
                while((result = ALooper_pollAll(m_engine->is_rendering() ? 0 : -1, nullptr, nullptr,
                        reinterpret_cast<void**>(&source))) >= 0 )
                {
                    if(source != nullptr)
                        source->process(m_app, source);

                    if(result == LOOPER_ID_USER)
                    {
                        if constexpr(__ncv_logging_enabled)
                            _log_android(::utilities::log_level::verbose) << "Processing devices input...";
                        m_engine->process_devices_input();
                    }
                    if(m_app->destroyRequested)
                    {
                        if constexpr(__ncv_logging_enabled)
                            _log_android(::utilities::log_level::info) << "Terminating event loop...";
                        return;
                    }
                }
                if(m_engine->is_rendering())
                {
                    if constexpr(__ncv_logging_enabled)
                        _log_android(::utilities::log_level::verbose) << "Processing application display...";
                    m_engine->process_display();
                }
            }
        }

    private:

        android_app* m_app = nullptr;
        std::shared_ptr<T> m_engine;
    };

}

#endif //NCV_EVENT_LOOP_HPP