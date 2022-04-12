#include <swapchain.h>
#include <graphic_pipeline.h>
#include <buffer.h>
#include<interface.h>

// std
#include <stdexcept>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <vector>
#include <algorithm>
#include <stdexcept>

void swapchain::create_surface()
{
    if (glfwCreateWindowSurface(_vk_context.get_instance(), _vk_context.getWindow().getglfwWindow(), nullptr, &_surface) != VK_SUCCESS)
    {
        throw std::runtime_error("faild to create window surface");
    }
}

swapchain::swapchain(vulkan_context &context) : _vk_context{context}
{
}

VkSurfaceKHR swapchain::get_surface()
{
    return _surface;
}

VkSurfaceFormatKHR swapchain::get_surface_format()
{
    return _surface_format;
}

VkSurfaceFormatKHR swapchain::choose_surface_format(std::vector<VkSurfaceFormatKHR> formats)
{
    for (const auto &format : formats)
    {
        if (format.format == VK_FORMAT_R8G8B8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
#ifdef DEBUG
            std::cout << "required format is found" << std::endl;
#endif
            return format;
        }
    }
#ifdef DEBUG
    std::cout << "default format is picked" << std::endl;
#endif
    return formats[0];
}

VkPresentModeKHR swapchain::choose_present_mode(std::vector<VkPresentModeKHR> mods)
{

    for (const auto &mod : mods)
    {
        if (mod == VK_PRESENT_MODE_MAILBOX_KHR)
        {
#ifdef DEBUG
            std::cout << "required present mode is found" << std::endl;
#endif
            return mod;
        }
    }
#ifdef DEBUG
    std::cout << "default present mode is picked" << std::endl;
#endif
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D swapchain::choose_extent(VkSurfaceCapabilitiesKHR capabilities)
{
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
        return capabilities.currentExtent;
    int height, width;
    glfwGetFramebufferSize(_vk_context.getWindow().getglfwWindow(), &width, &height);

    VkExtent2D actual_extent{
        std::clamp(static_cast<uint32_t>(width), capabilities.minImageExtent.width, capabilities.maxImageExtent.width),
        std::clamp(static_cast<uint32_t>(height), capabilities.minImageExtent.height, capabilities.maxImageExtent.height)};
    return actual_extent;
}

VkExtent2D swapchain::get_extent()
{
    return _extent;
}
void swapchain::create_swapchain()
{
    _surface_format = choose_surface_format(_vk_context.get_physical_device()._swapchainSupport.surface_formats);
    VkPresentModeKHR present_mode = choose_present_mode(_vk_context.get_physical_device()._swapchainSupport.surface_present_modes);
    _extent = choose_extent(_vk_context.get_physical_device()._swapchainSupport.capabilities);

    uint32_t image_count = std::clamp(
        _vk_context.get_physical_device()._swapchainSupport.capabilities.minImageCount + 1,
        _vk_context.get_physical_device()._swapchainSupport.capabilities.minImageCount,
        _vk_context.get_physical_device()._swapchainSupport.capabilities.maxImageCount);

    uint32_t family_indices[] = {_vk_context.get_physical_device().queueFamilies.graphicFamily.value(),
                                 _vk_context.get_physical_device().queueFamilies.presentFamily.value()};
    VkSwapchainCreateInfoKHR create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    create_info.surface = _vk_context.get_swapchain().get_surface();
    create_info.minImageCount = image_count;
    create_info.imageFormat = _surface_format.format;
    create_info.imageColorSpace = _surface_format.colorSpace;
    create_info.imageExtent = _extent;
    create_info.imageArrayLayers = 1;
    create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    create_info.queueFamilyIndexCount = 2;
    create_info.pQueueFamilyIndices = family_indices;
    create_info.preTransform = _vk_context.get_physical_device()._swapchainSupport.capabilities.currentTransform;
    create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    create_info.presentMode = present_mode;
    create_info.clipped = VK_TRUE;
    create_info.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(_vk_context.get_logical_device().get_vk_handler(), &create_info, nullptr, &_swapchain) != VK_SUCCESS)
        throw std::runtime_error("failed to create swapchain");

    uint32_t imageCount;
    vkGetSwapchainImagesKHR(_vk_context.get_logical_device().get_vk_handler(), _swapchain, &imageCount, nullptr);
    swapchainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(_vk_context.get_logical_device().get_vk_handler(), _swapchain, &imageCount, swapchainImages.data());

    create_image_views();
}

