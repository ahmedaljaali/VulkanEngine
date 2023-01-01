#include "KeyboardMovementController.h"

#include <cmath>
#include <limits>

namespace VE
{
    KeyboardMovementController::KeyboardMovementController(void) : keys{}, moveSpeed{3.0F}, turnSpeed{1.0F} {}

    void KeyboardMovementController::moveInPlaneXZ(GLFWwindow* window, float frameTime, GameObject& gameObject)
    {
        // Store user input for the rotation values
        glm::vec3 rotate{0.0F};

        if(glfwGetKey(window, keys.lookRight) == GLFW_PRESS)
        {
            rotate.y += 1.0F;
        }
        if(glfwGetKey(window, keys.lookLeft) == GLFW_PRESS)
        {
            rotate.y -= 1.0F;
        }
        if(glfwGetKey(window, keys.lookUp) == GLFW_PRESS)
        {
            rotate.x += 1.0F;
        }
        if(glfwGetKey(window, keys.lookDown) == GLFW_PRESS)
        {
            rotate.x -= 1.0F;
        }

        if(glm::dot(rotate, rotate) > std::numeric_limits<float>::epsilon())
        {
            gameObject.transform.rotation += turnSpeed * frameTime * glm::normalize(rotate);
        }

        // limit pitch values between about +/- 85ish degrees
        gameObject.transform.rotation.x = glm::clamp(gameObject.transform.rotation.x, -1.5F, 1.5F);
        gameObject.transform.rotation.y = glm::mod(gameObject.transform.rotation.y, glm::two_pi<float>());

        float yaw{gameObject.transform.rotation.y};
        const glm::vec3 forwardDir{std::sin(yaw), 0.0F, std::cos(yaw)};
        const glm::vec3 rightDir{forwardDir.z, 0.0F, -forwardDir.x};
        const glm::vec3 upDir{0.0F, -1.0F, 0.0F};

        // Store user input for the movement values
        glm::vec3 moveDir{0.0F};

        if(glfwGetKey(window, keys.moveForward) == GLFW_PRESS)
        {
            moveDir += forwardDir;
        }
        if(glfwGetKey(window, keys.moveBackward) == GLFW_PRESS)
        {
            moveDir -= forwardDir;
        }
        if(glfwGetKey(window, keys.moveRight) == GLFW_PRESS)
        {
            moveDir += rightDir;
        }
        if(glfwGetKey(window, keys.moveLeft) == GLFW_PRESS)
        {
            moveDir -= rightDir;
        }
        if(glfwGetKey(window, keys.moveUp) == GLFW_PRESS)
        {
            moveDir += upDir;
        }
        if(glfwGetKey(window, keys.moveDown) == GLFW_PRESS)
        {
            moveDir -= upDir;
        }

        if(glm::dot(moveDir, moveDir) > std::numeric_limits<float>::epsilon())
        {
            gameObject.transform.translation += moveSpeed * frameTime * glm::normalize(moveDir);
        }
    }
}
