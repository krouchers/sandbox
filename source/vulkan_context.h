#pragma once
#include <window.h>
#include <debug.h>
#include <physicalDevice.h>
#include <logical_device.h>
#include <renderpass.h>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

// std
#include <vector>
#include <string>
#include <memory>
//

class swapchain;
class graphic_pipeline;
class application;
template <typename T>
class buffer;

struct uniform_buffer_object
{
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
};

struct Vertex
{
    glm::vec2 position;
    glm::vec3 color;
};

struct extAndLayerInfo
{
    uint32_t layersCount;
    const char **ppLayers;
    uint32_t extCount;
    const char **ppExt;
};

class vulkan_context
{
private:
    VkInstance instance;
    std::vector<const char *> required_extantions;
    std::vector<const char *> required_layers{"VK_LAYER_KHRONOS_validation"};
    const bool is_debug_enabled;
    bool checkRequestedLayersSupport();
    std::unique_ptr<debug_messenger> debugMessenger;
    std::unique_ptr<physicalDevice> _physical_device;
    std::unique_ptr<logical_device> _p_logical_device;
    std::unique_ptr<graphic_pipeline> _graphic_pipeline;
    std::unique_ptr<swapchain> _swapchain;
    std::unique_ptr<renderpass> _renderpass;
    std::unique_ptr<buffer<Vertex>> _vertex_buffer;
    std::unique_ptr<buffer<uint32_t>> _index_buffer;

    std::vector<std::unique_ptr<buffer<uniform_buffer_object>>> _ubos;
    std::vector<VkDescriptorSet> _descriptor_sets;
    VkDescriptorPool _descriptor_pool;
    Window &window;

    void initPhysicalDevice() noexcept;
    void graphic_pipeline_init();
    void create_graphic_pipeline();

public:
    vulkan_context(Window &wnd, bool is_debug_enabled = false);
    ~vulkan_context();

    vulkan_context(const vulkan_context &) = delete;
    vulkan_context(const vulkan_context &&) = delete;
    vulkan_context &operator=(const vulkan_context &) = delete;
    vulkan_context &operator=(vulkan_context &&);
    std::vector<std::unique_ptr<buffer<uniform_buffer_object>>> &get_ubos();
    // geters
    std::vector<VkDescriptorSet> &get_descriptor_sets();
    VkInstance get_instance();
    buffer<Vertex> &get_vertex_buffer();
    Window &getWindow();
    physicalDevice &get_physical_device();
    swapchain &get_swapchain();
    logical_device &get_logical_device();
    renderpass &get_renderpass();
    graphic_pipeline &get_pipeline();
    buffer<uint32_t> &get_index_buffer();
    void destroy_ubos();
    void destroy_descriptor_sets();
    //
    // seters
    template <typename T>
    void set_buffer_data(buffer<T> &buf, std::vector<T> data);
    //
    void device_idle();

    void ubos_init();
    void update_ubo(uint32_t);
    void destroy_staged_vertex_buffer();
    void destroy_final_vertex_buffer();
    void create_vertex_buffer(std::vector<Vertex> &data);
    buffer<Vertex> create_staged_vertex_buffer(std::vector<Vertex> &data);
    buffer<uint32_t> create_staged_index_buffer(std::vector<uint32_t> &data);
    void create_final_vertex_buffer(size_t);
    void create_sync_objects();
    void create_descriptor_pool();
    void populate_descriptors();
    void allocate_descriptor_sets();
    void create_command_buffers();
    void create_surface();
    void renderpass_init();
    void create_renderpass();
    void create_swapchain();
    void write_descriptor_sets();
    void destroy_device();
    void create_index_buffer(std::vector<uint32_t> &data);
    extAndLayerInfo getExtAndLayersInfo() noexcept;
    void print_supported_extantions();
    void create_instance();
    void draw_frame();
    const char **enumerateExtations(uint32_t *);
    void print_required_extantions();
    void destroy_instance();
    void createLogicalDevice();
    void destroy_surface();
    void transfer_to_local_memory_vertex_data(std::vector<Vertex> &&data);
    void transfer_to_local_memory_index_buffer(std::vector<uint32_t> &data);
};
