#include "SimpleRenderSystem.h"

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
        glm::mat4 transform{1.0F};
        alignas(16) glm::vec3 color;
    };

    // Constructor
    SimpleRenderSystem::SimpleRenderSystem(Device& device, VkRenderPass renderPass)
        : m_device{device}, m_pipelineLayout{}
    {
        createPipelineLayout();
        createPipeline(renderPass);
    }

    // Destructor
    SimpleRenderSystem::~SimpleRenderSystem(void)
    {
        vkDestroyPipelineLayout(m_device.device(), m_pipelineLayout, nullptr);
    }

    void SimpleRenderSystem::createPipelineLayout(void)
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

    void SimpleRenderSystem::createPipeline(VkRenderPass renderPass)
    {
        if(m_pipelineLayout == nullptr)
        {
            throw std::runtime_error{"Can't create pipeline before pipeline layout!"};
        }

        PipelineConfigInfo pipelineConfig{};
        Pipeline::defaultPipelineConfig(pipelineConfig);

        pipelineConfig.renderPass = renderPass;
        pipelineConfig.pipelineLayout = m_pipelineLayout;

        m_pipeline = std::make_unique<Pipeline>(m_device, "shaders/simple.vert.spv", "shaders/simple.frag.spv",
                                                pipelineConfig);
    }

    void SimpleRenderSystem::renderGameObjects(VkCommandBuffer commandBuffer, std::vector<GameObject>& gameObjects,
                                               const Camera& camera)
    {
        m_pipeline->bind(commandBuffer);

        auto projectionView{camera.getProjection() * camera.getView()};

        // Render
        for(auto& obj : gameObjects)
        {

            SimplePushConstantData push{};
            // push.color = obj.objColor;
            push.transform = projectionView * obj.transform.mat4();

            vkCmdPushConstants(commandBuffer, m_pipelineLayout,
                               VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT, 0,
                               sizeof(SimplePushConstantData), &push);

            obj.model->bind(commandBuffer);
            obj.model->draw(commandBuffer);
        }
    }
}
