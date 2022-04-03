#include <vertex_buffer.h>
#include <vulkan_context.h>

// std
#include <stdexcept>
#include <iostream>

// void vertex_buffer::init_buffer()
// {
//     const std::vector<Vertex> vertices = {
//         {{0.0, -0.5}, {1.0, 0.0, 0.0}},
//         {{0.5, 0.5}, {0.0, 1.0, 0.0}},
//         {{-0.5, 0.5}, {0.0, 0.0, 1.0}}};
// }

VkVertexInputBindingDescription vertex_buffer::get_binding_description()
{
    VkVertexInputBindingDescription description{};
    description.stride = sizeof(Vertex);
    description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    description.binding = 0;

    return description;
}

std::array<VkVertexInputAttributeDescription, 2> vertex_buffer::get_atribute_description()
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

void vertex_buffer::create_buffer()
{
    VkBufferCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    info.size = sizeof(Vertex) * _vertices.value().size();
    info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(_vk_context.get_logical_device().get_vk_handler(), &info, nullptr, &_vertex_buffer) != VK_SUCCESS)
        throw std::runtime_error("failed to create vertex input buffer");
    vkGetBufferMemoryRequirements(_vk_context.get_logical_device().get_vk_handler(), _vertex_buffer, &memory_requirements);

    VkMemoryAllocateInfo allocate_info{};
    allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocate_info.allocationSize = memory_requirements.size;
    allocate_info.memoryTypeIndex = find_memory_type(memory_requirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT |
                                                                                             VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    if (vkAllocateMemory(_vk_context.get_logical_device().get_vk_handler(), &allocate_info, nullptr, &_vertex_memory) != VK_SUCCESS)
        throw std::runtime_error("failed to allocate vertex memory");

    vkBindBufferMemory(_vk_context.get_logical_device().get_vk_handler(), _vertex_buffer, _vertex_memory, 0);
}

void vertex_buffer::dispatch_vertex_data()
{
    void *host_visible_memory{nullptr};
    vkMapMemory(_vk_context.get_logical_device().get_vk_handler(), _vertex_memory, 0, VK_WHOLE_SIZE, 0, &host_visible_memory);
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
    vkUnmapMemory(_vk_context.get_logical_device().get_vk_handler(), _vertex_memory);
}
uint32_t vertex_buffer::find_memory_type(uint32_t memory_type_filter, VkMemoryPropertyFlags properties)
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

vertex_buffer::vertex_buffer(vulkan_context &vk_context, size_t size) : _vk_context{vk_context}, _vertices(std::vector<Vertex>(size)), quanity_loaded_vertices{0}
{
    create_buffer();
}

vertex_buffer::~vertex_buffer()
{
    vkDestroyBuffer(_vk_context.get_logical_device().get_vk_handler(), _vertex_buffer, nullptr);
    vkFreeMemory(_vk_context.get_logical_device().get_vk_handler(), _vertex_memory, nullptr);
}

VkBuffer *vertex_buffer::get_vertex_buffers()
{
    return &_vertex_buffer;
}

void vertex_buffer::add_vertex_data(std::vector<Vertex> &&data)
{
    size_t quantity_adding_vertexs = data.size();
    _vertices.value().insert(_vertices.value().begin() + quanity_loaded_vertices, data.begin(), data.end());
    quanity_loaded_vertices += quantity_adding_vertexs;
    dispatch_vertex_data();
}
size_t vertex_buffer::get_vertex_buffer_size()
{
    return _vertices.value().size();
}

size_t vertex_buffer::get_quantity_loaded_vertices()
{
    return quanity_loaded_vertices;
}