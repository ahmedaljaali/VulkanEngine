#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

// std
#include <cstdint>
#include <string>
#include <vector>

namespace VE
{
    class Window final
    {
    private:  // Private variables
        GLFWwindow* m_window;
        std::int32_t m_width;
        std::int32_t m_height;
        std::string m_name;
        bool m_framebufferResized;

    public:  // Public variables

    private:  // Private methods
        void init(void);
        static void framebufferResizedCallback(GLFWwindow* window, std::int32_t width, std::int32_t height);

    public:  // Public methods
        /*------------------------------------------------------------------*/
        /*                   Don't copy or move my class                    */

        Window(const Window& copy) = delete;
        Window& operator=(const Window& copy) = delete;
        Window(Window&& move) = delete;
        Window& operator=(Window&& move) = delete;
        /*------------------------------------------------------------------*/

        // Constructor
        Window(std::int32_t width, std::int32_t height, const std::string& name);

        // Destructor
        ~Window(void);

        bool shouldClose(void);
        void pollEvents(void);
        void createWindowSurface(VkInstance instance, VkSurfaceKHR* surface);
        static std::vector<const char*> getInstanceExtensions(void);
        VkExtent2D getExtent(void);
        [[nodiscard]] bool wasWindowResized(void) const;
        void resetWindowResizedFlag(void);
    };
}
