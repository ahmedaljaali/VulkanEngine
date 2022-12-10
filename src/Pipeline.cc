#include "Pipeline.h"

// std
#include <array>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <stdexcept>

namespace VE
{
    // Constructor
    Pipeline::Pipeline(Device& device, const std::string& vertFilePath, const std::string& fragFilePath,
                       const PipelineConfigInfo& configInfo)
        : m_device{device}, m_graphicsPipeline{}, m_vertShaderModule{}, m_fragShaderModule{}
    {
        createGraphicsPipeline(vertFilePath, fragFilePath, configInfo);
    }

    // Destructor
    Pipeline::~Pipeline(void)
    {
        vkDestroyShaderModule(m_device.device(), m_vertShaderModule, nullptr);
        vkDestroyShaderModule(m_device.device(), m_fragShaderModule, nullptr);

        vkDestroyPipeline(m_device.device(), m_graphicsPipeline, nullptr);
    }

    std::vector<char> Pipeline::readFile(const std::string& filePath)
    {
        std::fstream file{filePath, std::ios::binary | std::ios::ate | std::ios::in};

        if(!file.is_open())
        {
            throw std::runtime_error{"Filed to read a file(" + filePath + ")!"};
        }

        std::streamsize fileSize{file.tellg()};
        std::vector<char> buffer(static_cast<std::size_t>(fileSize));

        file.seekg(0);
        file.read(buffer.data(), fileSize);

        file.close();
        return buffer;
    }

