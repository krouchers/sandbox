#include <texture.h>

void texture::load_image(const std::string path)
{
    stbi_uc *pixels = stbi_load(path.c_str(), &_texture_width, &_texture_height, &_texture_channels, STBI_rgb_alpha);
    std::vector<stbi_uc> image_data(pixels, pixels + _texture_height * _texture_height * 4);
    _staging_buffer = std::make_unique<buffer<stbi_uc>>(
        _vk_context, image_data,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    _staging_buffer->dispatch_vertex_data();
    stbi_image_free(pixels);
}

void texture::create_image()
{
    VkImageCreateInfo image_info{};
    image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_info.imageType = VK_IMAGE_TYPE_2D;
    image_info.format = VK_FORMAT_R8G8B8A8_SRGB;
    image_info.extent.depth = 1;
    image_info.extent.width = _texture_width;
    image_info.extent.height = _texture_height;
    image_info.mipLevels = 1;
    image_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_info.arrayLayers = 1;
    image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_info.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    if (vkCreateImage(_vk_context->get_logical_device().get_vk_handler(),
                      &image_info, nullptr, &_texture_image) != VK_SUCCESS)
        throw std::runtime_error("faile to create texture image");
}

void texture::allocate_image_memory()
{
    VkMemoryRequirements requirements;
    vkGetImageMemoryRequirements(_vk_context->get_logical_device().get_vk_handler(),
                                 _texture_image, &requirements);
    VkMemoryAllocateInfo alloc_info{};
    alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.allocationSize = requirements.size;
    alloc_info.memoryTypeIndex = _staging_buffer->find_memory_type(requirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    if (vkAllocateMemory(_vk_context->get_logical_device().get_vk_handler(), &alloc_info, nullptr, &_texture_image_memory) != VK_SUCCESS)
        throw std::runtime_error("failed to allocate texture memory");

    vkBindImageMemory(_vk_context->get_logical_device().get_vk_handler(),
                      _texture_image, _texture_image_memory, 0);
}

texture::texture(vulkan_context *vk_cont, sampler *samp, const std::string tex_path)
    : _vk_context{vk_cont}, _sampler{samp}
{
    load_image(tex_path);
    create_image();
    allocate_image_memory();
    create_image_view();
}

texture::~texture()
{
    vkDestroyImageView(_vk_context->get_logical_device().get_vk_handler(), _texture_image_view, nullptr);
    vkDestroyImage(_vk_context->get_logical_device().get_vk_handler(), _texture_image, nullptr);
    vkFreeMemory(_vk_context->get_logical_device().get_vk_handler(), _texture_image_memory, nullptr);
}

VkCommandBuffer texture::begin_single_time_commands()
{
    VkCommandBufferAllocateInfo alloc_info{};
    alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    alloc_info.commandPool = _vk_context->get_swapchain().get_command_pool(GRAPHIC);
    alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    alloc_info.commandBufferCount = 1;

    VkCommandBuffer command_buffer;

    vkAllocateCommandBuffers(_vk_context->get_logical_device().get_vk_handler(), &alloc_info, &command_buffer);

    VkCommandBufferBeginInfo begin_info{};

    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(command_buffer, &begin_info);
    return command_buffer;
}

void texture::end_single_time_commands(VkCommandBuffer command_buffer)
{
    vkEndCommandBuffer(command_buffer);

    VkSubmitInfo sub_info{};
    sub_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    sub_info.commandBufferCount = 1;
    sub_info.pCommandBuffers = &command_buffer;

    vkQueueSubmit(
        *_vk_context->get_swapchain().get_grapchic_queue(),
        1, &sub_info, VK_NULL_HANDLE);
    vkQueueWaitIdle(*_vk_context->get_swapchain().get_grapchic_queue());
    vkFreeCommandBuffers(_vk_context->get_logical_device().get_vk_handler(),
                         _vk_context->get_swapchain().get_command_pool(GRAPHIC), 1, &command_buffer);
}

void texture::set_image_layout(VkImageLayout old_layout, VkImageLayout new_layout)
{
    VkCommandBuffer command_buffer = begin_single_time_commands();
    VkImageMemoryBarrier barier{};
    VkPipelineStageFlags src_stage{};
    VkPipelineStageFlags dst_stage{};
    if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
    {
        barier.srcAccessMask = 0;
        barier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        src_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        dst_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if (old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL and new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        barier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        src_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        dst_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    barier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barier.oldLayout = old_layout;
    barier.newLayout = new_layout;
    barier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barier.image = _texture_image;
    barier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barier.subresourceRange.baseMipLevel = 0;
    barier.subresourceRange.levelCount = 1;
    barier.subresourceRange.baseArrayLayer = 0;
    barier.subresourceRange.layerCount = 1;

    vkCmdPipelineBarrier(command_buffer,
                         src_stage, dst_stage,
                         0,
                         0, nullptr,
                         0, nullptr,
                         1, &barier);
    end_single_time_commands(command_buffer);
}
void texture::copy_buffer_to_image()
{
    VkCommandBuffer command_buffer = begin_single_time_commands();

    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = {0, 0, 0};
    region.imageExtent = {static_cast<uint32_t>(_texture_width), static_cast<uint32_t>(_texture_height), 1};

    vkCmdCopyBufferToImage(command_buffer, _staging_buffer->get_vk_handler(), _texture_image,
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
    end_single_time_commands(command_buffer);
}

void texture::create_image_view()
{
    VkImageViewCreateInfo view_info{};
    view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    view_info.image = _texture_image;
    view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    view_info.format = VK_FORMAT_R8G8B8A8_SRGB;
    view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    view_info.subresourceRange.baseMipLevel = 0;
    view_info.subresourceRange.levelCount = 1;
    view_info.subresourceRange.baseArrayLayer = 0;
    view_info.subresourceRange.layerCount = 1;

    if (vkCreateImageView(_vk_context->get_logical_device().get_vk_handler(),
                          &view_info, nullptr, &_texture_image_view) != VK_SUCCESS)
        throw std::runtime_error("failed to create texture image view");
}

VkImageView &texture::get_image_view()
{
    return _texture_image_view;
}

void texture::set_data()
{
    set_image_layout(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    copy_buffer_to_image();
    set_image_layout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    _staging_buffer = nullptr;
}