#include <devices/accelerometer.hpp>
#include <utilities/log.hpp>
#include <android_native_app_glue.h>
#include <android/sensor.h>
#include <array>

using namespace ::std;
using namespace ::utilities;

namespace devices
{
    accelerometer::accelerometer(android_app *a_app, const string &a_package_name)
        : m_queue{nullptr, [&](ASensorEventQueue* pt){
            ASensorManager_destroyEventQueue(m_manager, pt);
        }}
    {
        m_manager = ASensorManager_getInstanceForPackage(a_package_name.c_str());
        m_sensor = ASensorManager_getDefaultSensor(m_manager, ASENSOR_TYPE_ACCELEROMETER);
        m_queue.reset(ASensorManager_createEventQueue(m_manager, a_app->looper, LOOPER_ID_USER, nullptr, nullptr));

        if constexpr(__ncv_logging_enabled)
            _log_android(log_level::info) << "Logical accelerometer created.";
    }

    accelerometer::~accelerometer()
    {
        if constexpr(__ncv_logging_enabled)
            _log_android(log_level::info) << "Destroying accelerometer sensor...";
    }

    void accelerometer::disable()
    {
        m_rate = 0;
        ASensorEventQueue_disableSensor(m_queue.get(), m_sensor);
    }

    void accelerometer::enable(int32_t a_rate)
    {
        m_rate = a_rate < 1 ? 1 : (a_rate > max_rate ? max_rate : a_rate);
        ASensorEventQueue_enableSensor(m_queue.get(), m_sensor);
        ASensorEventQueue_setEventRate(m_queue.get(), m_sensor, 10e6L / a_rate);
    }

    glm::vec3 accelerometer::get_acceleration()
    {
        array<ASensorEvent, max_rate - 1> events;
        int e_count = -1;

        if(m_rate < 1)
            return {0.f, 0.f, 0.f};

        while((e_count = ASensorEventQueue_getEvents(m_queue.get(), events.data(), max_rate - 1)) > 0)
            if(e_count < max_rate - 1)
                break;

        if(e_count-- < 1)
            return {0.f, 0.f, 0.f};

        if constexpr(__ncv_logging_enabled)
            _log_android(log_level::verbose)
                    << "Accelerometer: x=" << events[e_count].acceleration.x
                    << " y=" << events[e_count].acceleration.y
                    << " z=" << events[e_count].acceleration.z;

        return glm::vec3{
                events[e_count].acceleration.x,
                events[e_count].acceleration.y,
                events[e_count].acceleration.z
        };
    }
}