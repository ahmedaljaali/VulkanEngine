#pragma once

#include "Window.h"

// Vulkan headers
#include <vulkan/vulkan.h>

// std
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace VE
{
    struct SwapChainSupportDetails
    {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };

    struct QueueFamilyIndices
    {
        std::optional<std::uint32_t> graphicsFamily;
        std::optional<std::uint32_t> presentFamily;
        [[nodiscard]] bool isComplete() const { return graphicsFamily.has_value() && presentFamily.has_value(); }
    };

    class Device final
    {
    private:  // Private variables
#ifdef NDEBUG
        const bool m_enableValidationLayers{};
#else
        const bool m_enableValidationLayers{true};
#endif

        VkInstance m_instance;
        VkDebugUtilsMessengerEXT m_debugMessenger;
        VkPhysicalDevice m_physicalDevice;
        Window& m_window;
        VkCommandPool m_commandPool;

        VkDevice m_device;
        VkSurfaceKHR m_surface;
        VkQueue m_graphicsQueue;
        VkQueue m_presentQueue;
        const std::vector<const char*> m_validationLayers;
        const std::vector<const char*> m_deviceExtensions;
        VkPhysicalDeviceProperties m_properties;

    public:  // Public variables

    private:  // Private methods
        void createInstance(void);
        void setupDebugMessenger(void);
        void createSurface(void);
        void pickPhysicalDevice(void);
        void createLogicalDevice(void);
        void createCommandPool(void);

        /*------------------------------------------------------------------*/
        /*                         Helper Functions                         */

        bool isDeviceSuitable(VkPhysicalDevice phyDevice);
        [[nodiscard]] std::vector<const char*> getRequiredExtensions() const;
        bool checkValidationLayerSupport();
        QueueFamilyIndices findQueueFamilies(VkPhysicalDevice phyDevice);
        void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
        void hasGflwRequiredInstanceExtensions();
        bool checkDeviceExtensionSupport(VkPhysicalDevice phyDevice);
        SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice phyDevice);
        /*------------------------------------------------------------------*/

    public:  // Public methods
        /*------------------------------------------------------------------*/
        /*                  Don't copy or move my class!!!                  */

        Device(const Device& copy) = delete;
        Device& operator=(const Device& copy) = delete;
        Device(Device&& move) = delete;
        Device& operator=(Device&& move) = delete;
        /*------------------------------------------------------------------*/

        // Constructor
        Device(Window& window);

        // Destructor
        ~Device(void);

        /*------------------------------------------------------------------*/
        /*                             Getters                              */

        VkCommandPool getCommandPool(void) { return m_commandPool; }
        VkDevice device(void) { return m_device; }

        VkSurfaceKHR surface(void) { return m_surface; }
        VkQueue graphicsQueue(void) { return m_graphicsQueue; }

        VkQueue presentQueue(void) { return m_presentQueue; }
        SwapChainSupportDetails getSwapChainSupport(void) { return querySwapChainSupport(m_physicalDevice); }

        std::uint32_t findMemoryType(std::uint32_t typeFilter, VkMemoryPropertyFlags properties);
        QueueFamilyIndices findPhysicalQueueFamilies(void) { return findQueueFamilies(m_physicalDevice); }

        VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates,
                                     VkImageTiling tiling,
                                     VkFormatFeatureFlags features);

        [[nodiscard]] const VkPhysicalDeviceProperties& getPhysicalDeviceProperties(void) const { return m_properties; }
        /*------------------------------------------------------------------*/

        /*------------------------------------------------------------------*/
        /*                     Buffer Helper Functions                      */

        // Buffer Helper Functions
        void createBuffer(std::size_t size,
                          VkBufferUsageFlags usage,
                          VkMemoryPropertyFlags properties,
                          VkBuffer& buffer,
                          VkDeviceMemory& bufferMemory);
        VkCommandBuffer beginSingleTimeCommands(void);

        void endSingleTimeCommands(VkCommandBuffer commandBuffer);
        void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

        void copyBufferToImage(
              VkBuffer buffer, VkImage image, std::uint32_t width, std::uint32_t height, std::uint32_t layerCount);
        void createImageWithInfo(const VkImageCreateInfo& imageInfo,
                                 VkMemoryPropertyFlags properties,
                                 VkImage& image,
                                 VkDeviceMemory& imageMemory);
        /*------------------------------------------------------------------*/
    };
}
