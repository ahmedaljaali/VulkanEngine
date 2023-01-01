#pragma once

#include "GameObject.h"
#include "Window.h"

namespace VE
{
    class KeyboardMovementController final
    {
    private:  // Private variables

    public:  // Public variables
        struct keyMapping
        {
            int moveLeft{GLFW_KEY_A};
            int moveRight{GLFW_KEY_D};
            int moveForward{GLFW_KEY_W};
            int moveBackward{GLFW_KEY_S};
            int moveUp{GLFW_KEY_E};
            int moveDown{GLFW_KEY_Q};
            int lookLeft{GLFW_KEY_LEFT};
            int lookRight{GLFW_KEY_RIGHT};
            int lookUp{GLFW_KEY_UP};
            int lookDown{GLFW_KEY_DOWN};
        };

        keyMapping keys;

        float moveSpeed;
        float turnSpeed;

    private:  // Private methods

    public:  // Public methods
        /*------------------------------------------------------------------*/
        /*                   Don't copy/move my class!!!                    */

        KeyboardMovementController(const KeyboardMovementController& copy) = delete;
        KeyboardMovementController& operator=(const KeyboardMovementController& copy) = delete;
        KeyboardMovementController(KeyboardMovementController&& move) = delete;
        KeyboardMovementController& operator=(KeyboardMovementController&& move) = delete;
        /*------------------------------------------------------------------*/

        // Constructor
        KeyboardMovementController(void);

        // Destructor
        ~KeyboardMovementController(void) = default;

        void moveInPlaneXZ(GLFWwindow* window, float frameTime, GameObject& gameObject);
    };
}
