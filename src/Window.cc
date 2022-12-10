#include "Window.h"

// std
#include <stdexcept>

namespace VE
{
    // Constructor
    Window::Window(std::int32_t width, std::int32_t height, const std::string& name)
        : m_window{}, m_width{width}, m_height{height}, m_name{name}
    {
        init();
    }

    // Destructor
    Window::~Window(void)
    {
        glfwDestroyWindow(m_window);
        glfwTerminate();
    }

    void Window::init(void)
    {
        glfwInit();

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        m_window = glfwCreateWindow(m_width, m_height, m_name.c_str(), nullptr, nullptr);
        if(!m_window)
        {
            throw std::runtime_error{"Failed to create GLFW window!"};
        }
    }

    bool Window::shouldClose(void) { return glfwWindowShouldClose(m_window); }

    void Window::pollEvents(void) { glfwPollEvents(); }

    void Window::createWindowSurface(VkInstance instance, VkSurfaceKHR* surface)
    {
        if(glfwCreateWindowSurface(instance, m_window, nullptr, surface) != VK_SUCCESS)
        {
            throw std::runtime_error{"Failed to create window surface!"};
        }
    }

    std::vector<const char*> Window::getInstanceExtensions(void)
    {
        std::uint32_t extensionCount{};
        const char** glfwExtensions{glfwGetRequiredInstanceExtensions(&extensionCount)};

        std::vector<const char*> extensions(glfwExtensions, glfwExtensions + extensionCount);

        return extensions;
    }

    VkExtent2D Window::getExtent(void)
    {
        std::int32_t width{};
        std::int32_t height{};
        glfwGetFramebufferSize(m_window, &width, &height);

        return {static_cast<std::uint32_t>(width), static_cast<std::uint32_t>(height)};
    }
}