    void Pipeline::createGraphicsPipeline(const std::string& vertFilePath,
                                          const std::string& fragFilePath,
                                          const PipelineConfigInfo& configInfo)
    {
        if(configInfo.pipelineLayout == VK_NULL_HANDLE)
        {
            throw std::runtime_error{"Can't create graphics pipeline: no pipelineLayout provided in configInfo"};
        }

        if(configInfo.renderPass == VK_NULL_HANDLE)
        {
            throw std::runtime_error{"Can't create graphics pipeline: no renderPass provided in configInfo"};
        }

        std::vector<char> vertCode{Pipeline::readFile(vertFilePath)};
        std::vector<char> fragCode{Pipeline::readFile(fragFilePath)};

        createShaderModule(vertCode, &m_vertShaderModule);
        createShaderModule(fragCode, &m_fragShaderModule);

        std::array<VkPipelineShaderStageCreateInfo, 2> shaderStagesInfos{};

        // Vertex Shader
        shaderStagesInfos[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStagesInfos[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
        shaderStagesInfos[0].module = m_vertShaderModule;
        shaderStagesInfos[0].pName = "main";
        shaderStagesInfos[0].flags = 0;
        shaderStagesInfos[0].pNext = nullptr;
        shaderStagesInfos[0].pSpecializationInfo = nullptr;

        // Fragment Shader
        shaderStagesInfos[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStagesInfos[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        shaderStagesInfos[1].module = m_fragShaderModule;
        shaderStagesInfos[1].pName = "main";
        shaderStagesInfos[1].flags = 0;
        shaderStagesInfos[1].pNext = nullptr;
        shaderStagesInfos[1].pSpecializationInfo = nullptr;

        // Vertex Input
        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputInfo.vertexAttributeDescriptionCount = 0;
        vertexInputInfo.pVertexAttributeDescriptions = nullptr;
        vertexInputInfo.vertexBindingDescriptionCount = 0;
        vertexInputInfo.pVertexBindingDescriptions = nullptr;

        // Viewports & Scissors
        VkPipelineViewportStateCreateInfo viewportInfo{};
        viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportInfo.viewportCount = 1;
        viewportInfo.pViewports = &configInfo.viewport;
        viewportInfo.scissorCount = 1;
        viewportInfo.pScissors = &configInfo.scissor;

        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;

        // Programmable Stages
        pipelineInfo.stageCount = static_cast<std::uint32_t>(shaderStagesInfos.size());
        pipelineInfo.pStages = shaderStagesInfos.data();

        // Fixed Function Stages
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &configInfo.inputAssemblyInfo;
        pipelineInfo.pViewportState = &viewportInfo;
        pipelineInfo.pRasterizationState = &configInfo.rasterizationInfo;
        pipelineInfo.pMultisampleState = &configInfo.multisampleInfo;
        pipelineInfo.pColorBlendState = &configInfo.colorBlendInfo;
        pipelineInfo.pDepthStencilState = &configInfo.depthStencilInfo;
        pipelineInfo.pDynamicState = nullptr;

        pipelineInfo.layout = configInfo.pipelineLayout;
        pipelineInfo.renderPass = configInfo.renderPass;
        pipelineInfo.subpass = configInfo.subpass;

        pipelineInfo.basePipelineIndex = -1;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

        if(vkCreateGraphicsPipelines(m_device.device(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr,
                                     &m_graphicsPipeline) != VK_SUCCESS)
        {
            throw std::runtime_error{"Failed to create the graphics pipeline!"};
        }
    }

    void Pipeline::createShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule)
    {
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size();
        createInfo.pCode = reinterpret_cast<const std::uint32_t*>(code.data());

        if(vkCreateShaderModule(m_device.device(), &createInfo, nullptr, shaderModule) != VK_SUCCESS)
        {
            throw std::runtime_error{"Failed to create shader module!"};
        }
    }

    PipelineConfigInfo Pipeline::defaultPipelineConfig(std::uint32_t width, std::uint32_t height)
    {
        PipelineConfigInfo configInfo{};

        // Input Assembly Stage
        configInfo.inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        configInfo.inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        configInfo.inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

        // Viewports
        configInfo.viewport.x = 0.0F;
        configInfo.viewport.y = 0.0F;
        configInfo.viewport.width = static_cast<float>(width);
        configInfo.viewport.height = static_cast<float>(height);
        configInfo.viewport.minDepth = 0.0F;
        configInfo.viewport.maxDepth = 1.0F;

        // Scissors
        configInfo.scissor.offset = {0, 0};
        configInfo.scissor.extent = {width, height};

        // Rasterization Stage
        configInfo.rasterizationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        configInfo.rasterizationInfo.depthClampEnable = VK_FALSE;
        configInfo.rasterizationInfo.rasterizerDiscardEnable = VK_FALSE;
        configInfo.rasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;
        configInfo.rasterizationInfo.lineWidth = 1.0F;
        configInfo.rasterizationInfo.cullMode = VK_CULL_MODE_NONE;
        configInfo.rasterizationInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
        configInfo.rasterizationInfo.depthBiasEnable = VK_FALSE;
        configInfo.rasterizationInfo.depthBiasConstantFactor = 0.0F;  // Optional
        configInfo.rasterizationInfo.depthBiasClamp = 0.0F;           // Optional
        configInfo.rasterizationInfo.depthBiasSlopeFactor = 0.0F;     // Optional

        // Multisample
        configInfo.multisampleInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        configInfo.multisampleInfo.sampleShadingEnable = VK_FALSE;
        configInfo.multisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        configInfo.multisampleInfo.minSampleShading = 1.0F;           // Optional
        configInfo.multisampleInfo.pSampleMask = nullptr;             // Optional
        configInfo.multisampleInfo.alphaToCoverageEnable = VK_FALSE;  // Optional
        configInfo.multisampleInfo.alphaToOneEnable = VK_FALSE;       // Optional

        // Configuration per attached framebuffer
        configInfo.colorBlendAttachment.colorWriteMask =
              VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        configInfo.colorBlendAttachment.blendEnable = VK_FALSE;
        configInfo.colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;   // Optional
        configInfo.colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;  // Optional
        configInfo.colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;              // Optional
        configInfo.colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;   // Optional
        configInfo.colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;  // Optional
        configInfo.colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;              // Optional

        // Contains the global color blending settings
        configInfo.colorBlendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        configInfo.colorBlendInfo.logicOpEnable = VK_FALSE;
        configInfo.colorBlendInfo.logicOp = VK_LOGIC_OP_COPY;  // Optional
        configInfo.colorBlendInfo.attachmentCount = 1;
        configInfo.colorBlendInfo.pAttachments = &configInfo.colorBlendAttachment;
        configInfo.colorBlendInfo.blendConstants[0] = 0.0F;  // Optional
        configInfo.colorBlendInfo.blendConstants[1] = 0.0F;  // Optional
        configInfo.colorBlendInfo.blendConstants[2] = 0.0F;  // Optional
        configInfo.colorBlendInfo.blendConstants[3] = 0.0F;  // Optional

        // Depth values comparison
        configInfo.depthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        configInfo.depthStencilInfo.depthTestEnable = VK_TRUE;
        configInfo.depthStencilInfo.depthWriteEnable = VK_TRUE;
        configInfo.depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS;
        configInfo.depthStencilInfo.depthBoundsTestEnable = VK_FALSE;
        configInfo.depthStencilInfo.minDepthBounds = 0.0F;  // Optional
        configInfo.depthStencilInfo.maxDepthBounds = 1.0F;  // Optional
        configInfo.depthStencilInfo.stencilTestEnable = VK_FALSE;
        configInfo.depthStencilInfo.front = {};  // Optional
        configInfo.depthStencilInfo.back = {};   // Optional

        return configInfo;
    }

    void Pipeline::bind(VkCommandBuffer commandBuffer)
    {
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphicsPipeline);
    }
}
