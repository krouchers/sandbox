#include <physicalDevice.h>
#include <vulkan_context.h>
#include<swapchain.h>

// std
#include <stdexcept>
#include <iostream>
#include <set>

void physicalDevice::pickPhysicalDevice() noexcept
{
    for (const auto &phDev : physicalDevices)
    {
        if (isDeviceSuitable(phDev))
        {
            _physicalDevice = phDev;
#ifdef DEBUG
            std::cout << "Device is picked" << std::endl;
#endif
            break;
        }
    }
}

bool physicalDevice::isDeviceSuitable(VkPhysicalDevice dev) noexcept
{
    VkPhysicalDeviceProperties properties{};
    VkPhysicalDeviceFeatures features{};
    getDeviceProperties(dev, &properties);
    getDeviceFeatures(dev, &features);
    findQueueFamilies(dev);
    populate_swapchain_details(dev);
    bool isRequiredExtentionsSupported = isRequiredExtensionsSupported(dev);
    return features.geometryShader && queueFamilies.isComplete() && _swapchainSupport.isComplete() && isRequiredExtentionsSupported;
}

void physicalDevice::populate_swapchain_details(VkPhysicalDevice dev)
{
    VkSurfaceCapabilitiesKHR capabilities{};
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
        dev,
        _vk_context.get_swapchain().get_surface(),
        &capabilities);
    _swapchainSupport.capabilities = capabilities;

    uint32_t count;
    vkGetPhysicalDeviceSurfaceFormatsKHR(dev, _vk_context.get_swapchain().get_surface(), &count, nullptr);
    _swapchainSupport.surface_formats = std::vector<VkSurfaceFormatKHR>(count);
    vkGetPhysicalDeviceSurfaceFormatsKHR(dev, _vk_context.get_swapchain().get_surface(), &count, _swapchainSupport.surface_formats.data());

    count = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(dev, _vk_context.get_swapchain().get_surface(), &count, nullptr);
    _swapchainSupport.surface_present_modes = std::vector<VkPresentModeKHR>(count);
    vkGetPhysicalDeviceSurfacePresentModesKHR(dev, _vk_context.get_swapchain().get_surface(), &count, _swapchainSupport.surface_present_modes.data());
}

void physicalDevice::printPhysicalDeviceProperties() noexcept
{
    for (const auto &property : physicalDeviceProperties)
    {
        std::cout << property.deviceName;
    }
}

physicalDevice::physicalDevice(vulkan_context &vk_context) : _vk_context{vk_context}
{
    uint32_t count;
    vkEnumeratePhysicalDevices(vk_context.get_instance(), &count, nullptr);
    if (count == 0)
        std::runtime_error("There is no available physical devices");
    else
    {
        physicalDevices = std::vector<VkPhysicalDevice>(count);
        vkEnumeratePhysicalDevices(vk_context.get_instance(), &count, physicalDevices.data());
    }

    pickPhysicalDevice();
}

void physicalDevice::getDeviceProperties(VkPhysicalDevice dev, VkPhysicalDeviceProperties *phDeviceProperties) noexcept
{
    vkGetPhysicalDeviceProperties(dev, phDeviceProperties);
    physicalDeviceProperties.push_back(*phDeviceProperties);
}
void physicalDevice::getDeviceFeatures(VkPhysicalDevice dev, VkPhysicalDeviceFeatures *phDeviceFeatures) noexcept
{
    vkGetPhysicalDeviceFeatures(dev, phDeviceFeatures);
    physicalDeviceFeatures.push_back(*phDeviceFeatures);
}

void physicalDevice::findQueueFamilies(VkPhysicalDevice dev) noexcept
{
    uint32_t count{};
    vkGetPhysicalDeviceQueueFamilyProperties(dev, &count, nullptr);
    physicalDeviceFamilyqueueProperties = std::vector<VkQueueFamilyProperties>(count);
    vkGetPhysicalDeviceQueueFamilyProperties(dev, &count, physicalDeviceFamilyqueueProperties.data());

    size_t i = 0;
    for (const auto &familyProperties : physicalDeviceFamilyqueueProperties)
    {
        if (familyProperties.queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            queueFamilies.graphicFamily = i;
            break;
        }
        ++i;
    }

    VkBool32 present_support = false;
    for (i = 0; i < physicalDeviceFamilyqueueProperties.size(); ++i)
    {
        vkGetPhysicalDeviceSurfaceSupportKHR(dev, static_cast<uint32_t>(i), _vk_context.get_swapchain().get_surface(), &present_support);
        if (present_support)
        {
            queueFamilies.presentFamily = i;
        }
    }
    i = 0;
    for (const auto &familyProperties : physicalDeviceFamilyqueueProperties)
    {
        if ((familyProperties.queueFlags & VK_QUEUE_TRANSFER_BIT) && !(familyProperties.queueFlags & VK_QUEUE_GRAPHICS_BIT))
        {
            queueFamilies.transferFamily = i;
            break;
        }
        ++i;
    }
#ifdef DEBUG
    std::cout << "graphic family index: " << queueFamilies.graphicFamily.value() << std::endl;
    std::cout << "present family index: " << queueFamilies.presentFamily.value() << std::endl;
    std::cout << "transfer family index: " << queueFamilies.transferFamily.value() << std::endl;
#endif
}

VkPhysicalDevice physicalDevice::getVkHandler() noexcept
{
    return _physicalDevice;
}

bool physicalDevice::isRequiredExtensionsSupported(VkPhysicalDevice dev)
{
    uint32_t count{};
    vkEnumerateDeviceExtensionProperties(dev, nullptr, &count, nullptr);
    supportedExtentionNames = std::vector<VkExtensionProperties>(count);
    vkEnumerateDeviceExtensionProperties(dev, nullptr, &count, supportedExtentionNames.data());

    std::set<std::string> requiredExtentions{requiredExtentionNames.begin(), requiredExtentionNames.end()};

    for (const auto &supportedExtention : supportedExtentionNames)
    {
        requiredExtentions.erase(std::string(supportedExtention.extensionName));
    }

    return requiredExtentions.empty();
}

std::vector<const char *> &physicalDevice::get_required_extentions()
{
    return requiredExtentionNames;
}
