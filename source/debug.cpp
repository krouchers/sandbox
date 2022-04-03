#include <debug.h>

// std
#include <iostream>

VkBool32 debug_messenger::debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
    void *pUserData)
{
    (void)messageSeverity;
    (void)messageType;
    (void)pCallbackData;
    (void)pUserData;
    std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

    return VK_FALSE;
}

void debug_messenger::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &info)
{
    info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    info.pfnUserCallback = debugCallback;
    info.pUserData = nullptr;
}

void debug_messenger::debugMessengerInit()
{
    VkDebugUtilsMessengerCreateInfoEXT info{};

    populateDebugMessengerCreateInfo(info);

    auto fun = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (fun != nullptr)
    {
        if (fun(instance,
                &info, nullptr, &vkmessanger) != VK_SUCCESS)
            throw std::runtime_error("Failed to create debug messenger");
    }
    else
        throw std::runtime_error("Failed to load vkCreateDebugUtilsMessenger function");
}

debug_messenger::~debug_messenger() noexcept(false)
{
    auto fun = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (fun != nullptr)
        fun(instance, vkmessanger, nullptr);
    else
        throw std::runtime_error("Cant load vkDestroyDebugUtilMessenger");
}

debug_messenger::debug_messenger(VkInstance inst) : instance{inst}
{
    debugMessengerInit();
}