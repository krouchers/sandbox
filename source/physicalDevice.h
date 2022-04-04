#pragma once
#include <vulkan/vulkan_core.h>
// std
#include <vector>
#include <optional>

class vulkan_context;
class physicalDevice
{
    VkPhysicalDevice _physicalDevice;
    vulkan_context &_vk_context;
    std::vector<VkPhysicalDevice> physicalDevices;
    std::vector<VkPhysicalDeviceFeatures> physicalDeviceFeatures;
    std::vector<VkPhysicalDeviceProperties> physicalDeviceProperties;
    std::vector<VkQueueFamilyProperties> physicalDeviceFamilyqueueProperties;
    std::vector<const char *> requiredExtentionNames = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    std::vector<VkExtensionProperties> supportedExtentionNames;

    bool isDeviceSuitable(VkPhysicalDevice) noexcept;

public:
    physicalDevice(const physicalDevice &) = delete;
    physicalDevice(const physicalDevice &&) = delete;
    physicalDevice &operator=(const physicalDevice &) = delete;
    struct queueFamilies
    {
        std::optional<uint32_t> graphicFamily;
        std::optional<uint32_t> presentFamily;
        std::optional<uint32_t> transferFamily;

        bool isComplete()
        {
            return graphicFamily && presentFamily && transferFamily;
        }
    } queueFamilies;

    struct swapchainDetails
    {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> surface_formats;
        std::vector<VkPresentModeKHR> surface_present_modes;
        bool isComplete(){
            return !surface_formats.empty() && !surface_present_modes.empty();
        } 
    } _swapchainSupport;

    bool isRequiredExtensionsSupported(VkPhysicalDevice dev);
    void populate_swapchain_details(VkPhysicalDevice);
    void printPhysicalDeviceProperties() noexcept;
    void getDeviceProperties(VkPhysicalDevice device, VkPhysicalDeviceProperties *phDeviceProperties) noexcept;
    void getDeviceFeatures(VkPhysicalDevice device, VkPhysicalDeviceFeatures *phDeviceFeatures) noexcept;
    physicalDevice(vulkan_context &);
    void pickPhysicalDevice() noexcept;
    void findQueueFamilies(VkPhysicalDevice) noexcept;
    VkPhysicalDevice getVkHandler() noexcept;

    std::vector<const char *> &get_required_extentions();
};