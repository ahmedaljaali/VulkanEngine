#pragma once

#include "Device.h"
#include "Model.h"
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
    class Renderer final
    {
    private:  // Private variables
        Window& m_window;
        Device& m_device;
        std::unique_ptr<SwapChain> m_swapChain;
        std::vector<VkCommandBuffer> m_commandBuffers;

        // Track the current image that is in progress
        std::uint32_t m_currentImageIndex;
        bool m_isFrameStarted;

        // Track frame index in this range [0, MAX_FRAMES_IN_FLIGHT]
        // Doesn't depend on m_currentImageIndex
        std::uint32_t m_currentFrameIndex;

    public:  // Public variables

    private:  // Private methods
        void createCommandBuffers(void);
        void freeCommandBuffers(void);
        void recreateSwapChain(void);

    public:  // Public methods
        /*------------------------------------------------------------------*/
        /*                  Don't copy or move my class!!!                  */

        Renderer(const Renderer& copy) = delete;
        Renderer& operator=(const Renderer& copy) = delete;
        Renderer(Renderer&& move) = delete;
        Renderer& operator=(Renderer&& move) = delete;
        /*------------------------------------------------------------------*/

        // Constructor
        Renderer(Window& window, Device& device);

        // Destructor
        ~Renderer(void);

        VkCommandBuffer beginFrame(void);
        void endFrame(void);

        void beginSwapChainRenderPass(VkCommandBuffer commandBuffer);
        void endSwapChainRenderPass(VkCommandBuffer commandBuffer);

        // Getters
        [[nodiscard]] bool isFrameInProgress(void) const { return m_isFrameStarted; }
        [[nodiscard]] VkRenderPass getSwapChainRenderPass(void) const { return m_swapChain->getRenderPass(); }
        [[nodiscard]] float getSwapChainAspectRatio(void) const { return m_swapChain->extentAspectRatio(); }
        [[nodiscard]] VkCommandBuffer getCurrentCommandBuffer(void) const;
        [[nodiscard]] std::uint32_t getFrameIndex(void) const;
    };
}
