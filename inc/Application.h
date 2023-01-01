#pragma once

#include "Device.h"
#include "GameObject.h"
#include "Model.h"
#include "Renderer.h"
#include "Window.h"

// vulkan headers
#include <vulkan/vulkan.h>

// std
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace VE
{
    class Application final
    {
    private:  // Private variables
        Window m_window;
        Device m_device;
        Renderer m_renderer;
        std::vector<GameObject> m_gameObjects;

    public:  // Public variables
        static constexpr std::uint32_t WIDTH{800};
        static constexpr std::uint32_t HEIGHT{600};

    private:  // Private methods
        void loadGameObjects(void);

    public:  // Public methods
        /*------------------------------------------------------------------*/
        /*                  Don't copy or move my class!!!                  */

        Application(const Application& copy) = delete;
        Application& operator=(const Application& copy) = delete;
        Application(Application&& move) = delete;
        Application& operator=(Application&& move) = delete;
        /*------------------------------------------------------------------*/

        // Constructor
        Application(void);

        // Destructor
        ~Application(void);

        void run(void);
        static void performance(float frameTime, bool inSeconds = false);
    };
}