swapchain::~swapchain()
{
    for (size_t i = 0; i < _max_frames_in_flight; ++i)
    {
        vkDestroySemaphore(_vk_context.get_logical_device().get_vk_handler(), _is_render_finished_semaphores[i], nullptr);
        vkDestroySemaphore(_vk_context.get_logical_device().get_vk_handler(), _is_image_available_semaphores[i], nullptr);
        vkDestroyFence(_vk_context.get_logical_device().get_vk_handler(), is_can_submit_work_to_GPU[i], nullptr);
    }

    vkDestroyCommandPool(_vk_context.get_logical_device().get_vk_handler(), _graphic_command_pool, nullptr);
    vkDestroyCommandPool(_vk_context.get_logical_device().get_vk_handler(), _transfer_command_pool, nullptr);
    destroy_image_views();
    vkDestroySwapchainKHR(_vk_context.get_logical_device().get_vk_handler(), _swapchain, nullptr);
    vkDestroySurfaceKHR(_vk_context.get_instance(), _surface, nullptr);
}

void swapchain::draw_frame()
{
    static uint32_t current_frame = 0;
    vkWaitForFences(_vk_context.get_logical_device().get_vk_handler(), 1, &is_can_submit_work_to_GPU[current_frame], VK_TRUE, UINT64_MAX);

    vkResetFences(_vk_context.get_logical_device().get_vk_handler(), 1, &is_can_submit_work_to_GPU[current_frame]);
    uint32_t image_index;

    vkAcquireNextImageKHR(_vk_context.get_logical_device().get_vk_handler(),
                          _swapchain, UINT64_MAX, _is_image_available_semaphores[current_frame],
                          nullptr, &image_index);

    _vk_context.update_ubo(current_frame);
    _vk_context.get_ubos()[current_frame]->dispatch_vertex_data();
    vkResetCommandBuffer(_command_buffers[current_frame], 0);
    record_buffer(_command_buffers[current_frame], image_index);

    VkSemaphore semophores_wait[] = {_is_image_available_semaphores[current_frame]};
    VkPipelineStageFlags stages_for_wait[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    VkSemaphore semophores_signal[] = {_is_render_finished_semaphores[current_frame]};

    VkSubmitInfo sub_info{};
    sub_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    sub_info.waitSemaphoreCount = 1;
    sub_info.pWaitSemaphores = semophores_wait;
    sub_info.pWaitDstStageMask = stages_for_wait;

    sub_info.commandBufferCount = 1;
    sub_info.pCommandBuffers = &_command_buffers[current_frame];

    sub_info.signalSemaphoreCount = 1;
    sub_info.pSignalSemaphores = semophores_signal;

    if (vkQueueSubmit(graphic_queue, 1, &sub_info, is_can_submit_work_to_GPU[current_frame]) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to submit graphic queue");
    }

    VkPresentInfoKHR present_info{};
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = &_is_render_finished_semaphores[current_frame];
    present_info.swapchainCount = 1;
    present_info.pSwapchains = &_swapchain;
    present_info.pImageIndices = &image_index;

    vkQueuePresentKHR(present_queue, &present_info);

    current_frame = (current_frame + 1) % _max_frames_in_flight;
}

void swapchain::create_command_pools()
{
    VkCommandPoolCreateInfo graphic_info{};
    graphic_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    graphic_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    graphic_info.queueFamilyIndex = _vk_context.get_physical_device().queueFamilies.graphicFamily.value();

    VkCommandPoolCreateInfo transfer_info{};
    transfer_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    transfer_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT | VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
    transfer_info.queueFamilyIndex = _vk_context.get_physical_device().queueFamilies.transferFamily.value();

    if (vkCreateCommandPool(_vk_context.get_logical_device().get_vk_handler(), &graphic_info, nullptr, &_graphic_command_pool) != VK_SUCCESS)
        throw std::runtime_error("failed to create graphic command pool");
    if (vkCreateCommandPool(_vk_context.get_logical_device().get_vk_handler(), &transfer_info, nullptr, &_transfer_command_pool) != VK_SUCCESS)
        throw std::runtime_error("failed to create transfer command pool");
}

void swapchain::create_image_views()
{
    swapchainImageViews.resize(swapchainImages.size());
    for (size_t i = 0; i < swapchainImages.size(); ++i)
    {
        VkImageViewCreateInfo create_info{};

        create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        create_info.image = swapchainImages[i];
        create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        create_info.format = _surface_format.format;
        VkComponentMapping components{
            VK_COMPONENT_SWIZZLE_IDENTITY,
            VK_COMPONENT_SWIZZLE_IDENTITY,
            VK_COMPONENT_SWIZZLE_IDENTITY,
            VK_COMPONENT_SWIZZLE_IDENTITY,
        };
        create_info.components = components;
        VkImageSubresourceRange subresourceRange{};
        subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        subresourceRange.baseMipLevel = 0;
        subresourceRange.levelCount = 1;
        subresourceRange.baseArrayLayer = 0;
        subresourceRange.layerCount = 1;
        create_info.subresourceRange = subresourceRange;

        vkCreateImageView(_vk_context.get_logical_device().get_vk_handler(), &create_info, nullptr, &swapchainImageViews[i]);
    }
}
void swapchain::destroy_image_views()
{
    for (auto imageView : swapchainImageViews)
        vkDestroyImageView(_vk_context.get_logical_device().get_vk_handler(), imageView, nullptr);
}

void swapchain::create_command_buffers()
{
    _command_buffers.resize(_max_frames_in_flight);

    VkCommandBufferAllocateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    info.commandPool = _graphic_command_pool;
    info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    info.commandBufferCount = _command_buffers.size();

    for (auto &command_buffer : _command_buffers)
    {

        if (vkAllocateCommandBuffers(_vk_context.get_logical_device().get_vk_handler(), &info, &command_buffer) != VK_SUCCESS)
            throw std::runtime_error("faildet to allocate command buffer");
    }
}

void swapchain::record_buffer(VkCommandBuffer command_buffer, uint32_t image_index)
{
    VkCommandBufferBeginInfo begin_info{};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    if (vkBeginCommandBuffer(command_buffer, &begin_info) != VK_SUCCESS)
        throw std::runtime_error("failed to begin command buffer");

    VkRenderPassBeginInfo renderpass_befin_info{};
    renderpass_befin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderpass_befin_info.renderPass = _vk_context.get_renderpass().get_vk_handle();
    renderpass_befin_info.framebuffer = _vk_context.get_renderpass().get_framebuffers()[image_index];
    renderpass_befin_info.renderArea.offset = {0, 0};
    renderpass_befin_info.renderArea.extent = _extent;
    renderpass_befin_info.clearValueCount = 1;
    VkClearValue clear_value = {0.8, 0.8, 0.8, 1.0};
    renderpass_befin_info.pClearValues = &clear_value;
    vkCmdBeginRenderPass(command_buffer, &renderpass_befin_info, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _vk_context.get_pipeline().get_vk_handle());

    VkBuffer *vertex_buffers = &_vk_context.get_vertex_buffer().get_vk_handler();
    VkDeviceSize offsets[] = {0};

    vkCmdBindIndexBuffer(command_buffer, _vk_context.get_index_buffer().get_vk_handler(), 0, VK_INDEX_TYPE_UINT32);
    vkCmdBindVertexBuffers(command_buffer, 0, 1, vertex_buffers, offsets);
    vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _vk_context.get_pipeline().get_pipeline_layout(),
                            0, 1, &_vk_context.get_descriptor_sets()[image_index], 0, nullptr);

    vkCmdDrawIndexed(command_buffer, _vk_context.get_index_buffer().get_buffer_size() / sizeof(uint32_t), 1, 0, 0, 0);
    vkCmdEndRenderPass(command_buffer);
    if (vkEndCommandBuffer(command_buffer) != VK_SUCCESS)
        throw std::runtime_error("failed to end frame buffer");
}

