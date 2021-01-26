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

#ifndef NCV_VULKAN_HPP
#define NCV_VULKAN_HPP

#include <engine/generic.hpp>
#include <core/android_permissions.hpp>
#include <vk_util/vk_helpers.hpp>
#include <devices/accelerometer.hpp>
#include <devices/image_reader.hpp>
#include <devices/camera.hpp>
#include <android_native_app_glue.h>
#include <unistd.h>

namespace graphics
{
    class simple_context;
    class complex_context;
}

namespace engine
{
    template<typename T>
    class vulkan: public generic<vulkan<T>>
    {
    public:

        typedef struct
        {
            glm::vec<2, uint32_t> screen_size;
            glm::vec2 touch_pos;
            glm::vec4 back_color;
        } data;

        vulkan(android_app* a_app, const std::string& a_package_name, const std::string& a_app_name,
            uint32_t a_app_version);
        ~vulkan();
        void process_devices_input();
        void process_display();
        static void _app_cmd_handler(android_app* a_app, int32_t a_cmd);
        static int32_t _app_input_handler(android_app* a_app, AInputEvent* a_event);

    private:

        void start_engine();
        void stop_engine() noexcept;

        static std::chrono::time_point<std::chrono::high_resolution_clock> m_time_pt;
        uint64_t m_frames_processed = 0;

        data m_state = {{0u, 0u}, {0.f, 0.f}, {1.f, 1.f, 1.f, 1.f}};

        T m_context;

        std::shared_ptr<::devices::accelerometer> m_accelerometer;
        std::shared_ptr<::devices::image_reader> m_img_reader;
        std::shared_ptr<::devices::camera> m_camera;
    };

    template<typename T>
    inline vulkan<T>::vulkan(android_app *a_app, const std::string &a_package_name,
        const std::string &a_app_name, uint32_t a_app_version)
        : generic<vulkan>{a_package_name, a_app_name, a_app_version},
          m_context{a_app_name}
    {
        // Register application context both ways

        this->m_app = a_app;
        a_app->userData = this;

        if constexpr(__ncv_logging_enabled)
            _log_android(::utilities::log_level::info) << "Vulkan engine has been initialized.";
    }

    template<typename T>
    inline vulkan<T>::~vulkan()
    {
        if constexpr(__ncv_logging_enabled)
            _log_android(::utilities::log_level::info) << "Destroying vulkan engine...";
    }

    template<typename T>
    inline void vulkan<T>::process_devices_input()
    {
        using ::devices::accelerometer;

        if(!this->m_rendering)
            return;

        int32_t c = floorf(((m_state.touch_pos.x * m_state.touch_pos.y) /
                            (m_state.screen_size.s * m_state.screen_size.t)) * 65535.f);

        c = c > 0xffff ? 0xffff : c;

        m_state.back_color = {
            ((c & 0xff00) >> 8) / 255.f,
            (c & 0xff) / 255.f,
            powf((accelerometer::g - fabsf(m_accelerometer->get_acceleration().y)) / accelerometer::g, 2.0f),
            1.f
        };

        if constexpr(__ncv_logging_enabled)
            _log_android(::utilities::log_level::verbose) << "Devices input has been processed.";
    }

    template<typename T>
    inline void vulkan<T>::process_display()
    {
        using std::chrono::nanoseconds;
        using std::chrono::high_resolution_clock;
        using std::chrono::duration_cast;

        if constexpr(__ncv_profiling_enabled)
        {
            if constexpr(std::is_same<decltype(m_context), ::graphics::complex_context>())
                if(this->m_cam_permission)
                    m_context.render_frame(m_state.back_color, m_img_reader->get_latest_buffer());
                else
                    m_context.render_frame(m_state.back_color);
            else if constexpr(std::is_same<decltype(m_context), ::graphics::simple_context>())
                m_context.render_frame(m_state.back_color);
            m_frames_processed++;
            auto ns = duration_cast<nanoseconds>(high_resolution_clock::now() - m_time_pt);
            _log_android(::utilities::log_level::verbose) << "Rate of processed frames: "
                << m_frames_processed / (ns.count() / 1e+9) << " Hz";
        }
        else
        {
            if constexpr(std::is_same<decltype(m_context), ::graphics::complex_context>())
                if(this->m_cam_permission)
                    m_context.render_frame(m_state.back_color, m_img_reader->get_latest_buffer());
                else
                    m_context.render_frame(m_state.back_color);
            else if constexpr(std::is_same<decltype(m_context), ::graphics::simple_context>())
                m_context.render_frame(m_state.back_color);
        }

        if constexpr(__ncv_logging_enabled)
            _log_android(::utilities::log_level::verbose) << "Display has been processed.";
    }

