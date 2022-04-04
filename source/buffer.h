#pragma once

// 3dparty
#include <vulkan/vulkan.h>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
// std
#include <vector>
#include <array>
#include <optional>
//

class vulkan_context;

struct Vertex
{
    glm::vec2 position;
    glm::vec3 color;
};

class buffer
{
    vulkan_context &_vk_context;
    VkVertexInputBindingDescription _binding_description;
    std::optional<std::vector<Vertex>> _vertices = std::nullopt;

    VkBuffer _buffer;
    VkDeviceMemory _memory;
    VkMemoryRequirements memory_requirements;

    size_t quanity_loaded_vertices;

    VkBufferUsageFlags _usage;
    VkMemoryPropertyFlags _memory_properties;

public:
    buffer(vulkan_context &, size_t size, VkBufferUsageFlags usage, VkMemoryPropertyFlags memory_properties);
    ~buffer();
    void add_vertex_data(std::vector<Vertex> &&data);
    // geters
    size_t get_buffer_size();
    VkBuffer &get_vk_handler();
    size_t get_quantity_loaded_vertices();
    VkDeviceMemory &get_vk_device_memory_handle();
    //
    void copy_buffer(buffer &src);
    VkVertexInputBindingDescription get_binding_description();
    void dispatch_vertex_data();
    void *data();
    std::array<VkVertexInputAttributeDescription, 2> get_atribute_description();
    uint32_t find_memory_type(uint32_t memory_type_filter, VkMemoryPropertyFlags properties);
    void create_buffer();
};