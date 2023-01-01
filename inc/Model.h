#pragma once

#include "Device.h"

// Vulkan headers
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

// glm
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

// std
#include <cstdint>
#include <vector>

namespace VE
{
    class Model final
    {
    private:  // Private variables
        Device& m_device;
        VkBuffer m_vertexBuffer;
        VkDeviceMemory m_vertexBufferMemory;
        std::uint32_t m_vertexCount;

    public:  // Public variables
        struct Vertex
        {
            glm::vec3 position;
            glm::vec3 color;

            static std::vector<VkVertexInputBindingDescription> getBindingDescriptions(void);
            static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions(void);
        };

    private:  // Private methods
        void createVertexBuffers(const std::vector<Vertex>& vertices);

    public:  // Public methods
        /*------------------------------------------------------------------*/
        /*                       Don't copy my class                        */

        Model(const Model& copy) = delete;
        Model& operator=(const Model& copy) = delete;
        Model(const Model&& move) = delete;
        Model& operator=(const Model&& move) = delete;
        /*------------------------------------------------------------------*/


        // Constructor
        Model(Device& device, const std::vector<Vertex>& vertices);

        // Destructor
        ~Model(void);

        void bind(VkCommandBuffer commandBuffer);
        void draw(VkCommandBuffer commandBuffer);
    };
}
