#include <renderpass.h>
#include <vulkan/vulkan.h>
#include <vulkan_context.h>
#include<swapchain.h>

// std
#include <stdexcept>

VkRenderPass &renderpass::get_vk_handle()
{
    return _renderpass;
}

renderpass::renderpass(vulkan_context &vk_cont) : _vk_context{vk_cont}
{
}
void renderpass::create_renderpass()
{
    VkAttachmentDescription color_attachment{};
    color_attachment.format = _vk_context.get_swapchain().get_surface_format().format;
    color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference color_attachment_ref{};
    color_attachment_ref.attachment = 0;
    color_attachment_ref.layout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpuss_info{};
    subpuss_info.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpuss_info.colorAttachmentCount = 1;
    subpuss_info.pColorAttachments = &color_attachment_ref;

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_MEMORY_WRITE_BIT;

    VkRenderPassCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    info.attachmentCount = 1;
    info.pAttachments = &color_attachment;
    info.subpassCount = 1;
    info.pSubpasses = &subpuss_info;
    info.dependencyCount = 1;
    info.pDependencies = &dependency;
    if (vkCreateRenderPass(_vk_context.get_logical_device().get_vk_handler(), &info, nullptr, &_renderpass) != VK_SUCCESS)
        throw std::runtime_error("failed to create renderpass");
}

renderpass::~renderpass()
{
    for (auto &framebuffer : swapchain_framebuffers)
    {
        vkDestroyFramebuffer(_vk_context.get_logical_device().get_vk_handler(), framebuffer, nullptr);
    }
    vkDestroyRenderPass(_vk_context.get_logical_device().get_vk_handler(), _renderpass, nullptr);
}
std::vector<VkFramebuffer> renderpass::get_framebuffers()
{
    return swapchain_framebuffers;
}

void renderpass::create_framebuffers()
{
    swapchain_framebuffers.resize(_vk_context.get_swapchain().get_swapchain_imageveiws().size());

    for (size_t i = 0; i < _vk_context.get_swapchain().get_swapchain_imageveiws().size(); ++i)
    {
        VkImageView attachments[] = {_vk_context.get_swapchain().get_swapchain_imageveiws()[i]};
        VkFramebufferCreateInfo framebuffer_info{};
        framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebuffer_info.renderPass = _vk_context.get_renderpass().get_vk_handle();
        framebuffer_info.attachmentCount = 1;
        framebuffer_info.pAttachments = attachments;
        framebuffer_info.width = _vk_context.get_swapchain().get_extent().width;
        framebuffer_info.height = _vk_context.get_swapchain().get_extent().height;
        framebuffer_info.layers = 1;

        if (vkCreateFramebuffer(_vk_context.get_logical_device().get_vk_handler(), &framebuffer_info, nullptr, &swapchain_framebuffers[i]) != VK_SUCCESS)
            throw std::runtime_error("failed to create framebuffer");
    }
}