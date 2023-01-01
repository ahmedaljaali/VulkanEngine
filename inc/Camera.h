#pragma once

// glm
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

namespace VE
{
    class Camera final
    {
    private:  // Private variables
        glm::mat4 m_projectionMat;
        glm::mat4 m_viewMat;

    public:  // Public variables

    private:  // Private methods

    public:  // Public methods
        /*------------------------------------------------------------------*/
        /*                   Don't copy/move my class!!!                    */

        Camera(const Camera& copy) = delete;
        Camera& operator=(const Camera& copy) = delete;
        Camera(Camera&& move) = delete;
        Camera& operator=(Camera&& move) = delete;
        /*------------------------------------------------------------------*/

        // Constructor
        Camera(void) = default;

        // Destructor
        ~Camera(void) = default;

        void setOrthographicProjection(float left, float right, float top, float bottom, float near, float far);

        void setPerspectiveProjection(float verticalFieldOfView, float aspect, float near, float far);

        void setViewDirection(glm::vec3 camPosition, glm::vec3 camPointingDirection,
                              glm::vec3 upDirection = glm::vec3{0, -1, 0});

        // When you want to have camera locked into a specific point in space no matter how the
        // camera/target moves the target will be held in the center of the view
        void setViewTarget(glm::vec3 camPosition, glm::vec3 targetPosition,
                           glm::vec3 upDirection = glm::vec3{0, -1, 0});

        // Use Euler angles to specify the orientation of the camera
        void setViewYXZ(glm::vec3 camPosition, glm::vec3 rotation);

        [[nodiscard]] const glm::mat4& getProjection(void) const { return m_projectionMat; }
        [[nodiscard]] const glm::mat4& getView(void) const { return m_viewMat; }
    };
}
