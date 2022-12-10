#pragma once

#include "Device.h"

// Vulkan headers
#include <vulkan/vulkan.h>

// std
#include <string>
#include <vector>

namespace VE
{
    struct PipelineConfigInfo
    {
        /*------------------------------------------------------------------*/
        /*                       Don't copy my class!!!                     */

        PipelineConfigInfo(const PipelineConfigInfo& copy) = delete;
        PipelineConfigInfo& operator=(const PipelineConfigInfo& copy) = delete;
        /*------------------------------------------------------------------*/

        PipelineConfigInfo(PipelineConfigInfo&& move) = default;
        PipelineConfigInfo& operator=(PipelineConfigInfo&& move) = default;

        // Constructor
        PipelineConfigInfo(void) = default;

        // Destructor
        ~PipelineConfigInfo(void) = default;

        VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo{};
        VkViewport viewport{};
        VkRect2D scissor{};
        VkPipelineRasterizationStateCreateInfo rasterizationInfo{};
        VkPipelineMultisampleStateCreateInfo multisampleInfo{};
        VkPipelineColorBlendAttachmentState colorBlendAttachment{};
        VkPipelineColorBlendStateCreateInfo colorBlendInfo{};
        VkPipelineDepthStencilStateCreateInfo depthStencilInfo{};

        // You have to set them manually
        VkPipelineLayout pipelineLayout{VK_NULL_HANDLE};
        VkRenderPass renderPass{VK_NULL_HANDLE};
        std::uint32_t subpass{};
    };

    class Pipeline final
    {
    private:  // Private variables
        Device& m_device;
        VkPipeline m_graphicsPipeline;
        VkShaderModule m_vertShaderModule;
        VkShaderModule m_fragShaderModule;

    public:  // Public variables

    private:  // Private methods
        static std::vector<char> readFile(const std::string& filePath);

        void createGraphicsPipeline(const std::string& vertFilePath,
                                    const std::string& fragFilePath,
                                    const PipelineConfigInfo& configInfo);

        void createShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule);

    public:  // Public methods
        /*------------------------------------------------------------------*/
        /*                 Don't copy or move my class!!!                   */

        Pipeline(const Pipeline& copy) = delete;
        Pipeline& operator=(const Pipeline& copy) = delete;
        Pipeline(Pipeline&& move) = delete;
        Pipeline& operator=(Pipeline&& move) = delete;
        /*------------------------------------------------------------------*/

        // Constructor
        Pipeline(Device& device, const std::string& vertFilePath, const std::string& fragFilePath,
                 const PipelineConfigInfo& configInfo);

        // Destructor
        ~Pipeline(void);

        static PipelineConfigInfo defaultPipelineConfig(std::uint32_t width, std::uint32_t height);

        void bind(VkCommandBuffer commandBuffer);
    };
}