    template<typename T>
    inline void vulkan<T>::_app_cmd_handler(android_app *a_app, int32_t a_cmd)
    {
        vulkan *engine = reinterpret_cast<vulkan*>(a_app->userData);
        ::core::android_permissions permissions;

        switch (a_cmd)
        {
            case APP_CMD_SAVE_STATE:
                engine->m_app->savedState = malloc(sizeof(data));
                *reinterpret_cast<data*>(engine->m_app->savedState) = engine->m_state;
                engine->m_app->savedStateSize = sizeof(data);
                break;
            case APP_CMD_INIT_WINDOW:
                if(engine->m_state.screen_size.s == 0u)
                    engine->m_state.screen_size = {
                        ANativeWindow_getWidth(engine->m_app->window),
                        ANativeWindow_getHeight(engine->m_app->window)
                    };
                break;
            case APP_CMD_GAINED_FOCUS:
                engine->m_cam_permission = permissions.is_camera_permitted(engine->m_app);
                engine->start_engine();
                break;
            case APP_CMD_TERM_WINDOW:
                engine->stop_engine();
                break;
            default:
                break;
        }

        if constexpr(__ncv_logging_enabled)
            _log_android(::utilities::log_level::verbose)
                << "Application command (code: " << a_cmd << ") is processed.";
    }

    template<typename T>
    inline int32_t vulkan<T>::_app_input_handler(android_app *a_app, AInputEvent *a_event)
    {
        vulkan *engine = reinterpret_cast<vulkan*>(a_app->userData);
        int32_t result = 0;
        auto e_type = AInputEvent_getType(a_event);

        if (e_type == AINPUT_EVENT_TYPE_MOTION)
        {
            engine->m_state.touch_pos = {
                AMotionEvent_getX(a_event, 0),
                AMotionEvent_getY(a_event, 0)
            };

            result = 1;
        }

        if constexpr(__ncv_logging_enabled)
        {
            if (e_type != AINPUT_EVENT_TYPE_MOTION)
                _log_android(::utilities::log_level::verbose) << "Input processed (event: "
                    << e_type << ")";
            else
                _log_android(::utilities::log_level::verbose)
                    << "Input processed (event: " << e_type << ", " << " data: "
                    << "{x=" << engine->m_state.touch_pos.x << ", y="
                    << engine->m_state.touch_pos.y << "}).";
        }

        return result;
    }

    template<typename T>
    inline void vulkan<T>::start_engine()
    {
        using namespace ::devices;

        // Initialize camera and image reader

        m_accelerometer = std::make_shared<accelerometer>(this->m_app, this->m_package_name);

        if constexpr(std::is_same<decltype(m_context), ::graphics::complex_context>())
        {
            if(this->m_cam_permission)
            {
                m_img_reader = std::make_shared<image_reader>(m_state.screen_size.s, m_state.screen_size.t,
                    AIMAGE_FORMAT_PRIVATE, AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE, 4);
                m_camera = std::make_shared<camera>(m_img_reader->get_window());
                m_camera->start_capturing();

                while (!m_img_reader->get_latest_buffer())
                    usleep(10000u);
            }
        }

        // Reset framerate counters

        if constexpr(__ncv_profiling_enabled)
        {
            m_time_pt = std::chrono::high_resolution_clock::now();
            m_frames_processed = 0;
        }

        // Initialize graphics

        if constexpr(std::is_same<decltype(m_context), ::graphics::complex_context>())
        {
            if(this->m_cam_permission)
                m_context.initialize_graphics(this->m_app, m_img_reader->get_latest_buffer());
            else
                m_context.initialize_graphics(this->m_app);
        }
        else if constexpr(std::is_same<decltype(m_context), ::graphics::simple_context>())
        {
            m_context.initialize_graphics(this->m_app);
        }

        this->m_rendering = true;

        m_accelerometer->enable();

        if constexpr(__ncv_logging_enabled)
            _log_android(::utilities::log_level::info) << "Vulkan engine has been started.";
    }

    template<typename T>
    inline void vulkan<T>::stop_engine() noexcept
    {
        if constexpr(std::is_same<decltype(m_context), ::graphics::complex_context>())
            if(this->m_cam_permission)
                m_camera->stop_capturing();
        this->m_rendering = false;
        m_accelerometer->disable();
        if constexpr(std::is_same<decltype(m_context), ::graphics::complex_context>())
        {
            m_camera.reset();
            m_img_reader.reset();
        }
        m_accelerometer.reset();
        if constexpr(__ncv_logging_enabled)
            _log_android(::utilities::log_level::info) << "Vulkan engine has been stopped.";
        return;
    }

    template<typename T>
    std::chrono::time_point<std::chrono::high_resolution_clock> vulkan<T>::m_time_pt =
        std::chrono::high_resolution_clock::now();
}

#endif //NCV_VULKAN_HPP
