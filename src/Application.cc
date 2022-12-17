#include "Application.h"
#include "FrameTime.h"

// glm
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// std
#include <array>
#include <cstdio>
#include <iostream>
#include <stdexcept>

namespace VE
{
    struct SimplePushConstantData
    {
        glm::mat2 transform{1.0F};
        glm::vec2 offset;
        alignas(16) glm::vec3 color;
    };

    // Constructor
    Application::Application(void) : m_window{WIDTH, HEIGHT, "VulkanEngine"}, m_device{m_window}, m_pipelineLayout{}
    {
        loadGameObjects();
        createPipelineLayout();
        recreateSwapChain();
        createCommandBuffers();
    }

    // Destructor
    Application::~Application(void) { vkDestroyPipelineLayout(m_device.device(), m_pipelineLayout, nullptr); }

    void Application::loadGameObjects(void)
    {
        std::vector<Model::Vertex> vertices{
              {{0.0F, -0.5F}, {1.0, 1.0, 1.0}}, {{0.5F, 0.5F}, {1.0, 1.0, 1.0}}, {{-0.5F, 0.5F}, {1.0, 1.0, 1.0}}};

        auto model{std::make_shared<Model>(m_device, vertices)};

        std::vector<glm::vec3> colors{
              {1.F, .7F, .73F}, {1.F, .87F, .73F}, {1.F, 1.F, .73F}, {.73F, 1.F, .8F}, {.73, .88F, 1.F}};

        for(auto& color : colors)
        {
            color = glm::pow(color, glm::vec3{2.2F});
        }

        for(std::uint32_t objectIndex{}; objectIndex < 40; ++objectIndex)
        {
            auto triangle{GameObject::createGameObject()};
            triangle.model = model;
            triangle.transform2d.scale = glm::vec2(.5F) + objectIndex * 0.025F;
            triangle.transform2d.rotation = objectIndex * glm::pi<float>() * .025f;
            triangle.objColor = colors[objectIndex % colors.size()];
            m_gameObjects.push_back(std::move(triangle));
        }
    }

    void Application::createPipelineLayout(void)
    {
        VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(SimplePushConstantData);

        VkPipelineLayoutCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        createInfo.setLayoutCount = 0;
        createInfo.pSetLayouts = nullptr;
        createInfo.pushConstantRangeCount = 1;
        createInfo.pPushConstantRanges = &pushConstantRange;

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
        m_commandBuffers.clear();
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

        renderGameObjects(m_commandBuffers[imageIndex]);

        vkCmdEndRenderPass(m_commandBuffers[imageIndex]);

        if(vkEndCommandBuffer(m_commandBuffers[imageIndex]) != VK_SUCCESS)
        {
            throw std::runtime_error{"Failed to finish recording command buffer!"};
        }
    }

    void Application::renderGameObjects(VkCommandBuffer commandBuffer)
    {
        m_pipeline->bind(commandBuffer);

        // Update
        for(std::uint32_t objIndex{}; auto& obj : m_gameObjects)
        {
            ++objIndex;
            obj.transform2d.rotation =
                  glm::mod<float>(obj.transform2d.rotation + 0.001f * objIndex, glm::two_pi<float>());
        }

        // Render
        for(auto& obj : m_gameObjects)
        {
            SimplePushConstantData push{};
            push.offset = obj.transform2d.translation;
            push.color = obj.objColor;
            push.transform = obj.transform2d.mat2();

            vkCmdPushConstants(commandBuffer, m_pipelineLayout,
                               VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT, 0,
                               sizeof(SimplePushConstantData), &push);

            obj.model->bind(commandBuffer);
            obj.model->draw(commandBuffer);
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
