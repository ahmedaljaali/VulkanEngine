#include "Application.h"
#include "FrameTime.h"

// std
#include <array>
#include <cstdio>
#include <iostream>
#include <stdexcept>

namespace VE
{
    // Constructor
    Application::Application(void) : m_window{WIDTH, HEIGHT, "VulkanEngine"}, m_device{m_window}, m_pipelineLayout{}
    {
        loadModels();
        createPipelineLayout();
        recreateSwapChain();
        createCommandBuffers();
    }

    // Destructor
    Application::~Application(void) { vkDestroyPipelineLayout(m_device.device(), m_pipelineLayout, nullptr); }

    void Application::loadModels(void)
    {
        std::vector<Model::Vertex> vertices{
              {{0.0F, -0.5F}, {1.0, 0.0, 0.0}}, {{0.5F, 0.5F}, {0.0, 1.0, 0.0}}, {{-0.5F, 0.5F}, {0.0, 0.0, 1.0}}};

        m_model = std::make_unique<Model>(m_device, vertices);
    }

    void Application::createPipelineLayout(void)
    {
        VkPipelineLayoutCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        createInfo.setLayoutCount = 0;
        createInfo.pSetLayouts = nullptr;
        createInfo.pushConstantRangeCount = 0;
        createInfo.pPushConstantRanges = nullptr;

        if(vkCreatePipelineLayout(m_device.device(), &createInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS)
        {
            throw std::runtime_error{"Failed to create pipeline layout!"};
        }
    }

    void Application::recreateSwapChain(void)
    {
        auto extent{m_window.getExtent()};

        while(extent.width == 0 || extent.height == 0)
        {
            extent = m_window.getExtent();
            glfwWaitEvents();
        }
        vkDeviceWaitIdle(m_device.device());

        if(m_swapChain == nullptr)
        {
            m_swapChain = std::make_unique<SwapChain>(m_device, m_window.getExtent());
        }
        else
        {
            m_swapChain = std::make_unique<SwapChain>(m_device, m_window.getExtent(), std::move(m_swapChain));
            if(m_swapChain->imageCount() != m_commandBuffers.size())
            {
                freeCommandBuffers();
                createCommandBuffers();
            }
        }
        createPipeline();
    }

    void Application::createPipeline(void)
    {
        if(m_swapChain == nullptr)
        {
            throw std::runtime_error{"Can't create pipeline before swap chain!"};
        }

        if(m_pipelineLayout == nullptr)
        {
            throw std::runtime_error{"Can't create pipeline before pipeline layout!"};
        }

        PipelineConfigInfo pipelineConfig{};
        Pipeline::defaultPipelineConfig(pipelineConfig);

        pipelineConfig.renderPass = m_swapChain->getRenderPass();
        pipelineConfig.pipelineLayout = m_pipelineLayout;

        m_pipeline = std::make_unique<Pipeline>(m_device, "shaders/simple.vert.spv", "shaders/simple.frag.spv",
                                                pipelineConfig);
    }

    void Application::createCommandBuffers(void)
    {
        m_commandBuffers.resize(m_swapChain->imageCount());

        // Allocate command buffers
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = m_device.getCommandPool();
        allocInfo.commandBufferCount = static_cast<std::uint32_t>(m_commandBuffers.size());

        if(vkAllocateCommandBuffers(m_device.device(), &allocInfo, m_commandBuffers.data()) != VK_SUCCESS)
        {
            throw std::runtime_error{"Failed to allocate command buffers!"};
        }
    }

    void Application::freeCommandBuffers(void)
    {
        vkFreeCommandBuffers(m_device.device(), m_device.getCommandPool(),
                             static_cast<std::uint32_t>(m_commandBuffers.size()), m_commandBuffers.data());
    }

    void Application::recordCommandBuffer(std::uint32_t imageIndex)
    {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if(vkBeginCommandBuffer(m_commandBuffers[imageIndex], &beginInfo) != VK_SUCCESS)
        {
            throw std::runtime_error{"Failed to begin recording command buffer!"};
        }

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = m_swapChain->getRenderPass();
        renderPassInfo.framebuffer = m_swapChain->getFrameBuffer(imageIndex);

        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = m_swapChain->getSwapChainExtent();

        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = {{0.0F, 0.0F, 0.0F, 1.0F}};
        clearValues[1].depthStencil = {1.0F, 0};

        renderPassInfo.clearValueCount = static_cast<std::uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(m_commandBuffers[imageIndex], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        VkViewport viewport{};
        viewport.x = 0.0F;
        viewport.y = 0.0F;
        viewport.width = static_cast<float>(m_swapChain->getSwapChainExtent().width);
        viewport.height = static_cast<float>(m_swapChain->getSwapChainExtent().height);
        viewport.minDepth = 0.0F;
        viewport.maxDepth = 1.0F;
        vkCmdSetViewport(m_commandBuffers[imageIndex], 0, 1, &viewport);

        VkRect2D scissor{{0, 0}, m_swapChain->getSwapChainExtent()};
        vkCmdSetScissor(m_commandBuffers[imageIndex], 0, 1, &scissor);

        m_pipeline->bind(m_commandBuffers[imageIndex]);

        m_model->bind(m_commandBuffers[imageIndex]);
        m_model->draw(m_commandBuffers[imageIndex]);

        vkCmdEndRenderPass(m_commandBuffers[imageIndex]);

        if(vkEndCommandBuffer(m_commandBuffers[imageIndex]) != VK_SUCCESS)
        {
            throw std::runtime_error{"Failed to finish recording command buffer!"};
        }
    }

    void Application::drawFrame(void)
    {
        std::uint32_t imageIndex{};
        auto result{m_swapChain->acquireNextImage(&imageIndex)};

        if(result == VK_ERROR_OUT_OF_DATE_KHR)
        {
            recreateSwapChain();
            return;
        }

        if(result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
        {
            throw std::runtime_error{"Failed to acquire swap chain image!"};
        }

        recordCommandBuffer(imageIndex);

        result = m_swapChain->submitCommandBuffers(&m_commandBuffers[imageIndex], &imageIndex);

        if(result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_window.wasWindowResized())
        {
            m_window.resetWindowResizedFlag();
            recreateSwapChain();
            return;
        }

        if(result != VK_SUCCESS)
        {
            throw std::runtime_error{"Failed to present swap chain image!"};
        }
    }

    void Application::run(void)
    {
        FrameTime frameTime;

        while(!m_window.shouldClose())
        {
            m_window.pollEvents();
            frameTime.gameLoopStarted();

            drawFrame();

            performance(frameTime.getFrameTime());
        }

        vkDeviceWaitIdle(m_device.device());
    }

    void Application::performance(float frameTime, bool inSeconds)
    {
        static float time{frameTime};
        static std::size_t numOfFrames{};
        ++numOfFrames;

        if(time >= 1)
        {
            if(inSeconds)
            {
                std::cerr << static_cast<double>(numOfFrames) << " FPS" << '\r';
            }
            else
            {
                std::cerr << 1000 / static_cast<double>(numOfFrames) << " ms/Frame" << '\r';
            }

            // Reset the timer
            numOfFrames = 0;
            time = 0;
        }
        else
        {
            time += frameTime;
        }
    }
}