std::vector<VkImageView> swapchain::get_swapchain_imageveiws()
{
    return swapchainImageViews;
}

void swapchain::create_sync_objects()
{
    _is_image_available_semaphores.resize(_max_frames_in_flight);
    _is_render_finished_semaphores.resize(_max_frames_in_flight);
    is_can_submit_work_to_GPU.resize(_max_frames_in_flight);

    VkSemaphoreCreateInfo semaphore_info{};
    semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fence_info{};
    fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < _max_frames_in_flight; ++i)
    {

        if (vkCreateSemaphore(_vk_context.get_logical_device().get_vk_handler(), &semaphore_info, nullptr, &_is_image_available_semaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(_vk_context.get_logical_device().get_vk_handler(), &semaphore_info, nullptr, &_is_render_finished_semaphores[i]) != VK_SUCCESS ||
            vkCreateFence(_vk_context.get_logical_device().get_vk_handler(), &fence_info, nullptr, &is_can_submit_work_to_GPU[i]) != VK_SUCCESS)
            throw std::runtime_error("failed to create sync objects");
    }
}
VkQueue *swapchain::get_grapchic_queue()
{
    return &graphic_queue;
}
VkQueue *swapchain::get_present_queue()
{
    return &present_queue;
}
VkQueue *swapchain::get_transfer_queue()
{
    return &transfer_queue;
}

VkCommandPool &swapchain::get_command_pool(pool_type type)
{
    switch (type)
    {
    case GRAPHIC:
        return _graphic_command_pool;
        break;

    case TRANSFER:
        return _transfer_command_pool;
        break;
    default:
        throw std::runtime_error("failed to get command pool");
    }
}


size_t swapchain::get_max_frames_in_flight(){
    return _max_frames_in_flight;
}