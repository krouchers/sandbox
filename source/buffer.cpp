#include <buffer.h>
#include <vulkan_context.h>

// std
#include <stdexcept>
#include <iostream>

VkVertexInputBindingDescription buffer::get_binding_description()
{
    VkVertexInputBindingDescription description{};
    description.stride = sizeof(Vertex);
    description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    description.binding = 0;

    return description;
}

std::array<VkVertexInputAttributeDescription, 2> buffer::get_atribute_description()
{
    std::array<VkVertexInputAttributeDescription, 2> attribute_descriptions{};
    attribute_descriptions[0].binding = 0;
    attribute_descriptions[0].location = 0;
    attribute_descriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
    attribute_descriptions[0].offset = offsetof(Vertex, position);
    attribute_descriptions[1].binding = 0;
    attribute_descriptions[1].location = 1;
    attribute_descriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attribute_descriptions[1].offset = offsetof(Vertex, color);
    return attribute_descriptions;
}

void buffer::create_buffer()
{
    VkBufferCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    info.size = _vertices.value().size();
    info.usage = _usage;
    info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(_vk_context.get_logical_device().get_vk_handler(), &info, nullptr, &_buffer) != VK_SUCCESS)
        throw std::runtime_error("failed to create vertex input buffer");
    vkGetBufferMemoryRequirements(_vk_context.get_logical_device().get_vk_handler(), _buffer, &memory_requirements);

    VkMemoryAllocateInfo allocate_info{};
    allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocate_info.allocationSize = memory_requirements.size;
    allocate_info.memoryTypeIndex = find_memory_type(memory_requirements.memoryTypeBits, _memory_properties);

    if (vkAllocateMemory(_vk_context.get_logical_device().get_vk_handler(), &allocate_info, nullptr, &_memory) != VK_SUCCESS)
        throw std::runtime_error("failed to allocate vertex memory");

    vkBindBufferMemory(_vk_context.get_logical_device().get_vk_handler(), _buffer, _memory, 0);
}

void buffer::dispatch_vertex_data()
{
    void *host_visible_memory{nullptr};
    vkMapMemory(_vk_context.get_logical_device().get_vk_handler(), _memory, 0, VK_WHOLE_SIZE, 0, &host_visible_memory);
#ifdef DEBUG
    // for (const auto &vertex : _vertices.value())
    // {
    //     if (vertex.color[0] != 0)
    //         std::cout << vertex.position.x << " " << vertex.position.y << " with color " << vertex.color.r << " " << vertex.color.g << " " << vertex.color.b << "\n";
    //     else
    //         break;
    // }
#endif
    memcpy(host_visible_memory, _vertices.value().data(), sizeof(Vertex) * quanity_loaded_vertices);
    vkUnmapMemory(_vk_context.get_logical_device().get_vk_handler(), _memory);
}
uint32_t buffer::find_memory_type(uint32_t memory_type_filter, VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties physical_device_memory_properties;
    vkGetPhysicalDeviceMemoryProperties(_vk_context.get_physical_device().getVkHandler(), &physical_device_memory_properties);

    for (uint32_t i = 0; i < physical_device_memory_properties.memoryTypeCount; ++i)
    {
        if (memory_type_filter & (1 << i) && (physical_device_memory_properties.memoryTypes[i].propertyFlags & properties) == properties)
        {
            return i;
        }
    }

    throw std::runtime_error("failed to find suitable memory type");
}

buffer::buffer(vulkan_context &vk_context, size_t size, VkBufferUsageFlags usage, VkMemoryPropertyFlags memory_properties)
    : _vk_context{vk_context}, _vertices(std::vector<Vertex>(size)), quanity_loaded_vertices{0},
      _usage{usage}, _memory_properties{memory_properties}
{
    create_buffer();
}

buffer::~buffer()
{
    vkDestroyBuffer(_vk_context.get_logical_device().get_vk_handler(), _buffer, nullptr);
    vkFreeMemory(_vk_context.get_logical_device().get_vk_handler(), _memory, nullptr);
}

VkBuffer &buffer::get_vk_handler()
{
    return _buffer;
}

void buffer::add_vertex_data(std::vector<Vertex> &&data)
{
    size_t quantity_adding_vertexs = data.size();
    _vertices.value().insert(_vertices.value().begin() + quanity_loaded_vertices, data.begin(), data.end());
    quanity_loaded_vertices += quantity_adding_vertexs;
    dispatch_vertex_data();
}
size_t buffer::get_buffer_size()
{
    return _vertices.value().size() * sizeof(_vertices.value()[0]);
}

size_t buffer::get_quantity_loaded_vertices()
{
    return quanity_loaded_vertices;
}

void buffer::copy_buffer(buffer &src)
{
    quanity_loaded_vertices = src.quanity_loaded_vertices;
    VkCommandBufferAllocateInfo buf_aloc_info{};
    buf_aloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    buf_aloc_info.commandPool = _vk_context.get_swapchain().get_command_pool(TRANSFER);
    buf_aloc_info.commandBufferCount = 1;
    buf_aloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

    VkCommandBuffer transfer_coomand_buffer;

    vkAllocateCommandBuffers(_vk_context.get_logical_device().get_vk_handler(), &buf_aloc_info, &transfer_coomand_buffer);

    VkCommandBufferBeginInfo begin_info{};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    VkBufferCopy copy_region{};
    copy_region.srcOffset = 0;
    copy_region.dstOffset = 0;
    copy_region.size = src.quanity_loaded_vertices * sizeof(Vertex);

    vkBeginCommandBuffer(transfer_coomand_buffer, &begin_info);

    vkCmdCopyBuffer(transfer_coomand_buffer, src.get_vk_handler(), _buffer, 1, &copy_region);

    vkEndCommandBuffer(transfer_coomand_buffer);

    VkSubmitInfo sub_info{};
    sub_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    sub_info.commandBufferCount = 1;
    sub_info.pCommandBuffers = &transfer_coomand_buffer;

    vkQueueSubmit(*_vk_context.get_swapchain().get_transfer_queue(), 1, &sub_info, VK_NULL_HANDLE);
    vkQueueWaitIdle(*_vk_context.get_swapchain().get_transfer_queue());
    vkFreeCommandBuffers(_vk_context.get_logical_device().get_vk_handler(), _vk_context.get_swapchain().get_command_pool(TRANSFER),
                         1, &transfer_coomand_buffer);
}

VkDeviceMemory &buffer::get_vk_device_memory_handle()
{
    return _memory;
}

void *buffer::data()
{
    return reinterpret_cast<void *>(const_cast<Vertex *>(_vertices.value().data()));
}