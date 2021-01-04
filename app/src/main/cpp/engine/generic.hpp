#ifndef NCV_GENERIC_HPP
#define NCV_GENERIC_HPP

#include <utilities/log.hpp>

class android_app;
class AInputEvent;

namespace engine
{
    template <typename T>
    class generic
    {
    public:

        generic(const std::string& a_package_name, const std::string& a_app_name, uint32_t a_app_version)
            : m_package_name{a_package_name}, m_app_name{a_app_name}, m_app_version{a_app_version}
        {
            if constexpr(__ncv_logging_enabled)
            {
                _log_android(::utilities::log_level::debug) << "Generic engine has been initialized.";
            }
        }

        virtual ~generic()
        {
            if constexpr(__ncv_logging_enabled)
                _log_android(::utilities::log_level::debug) << "Destroying generic engine...";
        };

        virtual void process_devices_input() = 0;
        virtual void process_display() = 0;

        bool is_rendering() const
        {
            return m_rendering;
        }

        static void app_cmd_handler(android_app* a_app, int32_t a_cmd)
        {
            T::_app_cmd_handler(a_app, a_cmd);
        }

        static int32_t app_input_handler(android_app* a_app, AInputEvent* a_event)
        {
            return T::_app_input_handler(a_app, a_event);
        }

    protected:

        virtual void start_engine() = 0;
        virtual void stop_engine() = 0;

        const std::string m_package_name, m_app_name;
        const uint32_t m_app_version;

        android_app* m_app = nullptr;
        bool m_rendering = false;
        bool m_cam_permission = false;
    };

}

#endif //NCV_GENERIC_HPP
