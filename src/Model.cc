#include "Model.h"

// std
#include <array>
#include <cstddef>
#include <cstring>
#include <stdexcept>

namespace VE
{
    // Constructor
    Model::Model(Device& device, const std::vector<Vertex>& vertices)
        : m_device{device}, m_vertexBuffer{}, m_vertexBufferMemory{}, m_vertexCount{}
    {
        createVertexBuffers(vertices);
    }

    // Destructor
    Model::~Model(void)
    {
        vkDestroyBuffer(m_device.device(), m_vertexBuffer, nullptr);
        vkFreeMemory(m_device.device(), m_vertexBufferMemory, nullptr);
    }


    void Model::bind(VkCommandBuffer commandBuffer)
    {
        std::array<VkBuffer, 1> buffers{m_vertexBuffer};
        std::array<VkDeviceSize, 1> offsets{0};

        vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers.data(), offsets.data());
    }

    void Model::draw(VkCommandBuffer commandBuffer) { vkCmdDraw(commandBuffer, m_vertexCount, 1, 0, 0); }

    void Model::createVertexBuffers(const std::vector<Vertex>& vertices)
    {
        m_vertexCount = static_cast<std::uint32_t>(vertices.size());

        if(m_vertexCount < 3)
        {
            throw std::runtime_error{"At least we should have three vertices!"};
        }

        std::size_t bufferSize{sizeof(vertices[0]) * m_vertexCount};

        m_device.createBuffer(bufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                              VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
                              m_vertexBuffer, m_vertexBufferMemory);

        void* data{nullptr};
        vkMapMemory(m_device.device(), m_vertexBufferMemory, 0, bufferSize, 0, &data);
        memcpy(data, vertices.data(), static_cast<std::size_t>(bufferSize));
        vkUnmapMemory(m_device.device(), m_vertexBufferMemory);
    }

    std::vector<VkVertexInputBindingDescription> Model::Vertex::getBindingDescriptions(void)
    {
        std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
        bindingDescriptions[0].binding = 0;
        bindingDescriptions[0].stride = sizeof(Model::Vertex);
        bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return bindingDescriptions;
    }

    std::vector<VkVertexInputAttributeDescription> Model::Vertex::getAttributeDescriptions(void)
    {
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions(2);

        // position
        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Model::Vertex, position);

        // position
        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Model::Vertex, color);

        return attributeDescriptions;
    }
}
