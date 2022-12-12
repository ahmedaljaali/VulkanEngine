#pragma once

#include "Device.h"
#include "Model.h"
#include "Pipeline.h"
#include "SwapChain.h"
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
        std::unique_ptr<SwapChain> m_swapChain;
        std::unique_ptr<Pipeline> m_pipeline;
        VkPipelineLayout m_pipelineLayout;
        std::vector<VkCommandBuffer> m_commandBuffers;
        std::unique_ptr<Model> m_model;

    public:  // Public variables
        static constexpr std::uint32_t WIDTH{800};
        static constexpr std::uint32_t HEIGHT{600};

    private:  // Private methods
        void loadModels(void);
        void createPipelineLayout(void);
        void createPipeline(void);
        void createCommandBuffers(void);
        void freeCommandBuffers(void);
        void recordCommandBuffer(std::uint32_t imageIndex);
        void drawFrame(void);
        void recreateSwapChain(void);

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
