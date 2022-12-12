#include "SwapChain.h"

// std
#include <array>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <limits>
#include <set>
#include <stdexcept>
#include <utility>

namespace VE
{

    SwapChain::SwapChain(Device& device, VkExtent2D windowExtent)
        : m_swapChainImageFormat{},
          m_swapChainExtent{},
          m_renderPass{},
          m_device{device},
          m_windowExtent{windowExtent},
          m_swapChain{},
          m_currentFrame{}
    {
        init();
    }

    SwapChain::SwapChain(Device& device, VkExtent2D windowExtent, std::shared_ptr<SwapChain> previousSwapChain)
        : m_swapChainImageFormat{},
          m_swapChainExtent{},
          m_renderPass{},
          m_device{device},
          m_windowExtent{windowExtent},
          m_swapChain{},
          m_oldSwapChain{std::move(previousSwapChain)},
          m_currentFrame{}
    {
        init();

        // clean up the old swap chain
        m_oldSwapChain = nullptr;
    }

    void SwapChain::init(void)
    {
        createSwapChain();
        createImageViews();
        createRenderPass();
        createDepthResources();
        createFramebuffers();
        createSyncObjects();
    }

    SwapChain::~SwapChain(void)
    {
        for(auto& imageView : m_swapChainImageViews)
        {
            vkDestroyImageView(m_device.device(), imageView, nullptr);
        }
        m_swapChainImageViews.clear();

        if(m_swapChain)
        {
            vkDestroySwapchainKHR(m_device.device(), m_swapChain, nullptr);
            m_swapChain = nullptr;
        }

        for(std::uint32_t imageIndex{}; imageIndex < m_depthImages.size(); ++imageIndex)
        {
            vkDestroyImageView(m_device.device(), m_depthImageViews[imageIndex], nullptr);
            vkDestroyImage(m_device.device(), m_depthImages[imageIndex], nullptr);
            vkFreeMemory(m_device.device(), m_depthImageMemorys[imageIndex], nullptr);
        }

        for(auto& framebuffer : m_swapChainFramebuffers)
        {
            vkDestroyFramebuffer(m_device.device(), framebuffer, nullptr);
        }

        vkDestroyRenderPass(m_device.device(), m_renderPass, nullptr);

        // cleanup synchronization objects
        for(std::size_t semaIndex{}; semaIndex < MAX_FRAMES_IN_FLIGHT; ++semaIndex)
        {
            vkDestroySemaphore(m_device.device(), m_renderFinishedSemaphores[semaIndex], nullptr);
            vkDestroySemaphore(m_device.device(), m_imageAvailableSemaphores[semaIndex], nullptr);
            vkDestroyFence(m_device.device(), m_inFlightFences[semaIndex], nullptr);
        }
    }

    VkResult SwapChain::acquireNextImage(std::uint32_t* imageIndex)
    {
        // Waiting for the command buffer to finished executing
        vkWaitForFences(m_device.device(),
                        1,
                        &m_inFlightFences[m_currentFrame],
                        VK_TRUE,
                        std::numeric_limits<std::uint64_t>::max());

        return vkAcquireNextImageKHR(m_device.device(),
                                     m_swapChain,
                                     std::numeric_limits<std::uint64_t>::max(),
                                     m_imageAvailableSemaphores[m_currentFrame],  // must be a not signaled semaphore
                                     VK_NULL_HANDLE,
                                     imageIndex);
    }

