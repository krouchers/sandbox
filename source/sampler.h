#pragma once
#include <vulkan/vulkan_core.h>
#include <vulkan_context.h>

class sampler
{
public:
    sampler(vulkan_context *);
    ~sampler();

    VkSampler &get_vk_handle();

private:
    VkSampler _sampler;
    vulkan_context *_vk_context;
};