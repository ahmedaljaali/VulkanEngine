#include "Device.h"

// std
#include <cstring>
#include <iostream>
#include <set>
#include <unordered_set>
#include <vector>

namespace VE
{
    // local callback functions
    static VKAPI_ATTR VkBool32 VKAPI_CALL
    debugCallback([[maybe_unused]] VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                  [[maybe_unused]] VkDebugUtilsMessageTypeFlagsEXT messageType,
                  const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                  [[maybe_unused]] void* pUserData)
    {
        std::cerr << "\nValidation layer: " << pCallbackData->pMessage << '\n';

        return VK_FALSE;
    }

    static VkResult CreateDebugUtilsMessengerEXT(VkInstance instance,
                                                 const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                                                 const VkAllocationCallbacks* pAllocator,
                                                 VkDebugUtilsMessengerEXT* pDebugMessenger)
    {
        auto func{reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
              vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"))};

        if(func)
        {
            return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
        }
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }

    static void DestroyDebugUtilsMessengerEXT(VkInstance instance,
                                              VkDebugUtilsMessengerEXT debugMessenger,
                                              const VkAllocationCallbacks* pAllocator)
    {
        auto func{reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
              vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"))};

        if(func)
        {
            func(instance, debugMessenger, pAllocator);
        }
    }

    // class member functions
    Device::Device(Window& window)
        : m_instance{},
          m_debugMessenger{},
          m_physicalDevice{},
          m_window{window},
          m_commandPool{},
          m_device{},
          m_surface{},
          m_graphicsQueue{},
          m_presentQueue{},
          m_validationLayers{"VK_LAYER_KHRONOS_validation"},
          m_deviceExtensions{VK_KHR_SWAPCHAIN_EXTENSION_NAME},
          m_properties{}
    {
        createInstance();
        setupDebugMessenger();
        createSurface();
        pickPhysicalDevice();
        createLogicalDevice();
        createCommandPool();
    }

    Device::~Device(void)
    {
        vkDestroyCommandPool(m_device, m_commandPool, nullptr);
        vkDestroyDevice(m_device, nullptr);

        if(m_enableValidationLayers)
        {
            DestroyDebugUtilsMessengerEXT(m_instance, m_debugMessenger, nullptr);
        }

        vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
        vkDestroyInstance(m_instance, nullptr);
    }

    void Device::createInstance(void)
    {
        if(m_enableValidationLayers && !checkValidationLayerSupport())
        {
            throw std::runtime_error{"Validation layers requested, but not available!"};
        }

        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "VulkanEngine";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "No Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_3;

        VkInstanceCreateInfo instanceCreateInfo{};
        instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        instanceCreateInfo.pApplicationInfo = &appInfo;

        // Global extensions
        std::vector<const char*> extensions{getRequiredExtensions()};
        instanceCreateInfo.enabledExtensionCount = static_cast<std::uint32_t>(extensions.size());
        instanceCreateInfo.ppEnabledExtensionNames = extensions.data();

        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
        if(m_enableValidationLayers)
        {
            // Global validation layers
            instanceCreateInfo.enabledLayerCount = static_cast<std::uint32_t>(m_validationLayers.size());
            instanceCreateInfo.ppEnabledLayerNames = m_validationLayers.data();

            // Instance creation/destruction debug
            populateDebugMessengerCreateInfo(debugCreateInfo);
            instanceCreateInfo.pNext = reinterpret_cast<VkDebugUtilsMessengerCreateInfoEXT*>(&debugCreateInfo);
        }
        else
        {
            instanceCreateInfo.enabledLayerCount = 0;
            instanceCreateInfo.pNext = nullptr;
        }

        if(vkCreateInstance(&instanceCreateInfo, nullptr, &m_instance) != VK_SUCCESS)
        {
            throw std::runtime_error{"Failed to create instance!"};
        }

        hasGflwRequiredInstanceExtensions();
    }

    void Device::pickPhysicalDevice(void)
    {
        std::uint32_t deviceCount{};
        vkEnumeratePhysicalDevices(m_instance, &deviceCount, nullptr);

        if(!deviceCount)
        {
            throw std::runtime_error{"Failed to find GPUs with Vulkan support!"};
        }

        std::cout << "Device count: " << deviceCount << '\n';
        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(m_instance, &deviceCount, devices.data());

        for(const auto& device : devices)
        {
            if(isDeviceSuitable(device))
            {
                m_physicalDevice = device;
                break;
            }
        }

        if(m_physicalDevice == VK_NULL_HANDLE)
        {
            throw std::runtime_error{"Failed to find a suitable GPU!"};
        }

        vkGetPhysicalDeviceProperties(m_physicalDevice, &m_properties);
        std::cout << "physical device: " << static_cast<char*>(m_properties.deviceName) << '\n';
    }

    void Device::createLogicalDevice(void)
    {
        QueueFamilyIndices indices{findQueueFamilies(m_physicalDevice)};

        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        std::set<std::uint32_t> uniqueQueueFamilies{indices.graphicsFamily.value(), indices.presentFamily.value()};

        for(float queuePriority{}; std::uint32_t queueFamily : uniqueQueueFamilies)
        {
            VkDeviceQueueCreateInfo queueCreateInfo{};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = queueFamily;
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;

            queueCreateInfos.push_back(queueCreateInfo);
        }

        VkPhysicalDeviceFeatures deviceFeatures{};
        deviceFeatures.samplerAnisotropy = VK_TRUE;

        VkDeviceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

        createInfo.queueCreateInfoCount = static_cast<std::uint32_t>(queueCreateInfos.size());
        createInfo.pQueueCreateInfos = queueCreateInfos.data();

        createInfo.pEnabledFeatures = &deviceFeatures;
        createInfo.enabledExtensionCount = static_cast<std::uint32_t>(m_deviceExtensions.size());
        createInfo.ppEnabledExtensionNames = m_deviceExtensions.data();

        // might not really be necessary anymore because device specific validation layers
        // have been deprecated
        if(m_enableValidationLayers)
        {
            createInfo.enabledLayerCount = static_cast<std::uint32_t>(m_validationLayers.size());
            createInfo.ppEnabledLayerNames = m_validationLayers.data();
        }
        else
        {
            createInfo.enabledLayerCount = 0;
        }

        if(vkCreateDevice(m_physicalDevice, &createInfo, nullptr, &m_device) != VK_SUCCESS)
        {
            throw std::runtime_error{"Failed to create logical device!"};
        }

        vkGetDeviceQueue(m_device, indices.graphicsFamily.value(), 0, &m_graphicsQueue);
        vkGetDeviceQueue(m_device, indices.presentFamily.value(), 0, &m_presentQueue);
    }

    void Device::createCommandPool(void)
    {
        QueueFamilyIndices queueFamilyIndices{findPhysicalQueueFamilies()};

        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
        poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

        if(vkCreateCommandPool(m_device, &poolInfo, nullptr, &m_commandPool) != VK_SUCCESS)
        {
            throw std::runtime_error{"Failed to create command pool!"};
        }
    }

    void Device::createSurface(void) { m_window.createWindowSurface(m_instance, &m_surface); }

    bool Device::isDeviceSuitable(VkPhysicalDevice phyDevice)
    {
        QueueFamilyIndices indices{findQueueFamilies(phyDevice)};

        bool extensionsSupported{checkDeviceExtensionSupport(phyDevice)};

        bool swapChainAdequate{};
        if(extensionsSupported)
        {
            SwapChainSupportDetails swapChainSupport{querySwapChainSupport(phyDevice)};
            swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
        }

        VkPhysicalDeviceFeatures supportedFeatures;
        vkGetPhysicalDeviceFeatures(phyDevice, &supportedFeatures);

        return indices.isComplete() && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
    }

    void Device::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
    {
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity =
              VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
              VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo.pfnUserCallback = debugCallback;
        createInfo.pUserData = nullptr;  // Optional
    }

    void Device::setupDebugMessenger(void)
    {
        if(!m_enableValidationLayers)
        {
            return;
        }
        VkDebugUtilsMessengerCreateInfoEXT createInfo{};
        populateDebugMessengerCreateInfo(createInfo);
        if(CreateDebugUtilsMessengerEXT(m_instance, &createInfo, nullptr, &m_debugMessenger) != VK_SUCCESS)
        {
            throw std::runtime_error{"Failed to set up debug messenger!"};
        }
    }

    bool Device::checkValidationLayerSupport(void)
    {
        std::uint32_t layerCount{};
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

        for(const char* layerName : m_validationLayers)
        {
            bool layerFound{};

            for(const auto& layerProperties : availableLayers)
            {
                if(strcmp(layerName, static_cast<const char*>(layerProperties.layerName)) == 0)
                {
                    layerFound = true;
                    break;
                }
            }

            if(!layerFound)
            {
                return false;
            }
        }

        return true;
    }

    std::vector<const char*> Device::getRequiredExtensions(void) const
    {
        std::vector<const char*> extensions{Window::getInstanceExtensions()};

        if(m_enableValidationLayers)
        {
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        return extensions;
    }

    void Device::hasGflwRequiredInstanceExtensions(void)
    {
        std::uint32_t extensionCount{};
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
        std::vector<VkExtensionProperties> extensions(extensionCount);
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

        std::cout << "available extensions:" << '\n';
        std::unordered_set<std::string> available;
        for(const auto& extension : extensions)
        {
            std::cout << "\t" << static_cast<const char*>(extension.extensionName)
                      << ", VERSION: " << extension.specVersion << '\n';
            available.insert(static_cast<const char*>(extension.extensionName));
        }

        std::cout << "required extensions:" << '\n';
        auto requiredExtensions{getRequiredExtensions()};
        for(const auto& required : requiredExtensions)
        {
            std::cout << "\t" << required << '\n';
            if(available.find(required) == available.end())
            {
                throw std::runtime_error{"Missing required glfw extension"};
            }
        }
    }

    bool Device::checkDeviceExtensionSupport(VkPhysicalDevice phyDevice)
    {
        std::uint32_t extensionCount{};
        vkEnumerateDeviceExtensionProperties(phyDevice, nullptr, &extensionCount, nullptr);

        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(phyDevice, nullptr, &extensionCount, availableExtensions.data());

        std::set<std::string> requiredExtensions(m_deviceExtensions.begin(), m_deviceExtensions.end());

        for(const auto& extension : availableExtensions)
        {
            requiredExtensions.erase(static_cast<const char*>(extension.extensionName));
        }

        return requiredExtensions.empty();
    }

    QueueFamilyIndices Device::findQueueFamilies(VkPhysicalDevice phyDevice)
    {
        QueueFamilyIndices indices;

        std::uint32_t queueFamilyCount{};
        vkGetPhysicalDeviceQueueFamilyProperties(phyDevice, &queueFamilyCount, nullptr);

        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(phyDevice, &queueFamilyCount, queueFamilies.data());

        for(std::uint32_t familyIndex{}; const auto& queueFamily : queueFamilies)
        {
            if(queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
            {
                indices.graphicsFamily = familyIndex;
            }

            VkBool32 presentSupport{};
            vkGetPhysicalDeviceSurfaceSupportKHR(phyDevice, familyIndex, m_surface, &presentSupport);
            if(queueFamily.queueCount > 0 && presentSupport)
            {
                indices.presentFamily = familyIndex;
            }

            if(indices.isComplete())
            {
                break;
            }

            ++familyIndex;
        }

        return indices;
    }

    SwapChainSupportDetails Device::querySwapChainSupport(VkPhysicalDevice phyDevice)
    {
        SwapChainSupportDetails details;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(phyDevice, m_surface, &details.capabilities);

        std::uint32_t formatCount{};
        vkGetPhysicalDeviceSurfaceFormatsKHR(phyDevice, m_surface, &formatCount, nullptr);

        if(formatCount)
        {
            details.formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(phyDevice, m_surface, &formatCount, details.formats.data());
        }

        std::uint32_t presentModeCount{};
        vkGetPhysicalDeviceSurfacePresentModesKHR(phyDevice, m_surface, &presentModeCount, nullptr);

        if(presentModeCount)
        {
            details.presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(
                  phyDevice, m_surface, &presentModeCount, details.presentModes.data());
        }
        return details;
    }

    VkFormat Device::findSupportedFormat(const std::vector<VkFormat>& candidates,
                                         VkImageTiling tiling,
                                         VkFormatFeatureFlags features)
    {
        for(VkFormat format : candidates)
        {
            VkFormatProperties props;
            vkGetPhysicalDeviceFormatProperties(m_physicalDevice, format, &props);

            if((tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features)) ||
               (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features)) == features)
            {
                return format;
            }
        }
        throw std::runtime_error{"Failed to find supported format!"};
    }

    std::uint32_t Device::findMemoryType(std::uint32_t typeFilter, VkMemoryPropertyFlags properties)
    {
        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(m_physicalDevice, &memProperties);
        for(std::uint32_t propertyIndex{}; propertyIndex < memProperties.memoryTypeCount; ++propertyIndex)
        {
            if((typeFilter & (1 << propertyIndex)) &&
               (memProperties.memoryTypes[propertyIndex].propertyFlags & properties) == properties)
            {
                return propertyIndex;
            }
        }

        throw std::runtime_error{"Failed to find suitable memory type!"};
    }

    void Device::createBuffer(VkDeviceSize size,
                              VkBufferUsageFlags usage,
                              VkMemoryPropertyFlags properties,
                              VkBuffer& buffer,
                              VkDeviceMemory& bufferMemory)
    {
        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = size;
        bufferInfo.usage = usage;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if(vkCreateBuffer(m_device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
        {
            throw std::runtime_error{"Failed to create vertex buffer!"};
        }

        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(m_device, buffer, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

        if(vkAllocateMemory(m_device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS)
        {
            throw std::runtime_error{"Failed to allocate vertex buffer memory!"};
        }

        vkBindBufferMemory(m_device, buffer, bufferMemory, 0);
    }

    VkCommandBuffer Device::beginSingleTimeCommands(void)
    {
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = m_commandPool;
        allocInfo.commandBufferCount = 1;

        VkCommandBuffer commandBuffer{};
        vkAllocateCommandBuffers(m_device, &allocInfo, &commandBuffer);

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer(commandBuffer, &beginInfo);
        return commandBuffer;
    }

    void Device::endSingleTimeCommands(VkCommandBuffer commandBuffer)
    {
        vkEndCommandBuffer(commandBuffer);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(m_graphicsQueue);

        vkFreeCommandBuffers(m_device, m_commandPool, 1, &commandBuffer);
    }

    void Device::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
    {
        VkCommandBuffer commandBuffer{beginSingleTimeCommands()};

        VkBufferCopy copyRegion{};
        copyRegion.srcOffset = 0;  // Optional
        copyRegion.dstOffset = 0;  // Optional
        copyRegion.size = size;
        vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

        endSingleTimeCommands(commandBuffer);
    }

    void Device::copyBufferToImage(
          VkBuffer buffer, VkImage image, std::uint32_t width, std::uint32_t height, std::uint32_t layerCount)
    {
        VkCommandBuffer commandBuffer{beginSingleTimeCommands()};

        VkBufferImageCopy region{};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;

        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = layerCount;

        region.imageOffset = {0, 0, 0};
        region.imageExtent = {width, height, 1};

        vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
        endSingleTimeCommands(commandBuffer);
    }

    void Device::createImageWithInfo(const VkImageCreateInfo& imageInfo,
                                     VkMemoryPropertyFlags properties,
                                     VkImage& image,
                                     VkDeviceMemory& imageMemory)
    {
        if(vkCreateImage(m_device, &imageInfo, nullptr, &image) != VK_SUCCESS)
        {
            throw std::runtime_error{"Failed to create image!"};
        }

        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(m_device, image, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

        if(vkAllocateMemory(m_device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS)
        {
            throw std::runtime_error{"Failed to allocate image memory!"};
        }

        if(vkBindImageMemory(m_device, image, imageMemory, 0) != VK_SUCCESS)
        {
            throw std::runtime_error{"Failed to bind image memory!"};
        }
    }
}
