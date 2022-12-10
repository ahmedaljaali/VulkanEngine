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
    Application::Application(void)
        : m_window{WIDTH, HEIGHT, "VulkanEngine"},
          m_device{m_window},
          m_swapChain{m_device, m_window.getExtent()},
          m_pipelineLayout{}
    {
        createPipelineLayout();
        createPipeline();
        createCommandBuffers();
    }

    // Destructor
    Application::~Application(void) { vkDestroyPipelineLayout(m_device.device(), m_pipelineLayout, nullptr); }

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

    void Application::createPipeline(void)
    {
        PipelineConfigInfo pipelineConfig{Pipeline::defaultPipelineConfig(m_swapChain.width(), m_swapChain.height())};

        pipelineConfig.renderPass = m_swapChain.getRenderPass();
        pipelineConfig.pipelineLayout = m_pipelineLayout;

        m_pipeline = std::make_unique<Pipeline>(m_device, "shaders/simple.vert.spv", "shaders/simple.frag.spv",
                                                pipelineConfig);
    }

    void Application::createCommandBuffers(void)
    {
        m_commandBuffers.resize(m_swapChain.imageCount());

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

        // Record commands
        for(std::uint32_t bufferIndex{}; bufferIndex < m_commandBuffers.size(); ++bufferIndex)
        {
            VkCommandBufferBeginInfo beginInfo{};
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

            if(vkBeginCommandBuffer(m_commandBuffers[bufferIndex], &beginInfo) != VK_SUCCESS)
            {
                throw std::runtime_error{"Failed to begin recording command buffer!"};
            }

            VkRenderPassBeginInfo renderPassInfo{};
            renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            renderPassInfo.renderPass = m_swapChain.getRenderPass();
            renderPassInfo.framebuffer = m_swapChain.getFrameBuffer(bufferIndex);

            renderPassInfo.renderArea.offset = {0, 0};
            renderPassInfo.renderArea.extent = m_swapChain.getSwapChainExtent();

            std::array<VkClearValue, 2> clearValues{};
            clearValues[0].color = {{0.0F, 0.0F, 0.0F, 1.0F}};
            clearValues[1].depthStencil = {1.0F, 0};

            renderPassInfo.clearValueCount = static_cast<std::uint32_t>(clearValues.size());
            renderPassInfo.pClearValues = clearValues.data();

            vkCmdBeginRenderPass(m_commandBuffers[bufferIndex], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

            m_pipeline->bind(m_commandBuffers[bufferIndex]);

            vkCmdDraw(m_commandBuffers[bufferIndex], 3, 1, 0, 0);

            vkCmdEndRenderPass(m_commandBuffers[bufferIndex]);

            if(vkEndCommandBuffer(m_commandBuffers[bufferIndex]) != VK_SUCCESS)
            {
                throw std::runtime_error{"Failed to finish recording command buffer!"};
            }
        }
    }

    void Application::drawFrame(void)
    {
        std::uint32_t imageIndex{};
        auto result{m_swapChain.acquireNextImage(&imageIndex)};

        if(result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
        {
            throw std::runtime_error{"Failed to acquire swap chain image!"};
        }

        result = m_swapChain.submitCommandBuffers(&m_commandBuffers[imageIndex], &imageIndex);

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
