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
