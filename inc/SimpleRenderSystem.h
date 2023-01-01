#pragma once

#include "Device.h"
#include "GameObject.h"
#include "Model.h"
#include "Pipeline.h"
#include "Camera.h"

// vulkan headers
#include <vulkan/vulkan.h>

// std
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace VE
{
    class SimpleRenderSystem final
    {
    private:  // Private variables
        Device& m_device;
        std::unique_ptr<Pipeline> m_pipeline;
        VkPipelineLayout m_pipelineLayout;

    public:  // Public variables
        void renderGameObjects(VkCommandBuffer commandBuffer, std::vector<GameObject>& gameObjects, const Camera& camera);

    private:  // Private methods
        void createPipelineLayout(void);
        void createPipeline(VkRenderPass renderPass);

    public:  // Public methods
        /*------------------------------------------------------------------*/
        /*                  Don't copy or move my class!!!                  */

        SimpleRenderSystem(const SimpleRenderSystem& copy) = delete;
        SimpleRenderSystem& operator=(const SimpleRenderSystem& copy) = delete;
        SimpleRenderSystem(SimpleRenderSystem&& move) = delete;
        SimpleRenderSystem& operator=(SimpleRenderSystem&& move) = delete;
        /*------------------------------------------------------------------*/

        // Constructor
        SimpleRenderSystem(Device& device, VkRenderPass renderPass);

        // Destructor
        ~SimpleRenderSystem(void);
    };
}
