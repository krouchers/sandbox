#include <buffer.h>
#include <swapchain.h>

template <typename T>
VkVertexInputBindingDescription buffer<T>::get_binding_description()
{
    VkVertexInputBindingDescription description{};
    description.stride = sizeof(T);
    description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    description.binding = 0;

    return description;
}

template <typename T>
std::array<VkVertexInputAttributeDescription, 3> buffer<T>::get_atribute_description()
{
    std::array<VkVertexInputAttributeDescription, 3> attribute_descriptions{};
    attribute_descriptions[0].binding = 0;
    attribute_descriptions[0].location = 0;
    attribute_descriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
    attribute_descriptions[0].offset = offsetof(T, position);

    attribute_descriptions[1].binding = 0;
    attribute_descriptions[1].location = 1;
    attribute_descriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attribute_descriptions[1].offset = offsetof(T, color);

    attribute_descriptions[2].binding = 0;
    attribute_descriptions[2].location = 2;
    attribute_descriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
    attribute_descriptions[2].offset = offsetof(T, texture_coord);
    return attribute_descriptions;
}

template <typename T>
void buffer<T>::create_buffer()
{
    VkBufferCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    info.size = _size;
    info.usage = _usage;
    info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(_vk_context->get_logical_device().get_vk_handler(), &info, nullptr, &_buffer) != VK_SUCCESS)
        throw std::runtime_error("failed to create vertex input buffer");
    vkGetBufferMemoryRequirements(_vk_context->get_logical_device().get_vk_handler(), _buffer, &memory_requirements);

    VkMemoryAllocateInfo allocate_info{};
    allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocate_info.allocationSize = memory_requirements.size;
    allocate_info.memoryTypeIndex = find_memory_type(memory_requirements.memoryTypeBits, _memory_properties);

    if (vkAllocateMemory(_vk_context->get_logical_device().get_vk_handler(), &allocate_info, nullptr, &_memory) != VK_SUCCESS)
        throw std::runtime_error("failed to allocate vertex memory");

    vkBindBufferMemory(_vk_context->get_logical_device().get_vk_handler(), _buffer, _memory, 0);
}

template <typename T>
void buffer<T>::dispatch_vertex_data()
{
    void *host_visible_memory{nullptr};
    vkMapMemory(_vk_context->get_logical_device().get_vk_handler(), _memory, 0, VK_WHOLE_SIZE, 0, &host_visible_memory);
    memcpy(host_visible_memory, _vertices.data(), sizeof(T) * _vertices.size());
    vkUnmapMemory(_vk_context->get_logical_device().get_vk_handler(), _memory);
}

template <typename T>
uint32_t buffer<T>::find_memory_type(uint32_t memory_type_filter, VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties physical_device_memory_properties;
    vkGetPhysicalDeviceMemoryProperties(_vk_context->get_physical_device().getVkHandler(), &physical_device_memory_properties);

    for (uint32_t i = 0; i < physical_device_memory_properties.memoryTypeCount; ++i)
    {
        if (memory_type_filter & (1 << i) && (physical_device_memory_properties.memoryTypes[i].propertyFlags & properties) == properties)
        {
            return i;
        }
    }

    throw std::runtime_error("failed to find suitable memory type");
}

template <typename T>
buffer<T>::buffer(vulkan_context *vk_context, std::vector<T> &data, VkBufferUsageFlags usage, VkMemoryPropertyFlags memory_properties)
    : _vk_context{vk_context}, _size{data.size() * sizeof(T)}, _vertices(data),
      _usage{usage}, _memory_properties{memory_properties}
{
    create_buffer();
}

template <typename T>
buffer<T>::~buffer()
{
    vkDestroyBuffer(_vk_context->get_logical_device().get_vk_handler(), _buffer, nullptr);
    vkFreeMemory(_vk_context->get_logical_device().get_vk_handler(), _memory, nullptr);
}

template <typename T>
VkBuffer &buffer<T>::get_vk_handler()
{
    return _buffer;
}

template <typename T>
size_t buffer<T>::get_buffer_size()
{
    return _size;
}

template <typename T>
void buffer<T>::copy_buffer(buffer &src)
{
    VkCommandBufferAllocateInfo buf_aloc_info{};
    buf_aloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    buf_aloc_info.commandPool = _vk_context->get_swapchain().get_command_pool(TRANSFER);
    buf_aloc_info.commandBufferCount = 1;
    buf_aloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

    VkCommandBuffer transfer_coomand_buffer;

    vkAllocateCommandBuffers(_vk_context->get_logical_device().get_vk_handler(), &buf_aloc_info, &transfer_coomand_buffer);

    VkCommandBufferBeginInfo begin_info{};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    VkBufferCopy copy_region{};
    copy_region.srcOffset = 0;
    copy_region.dstOffset = 0;
    copy_region.size = src.get_buffer_size();

    vkBeginCommandBuffer(transfer_coomand_buffer, &begin_info);

    vkCmdCopyBuffer(transfer_coomand_buffer, src.get_vk_handler(), _buffer, 1, &copy_region);

    vkEndCommandBuffer(transfer_coomand_buffer);

    VkSubmitInfo sub_info{};
    sub_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    sub_info.commandBufferCount = 1;
    sub_info.pCommandBuffers = &transfer_coomand_buffer;

    vkQueueSubmit(*_vk_context->get_swapchain().get_transfer_queue(), 1, &sub_info, VK_NULL_HANDLE);
    vkQueueWaitIdle(*_vk_context->get_swapchain().get_transfer_queue());
    vkFreeCommandBuffers(_vk_context->get_logical_device().get_vk_handler(), _vk_context->get_swapchain().get_command_pool(TRANSFER),
                         1, &transfer_coomand_buffer);
}

template <typename T>
VkDeviceMemory &buffer<T>::get_vk_device_memory_handle()
{
    return _memory;
}

template <typename T>
std::vector<T> &buffer<T>::data()
{
    return _vertices;
}

template <typename T>
void buffer<T>::set_data(std::vector<T> &data)
{
    _vertices = data;
    _size = sizeof(T) * _vertices.size();
}
template <typename T>
buffer<T>::buffer()
{
}
template <typename T>
buffer<T>::buffer(vulkan_context *vk_context, size_t size, VkBufferUsageFlags usage, VkMemoryPropertyFlags memory_properties)
    : _vk_context{vk_context}, _size{size * sizeof(T)},
      _usage{usage}, _memory_properties{memory_properties}
{
    create_buffer();
}