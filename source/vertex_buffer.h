#pragma once

// 3dparty
#include <vulkan/vulkan.h>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
// std
#include <vector>
#include <array>
#include<optional>
//

class vulkan_context;

struct Vertex
{
    glm::vec2 position;
    glm::vec3 color;
};

class vertex_buffer
{
    vulkan_context &_vk_context;
    VkVertexInputBindingDescription _binding_description;
    std::optional<std::vector<Vertex>> _vertices = std::nullopt;

    VkBuffer _vertex_buffer;
    VkDeviceMemory _vertex_memory;
    VkMemoryRequirements memory_requirements;

    size_t quanity_loaded_vertices;

public:
    vertex_buffer(vulkan_context &, size_t size);
    ~vertex_buffer();
    void add_vertex_data(std::vector<Vertex> &&data);
    //geters
    size_t get_vertex_buffer_size();
    size_t get_quantity_loaded_vertices();
    //
    VkBuffer *get_vertex_buffers();
    VkVertexInputBindingDescription get_binding_description();
    void dispatch_vertex_data();
    std::array<VkVertexInputAttributeDescription, 2> get_atribute_description();
    uint32_t find_memory_type(uint32_t memory_type_filter, VkMemoryPropertyFlags properties);
    void create_buffer();
};