    VkResult SwapChain::submitCommandBuffers(const VkCommandBuffer* commandBuffer, std::uint32_t* imageIndex)
    {
        if(m_imagesInFlight[*imageIndex] != VK_NULL_HANDLE)
        {
            vkWaitForFences(m_device.device(), 1, &m_imagesInFlight[*imageIndex], VK_TRUE, UINT64_MAX);
        }
        m_imagesInFlight[*imageIndex] = m_inFlightFences[m_currentFrame];

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        std::array<VkSemaphore, 1> waitSemaphores{m_imageAvailableSemaphores[m_currentFrame]};
        std::array<VkPipelineStageFlags, 1> waitStages{VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        submitInfo.waitSemaphoreCount = static_cast<std::uint32_t>(waitSemaphores.size());
        submitInfo.pWaitSemaphores = waitSemaphores.data();
        submitInfo.pWaitDstStageMask = waitStages.data();

        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = commandBuffer;

        std::array<VkSemaphore, 1> signalSemaphores{m_renderFinishedSemaphores[m_currentFrame]};
        submitInfo.signalSemaphoreCount = static_cast<std::uint32_t>(signalSemaphores.size());
        submitInfo.pSignalSemaphores = signalSemaphores.data();

        vkResetFences(m_device.device(), 1, &m_inFlightFences[m_currentFrame]);
        if(vkQueueSubmit(m_device.graphicsQueue(), 1, &submitInfo, m_inFlightFences[m_currentFrame]) != VK_SUCCESS)
        {
            throw std::runtime_error{"Failed to submit draw command buffer!"};
        }

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        // Waiting for the command buffer to finished executing
        presentInfo.waitSemaphoreCount = static_cast<std::uint32_t>(signalSemaphores.size());
        presentInfo.pWaitSemaphores = signalSemaphores.data();

        std::vector<VkSwapchainKHR> swapChains{m_swapChain};
        presentInfo.swapchainCount = static_cast<std::uint32_t>(swapChains.size());
        presentInfo.pSwapchains = swapChains.data();

        presentInfo.pImageIndices = imageIndex;

        m_currentFrame = (m_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

        return vkQueuePresentKHR(m_device.presentQueue(), &presentInfo);
    }

    void SwapChain::createSwapChain(void)
    {
        SwapChainSupportDetails swapChainSupport{m_device.getSwapChainSupport()};

        VkSurfaceFormatKHR surfaceFormat{chooseSwapSurfaceFormat(swapChainSupport.formats)};
        VkPresentModeKHR presentMode{chooseSwapPresentMode(swapChainSupport.presentModes)};
        VkExtent2D extent{chooseSwapExtent(swapChainSupport.capabilities)};

        std::uint32_t imageCount{swapChainSupport.capabilities.minImageCount + 1};

        // Checking if we don't have limits
        if(swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
        {
            imageCount = swapChainSupport.capabilities.maxImageCount;
        }

        VkSwapchainCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = m_device.surface();

        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        QueueFamilyIndices indices{m_device.findPhysicalQueueFamilies()};
        std::array<std::uint32_t, 2> queueFamilyIndices{indices.graphicsFamily.value(), indices.presentFamily.value()};

        if(indices.graphicsFamily != indices.presentFamily)
        {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = static_cast<std::uint32_t>(queueFamilyIndices.size());
            createInfo.pQueueFamilyIndices = queueFamilyIndices.data();
        }
        else
        {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            createInfo.queueFamilyIndexCount = 0;      // Optional
            createInfo.pQueueFamilyIndices = nullptr;  // Optional
        }

        createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

        createInfo.presentMode = presentMode;
        createInfo.clipped = VK_TRUE;

        createInfo.oldSwapchain = (m_oldSwapChain == nullptr) ? VK_NULL_HANDLE : m_oldSwapChain->m_swapChain;

        if(vkCreateSwapchainKHR(m_device.device(), &createInfo, nullptr, &m_swapChain) != VK_SUCCESS)
        {
            throw std::runtime_error{"Failed to create swap chain!"};
        }

        // we only specified a minimum number of images in the swap chain, so the implementation is
        // allowed to create a swap chain with more. That's why we'll first query the final number of
        // images with vkGetSwapchainImagesKHR, then resize the container and finally call it again to
        // retrieve the handles.
        vkGetSwapchainImagesKHR(m_device.device(), m_swapChain, &imageCount, nullptr);
        m_swapChainImages.resize(imageCount);
        vkGetSwapchainImagesKHR(m_device.device(), m_swapChain, &imageCount, m_swapChainImages.data());

        m_swapChainImageFormat = surfaceFormat.format;
        m_swapChainExtent = extent;
    }

    void SwapChain::createImageViews(void)
    {
        m_swapChainImageViews.resize(m_swapChainImages.size());

        for(std::size_t imageIndex{}; imageIndex < m_swapChainImages.size(); ++imageIndex)
        {
            VkImageViewCreateInfo viewInfo{};
            viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            viewInfo.image = m_swapChainImages[imageIndex];
            viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            viewInfo.format = m_swapChainImageFormat;
            viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            viewInfo.subresourceRange.baseMipLevel = 0;
            viewInfo.subresourceRange.levelCount = 1;
            viewInfo.subresourceRange.baseArrayLayer = 0;
            viewInfo.subresourceRange.layerCount = 1;

            if(vkCreateImageView(m_device.device(), &viewInfo, nullptr, &m_swapChainImageViews[imageIndex]) !=
               VK_SUCCESS)
            {
                throw std::runtime_error{"Failed to create texture image view!"};
            }
        }
    }

    void SwapChain::createRenderPass(void)
    {
        VkAttachmentDescription depthAttachment{};
        depthAttachment.format = findDepthFormat();
        depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkAttachmentReference depthAttachmentRef{};
        depthAttachmentRef.attachment = 1;
        depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkAttachmentDescription colorAttachment{};
        colorAttachment.format = getSwapChainImageFormat();
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentReference colorAttachmentRef{};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;
        subpass.pDepthStencilAttachment = &depthAttachmentRef;

        VkSubpassDependency dependency{};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.srcAccessMask = 0;

        dependency.srcStageMask =
              VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.dstSubpass = 0;

        dependency.dstStageMask =
              VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        std::array<VkAttachmentDescription, 2> attachments{colorAttachment, depthAttachment};

        VkRenderPassCreateInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = static_cast<std::uint32_t>(attachments.size());
        renderPassInfo.pAttachments = attachments.data();
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies = &dependency;

        if(vkCreateRenderPass(m_device.device(), &renderPassInfo, nullptr, &m_renderPass) != VK_SUCCESS)
        {
            throw std::runtime_error{"Failed to create render pass!"};
        }
    }

    void SwapChain::createFramebuffers(void)
    {
        m_swapChainFramebuffers.resize(imageCount());
        for(std::size_t imgaeIndex{}; imgaeIndex < imageCount(); ++imgaeIndex)
        {
            std::array<VkImageView, 2> attachments{m_swapChainImageViews[imgaeIndex], m_depthImageViews[imgaeIndex]};

            VkExtent2D swapChainExtent{getSwapChainExtent()};
            VkFramebufferCreateInfo framebufferInfo{};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = m_renderPass;
            framebufferInfo.attachmentCount = static_cast<std::uint32_t>(attachments.size());
            framebufferInfo.pAttachments = attachments.data();
            framebufferInfo.width = swapChainExtent.width;
            framebufferInfo.height = swapChainExtent.height;
            framebufferInfo.layers = 1;

            if(vkCreateFramebuffer(
                     m_device.device(), &framebufferInfo, nullptr, &m_swapChainFramebuffers[imgaeIndex]) != VK_SUCCESS)
            {
                throw std::runtime_error{"Failed to create framebuffer!"};
            }
        }
    }

    void SwapChain::createDepthResources(void)
    {
        VkFormat depthFormat{findDepthFormat()};
        VkExtent2D swapChainExtent{getSwapChainExtent()};

        m_depthImages.resize(imageCount());
        m_depthImageMemorys.resize(imageCount());
        m_depthImageViews.resize(imageCount());

        for(std::uint32_t imageIndex{}; imageIndex < m_depthImages.size(); ++imageIndex)
        {
            VkImageCreateInfo imageInfo{};
            imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            imageInfo.imageType = VK_IMAGE_TYPE_2D;
            imageInfo.extent.width = swapChainExtent.width;
            imageInfo.extent.height = swapChainExtent.height;
            imageInfo.extent.depth = 1;
            imageInfo.mipLevels = 1;
            imageInfo.arrayLayers = 1;
            imageInfo.format = depthFormat;
            imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
            imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
            imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
            imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            imageInfo.flags = 0;

            m_device.createImageWithInfo(imageInfo,
                                         VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                         m_depthImages[imageIndex],
                                         m_depthImageMemorys[imageIndex]);

            VkImageViewCreateInfo viewInfo{};
            viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            viewInfo.image = m_depthImages[imageIndex];
            viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            viewInfo.format = depthFormat;
            viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
            viewInfo.subresourceRange.baseMipLevel = 0;
            viewInfo.subresourceRange.levelCount = 1;
            viewInfo.subresourceRange.baseArrayLayer = 0;
            viewInfo.subresourceRange.layerCount = 1;

            if(vkCreateImageView(m_device.device(), &viewInfo, nullptr, &m_depthImageViews[imageIndex]) != VK_SUCCESS)
            {
                throw std::runtime_error{"Failed to create texture image view!"};
            }
        }
    }

    void SwapChain::createSyncObjects(void)
    {
        m_imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        m_renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        m_inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
        m_imagesInFlight.resize(imageCount(), VK_NULL_HANDLE);

        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for(std::size_t syncObjIndex{}; syncObjIndex < MAX_FRAMES_IN_FLIGHT; ++syncObjIndex)
        {
            if(vkCreateSemaphore(
                     m_device.device(), &semaphoreInfo, nullptr, &m_imageAvailableSemaphores[syncObjIndex]) ||
               vkCreateSemaphore(
                     m_device.device(), &semaphoreInfo, nullptr, &m_renderFinishedSemaphores[syncObjIndex]) ||
               vkCreateFence(m_device.device(), &fenceInfo, nullptr, &m_inFlightFences[syncObjIndex]) != VK_SUCCESS)
            {
                throw std::runtime_error{"Failed to create synchronization objects!"};
            }
        }
    }

    VkSurfaceFormatKHR SwapChain::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
    {
        for(const auto& availableFormat : availableFormats)
        {
            if(availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
               availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            {
                return availableFormat;
            }
        }

        return availableFormats[0];
    }

    VkPresentModeKHR SwapChain::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
    {
        for(const auto& availablePresentMode : availablePresentModes)
        {
            if(availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
            {
                std::cout << "Present mode: Mailbox" << std::endl;
                return availablePresentMode;
            }

            if(availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR)
            {
                std::cout << "Present mode: Immediate" << std::endl;
                return availablePresentMode;
            }
        }

        std::cout << "Present mode: V-Sync" << std::endl;
        return VK_PRESENT_MODE_FIFO_KHR;
    }

    VkExtent2D SwapChain::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
    {
        if(capabilities.currentExtent.width != std::numeric_limits<std::uint32_t>::max())
        {
            return capabilities.currentExtent;
        }

        VkExtent2D actualExtent{m_windowExtent};
        actualExtent.width = std::max(capabilities.minImageExtent.width,
                                      std::min(capabilities.maxImageExtent.width, actualExtent.width));
        actualExtent.height = std::max(capabilities.minImageExtent.height,
                                       std::min(capabilities.maxImageExtent.height, actualExtent.height));

        return actualExtent;
    }

    VkFormat SwapChain::findDepthFormat(void)
    {
        return m_device.findSupportedFormat(
              {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
              VK_IMAGE_TILING_OPTIMAL,
              VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
    }
}
