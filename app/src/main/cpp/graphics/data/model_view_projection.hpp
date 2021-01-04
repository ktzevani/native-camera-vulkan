#ifndef NCV_MODEL_VIEW_PROJECTION_HPP
#define NCV_MODEL_VIEW_PROJECTION_HPP

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <chrono>

namespace graphics{ namespace data{
    struct model_view_projection
    {
        model_view_projection(float a_screen_ratio)
        {
            m_model = rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 1.0f)) *
                    scale(glm::mat4(1.0f), glm::vec3(0.325f, 0.325f, 0.325f));
            m_view = lookAt(glm::vec3(0.0f, 0.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f),
                    glm::vec3(0.0f, 1.0f, 0.0f));
            m_projection = glm::perspective(glm::radians(45.0f), a_screen_ratio, 0.1f, 10.0f);
            m_projection[1][1] *= -1;
        }

        void set_camera_y(float a_yaxis)
        {
            m_view = lookAt(glm::vec3(0.0f, 0.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f),
                glm::vec3(0.0f, a_yaxis, 0.0f));
        }

        void rotate_view(std::chrono::time_point<std::chrono::steady_clock>& ref_time)
        {
            auto cur_time = std::chrono::high_resolution_clock::now();
            auto time_elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(cur_time - ref_time);
            auto angle = (static_cast<float>(time_elapsed.count() % 3600000000) / 3.6e+9f) * 360.0f;

            m_model =
                translate(glm::mat4(1.0f), glm::vec3(0.04f, 0.0f, 0.0f)) *
                rotate(glm::mat4(1.0f), glm::radians(28.0f), glm::vec3(0.0f, -1.0f, 0.0f)) *
                rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(1.0f, 0.0f, 0.0f)) *
                rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 1.0f)) *
                scale(glm::mat4(1.0f), glm::vec3(0.325f, 0.325f, 0.325f));
        }

        glm::mat4 m_model;
        glm::mat4 m_view;
        glm::mat4 m_projection;
    };
}}

#endif //NCV_MODEL_VIEW_PROJECTION_HPP
