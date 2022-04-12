#include <logical_device.h>
#include <vulkan_context.h>
#include <swapchain.h>
#include <vulkan/vulkan.h>

// std
#include <stdexcept>
#include <set>
#include <cstring>
#include <string>
#include <iostream>

class vulkan_context;

logical_device::logical_device(vulkan_context &vk_context) : _vk_context{vk_context}
{
    std::set<uint32_t> uniqueQueueFamilies{
        _vk_context.get_physical_device().queueFamilies.graphicFamily.value(),
        _vk_context.get_physical_device().queueFamilies.presentFamily.value(),
        _vk_context.get_physical_device().queueFamilies.transferFamily.value()};
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    float queuePriority = 1.0f;
    for (auto queueFamily : uniqueQueueFamilies)
    {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }
    VkDeviceCreateInfo createInfo{};
    VkPhysicalDeviceFeatures enabled_features{};
    enabled_features.samplerAnisotropy = VK_TRUE;
    createInfo.pEnabledFeatures = &enabled_features;
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.ppEnabledExtensionNames = _vk_context.get_physical_device().get_required_extentions().data();
    createInfo.enabledExtensionCount = static_cast<uint32_t>(_vk_context.get_physical_device().get_required_extentions().size());
    if (_vk_context.getExtAndLayersInfo().layersCount != 0)
    {
        createInfo.enabledLayerCount = _vk_context.getExtAndLayersInfo().layersCount;
        createInfo.ppEnabledLayerNames = _vk_context.getExtAndLayersInfo().ppLayers;
    }

    if (vkCreateDevice(_vk_context.get_physical_device().getVkHandler(), &createInfo, nullptr, &device) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create logical device");
    }

    vkGetDeviceQueue(device, _vk_context.get_physical_device().queueFamilies.graphicFamily.value(), 0, _vk_context.get_swapchain().get_grapchic_queue());
    vkGetDeviceQueue(device, _vk_context.get_physical_device().queueFamilies.presentFamily.value(), 0, _vk_context.get_swapchain().get_present_queue());
    vkGetDeviceQueue(device, _vk_context.get_physical_device().queueFamilies.transferFamily.value(), 0, _vk_context.get_swapchain().get_transfer_queue());
#ifdef DEBUG
    std::cout << "Logical device is created" << std::endl;
#endif
}

logical_device::~logical_device()
{
    vkDestroyDevice(device, nullptr);
}

VkDevice logical_device::get_vk_handler()
{
    return device;
}