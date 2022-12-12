#pragma once

#include "Device.h"

// Vulkan headers
#include <vulkan/vulkan.h>

// std
#include <cstdint>
#include <string>
#include <vector>
#include <memory>

namespace VE
{
    class SwapChain final
    {
    private:  // Private variables
        VkFormat m_swapChainImageFormat;
        VkExtent2D m_swapChainExtent;

        std::vector<VkFramebuffer> m_swapChainFramebuffers;
        VkRenderPass m_renderPass;

        std::vector<VkImage> m_depthImages;
        std::vector<VkDeviceMemory> m_depthImageMemorys;
        std::vector<VkImageView> m_depthImageViews;
        std::vector<VkImage> m_swapChainImages;
        std::vector<VkImageView> m_swapChainImageViews;

        Device& m_device;
        VkExtent2D m_windowExtent;

        VkSwapchainKHR m_swapChain;
        std::shared_ptr<SwapChain> m_oldSwapChain;

        std::vector<VkSemaphore> m_imageAvailableSemaphores;
        std::vector<VkSemaphore> m_renderFinishedSemaphores;
        std::vector<VkFence> m_inFlightFences;
        std::vector<VkFence> m_imagesInFlight;
        std::size_t m_currentFrame;

    public:  // Public variables
        static constexpr int MAX_FRAMES_IN_FLIGHT{2};

    private:  // Private methods
        void init(void);
        void createSwapChain(void);
        void createImageViews(void);
        void createDepthResources(void);
        void createRenderPass(void);
        void createFramebuffers(void);
        void createSyncObjects(void);

        /*------------------------------------------------------------------*/
        /*                         Helper Functions                         */

        VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
        VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
        VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
        /*------------------------------------------------------------------*/

    public:  // Public methods
        /*------------------------------------------------------------------*/
        /*                  Don't copy or move my class!!                   */

        SwapChain(const SwapChain& copy) = delete;
        SwapChain& operator=(const SwapChain& copy) = delete;
        SwapChain(SwapChain&& move) = delete;
        SwapChain& operator=(SwapChain&& move) = delete;
        /*------------------------------------------------------------------*/

        // Constructor
        SwapChain(Device& device, VkExtent2D windowExtent);

        // Constructor
        SwapChain(Device& device, VkExtent2D windowExtent, std::shared_ptr<SwapChain> previousSwapChain);

        // Destructor
        ~SwapChain(void);

        /*------------------------------------------------------------------*/
        /*                             Getters                              */

        VkFramebuffer getFrameBuffer(std::uint32_t index) { return m_swapChainFramebuffers[index]; }
        VkRenderPass getRenderPass(void) { return m_renderPass; }
        VkImageView getImageView(std::uint32_t index) { return m_swapChainImageViews[index]; }
        std::size_t imageCount(void) { return m_swapChainImages.size(); }
        VkFormat getSwapChainImageFormat(void) { return m_swapChainImageFormat; }
        VkExtent2D getSwapChainExtent(void) { return m_swapChainExtent; }
        [[nodiscard]] std::uint32_t width(void) const { return m_swapChainExtent.width; }
        [[nodiscard]] std::uint32_t height(void) const { return m_swapChainExtent.height; }
        /*------------------------------------------------------------------*/

        [[nodiscard]] float extentAspectRatio(void) const
        {
            return static_cast<float>(m_swapChainExtent.width) / static_cast<float>(m_swapChainExtent.height);
        }
        VkFormat findDepthFormat(void);

        VkResult acquireNextImage(std::uint32_t* imageIndex);
        VkResult submitCommandBuffers(const VkCommandBuffer* commandBuffer, std::uint32_t* imageIndex);
    };
}
