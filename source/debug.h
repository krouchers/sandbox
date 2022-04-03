#pragma once
#include <vulkan/vulkan_core.h>

class debug_messenger
{
    VkInstance instance;
    VkDebugUtilsMessengerEXT vkmessanger;
    static VkBool32 debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
        void *pUserData);

public:
    ~debug_messenger() noexcept(false);
    debug_messenger(VkInstance);
    void debugMessengerInit();
    static void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &info);
};