#ifndef NCV_ACCELEROMETER_HPP
#define NCV_ACCELEROMETER_HPP

#include <functional>
#include <glm/glm.hpp>

class ASensorEventQueue;
class ASensorManager;
class ASensor;
class android_app;

namespace devices
{
    class accelerometer
    {
    public:

        using ASensorEventQueue_ptr = std::unique_ptr<ASensorEventQueue, std::function<void(ASensorEventQueue*)>>;

        constexpr static float g = 9.81f;
        constexpr static uint32_t max_rate = 120u;

        explicit accelerometer(android_app* a_app, const std::string& a_package_name);
        ~accelerometer();

        void disable();
        void enable(int32_t a_rate = 60);
        glm::vec3 get_acceleration();

    private:

        ASensorEventQueue_ptr m_queue;
        ASensorManager* m_manager = nullptr;
        const ASensor* m_sensor = nullptr;

        uint32_t m_rate = 0;
    };
}

#endif //NCV_ACCELEROMETER_HPP
