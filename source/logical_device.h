#pragma once
#include <vulkan/vulkan_core.h>
#include <physicalDevice.h>
// std
#include <memory>
class vulkan_context;

class logical_device
{
    VkDevice device;
    vulkan_context &_vk_context;
    VkQueue graphicsQueue;
    VkQueue presentQueue;
    VkQueue transferQueue;

public:
    ~logical_device();
    logical_device(vulkan_context &);
    bool isRequiredExtensionsSupported();
    VkDevice get_vk_handler();
};