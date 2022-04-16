#pragma once
#include <vulkan_context.h>
// 3dparty
#include <vulkan/vulkan.h>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
// std
#include <vector>
#include <array>
#include <optional>
#include <stdexcept>
#include <iostream>
//

template <typename T>
class buffer
{
    vulkan_context *_vk_context;
    VkVertexInputBindingDescription _binding_description;
    size_t _size;
    std::vector<T> _vertices;

    VkBuffer _buffer;
    VkDeviceMemory _memory;
    VkMemoryRequirements memory_requirements;

    VkBufferUsageFlags _usage;
    VkMemoryPropertyFlags _memory_properties;

public:
    buffer(vulkan_context *, std::vector<T> &data, VkBufferUsageFlags usage, VkMemoryPropertyFlags memory_properties);
    buffer(vulkan_context *, size_t size, VkBufferUsageFlags usage, VkMemoryPropertyFlags memory_properties);
    buffer();
    ~buffer();
    // geters
    size_t get_buffer_size();
    VkBuffer &get_vk_handler();
    VkDeviceMemory &get_vk_device_memory_handle();
    //
    void copy_buffer(buffer &src);
    void set_data(std::vector<T> &data);
    VkVertexInputBindingDescription get_binding_description();
    void dispatch_vertex_data();
    std::vector<T> &data();
    std::array<VkVertexInputAttributeDescription, 3> get_atribute_description();
    void create_buffer();
};

#include <buffer_impl.h>