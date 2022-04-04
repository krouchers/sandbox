#pragma once
#include <window.h>
#include <debug.h>
#include <physicalDevice.h>
#include <logical_device.h>
#include <swapchain.h>
#include <renderpass.h>
#include <buffer.h>

// std
#include <vector>
#include <string>
#include <memory>
//

class swapchain;
class graphic_pipeline;
class application;
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
    std::unique_ptr<buffer> _staged_vertex_buffer;
    std::unique_ptr<buffer> _final_vertex_buffer;
    std::unique_ptr<buffer> _index_buffer;
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

    // geters
    VkInstance get_instance();
    buffer &get_staged_vertex_buffer();
    buffer &get_final_vertex_buffer();
    Window &getWindow();
    physicalDevice &get_physical_device();
    swapchain &get_swapchain();
    logical_device &get_logical_device();
    renderpass &get_renderpass();
    graphic_pipeline &get_pipeline();
    //
    // seters
    void add_vertex_data(std::vector<Vertex> &&data);
    void set_vertex_data_to_buffer(buffer &buf, std::vector<Vertex> data);
    //
    void device_idle();
    void destroy_staged_vertex_buffer();
    void destroy_final_vertex_buffer();
    void create_staged_vertex_buffer(size_t);
    void create_final_vertex_buffer(size_t);
    void create_sync_objects();
    void create_command_buffers();
    void create_surface();
    void renderpass_init();
    void create_renderpass();
    void create_swapchain();
    void destroy_device();
    void create_index_buffer();
    extAndLayerInfo getExtAndLayersInfo() noexcept;
    void print_supported_extantions();
    void create_instance();
    void draw_frame();
    const char **enumerateExtations(uint32_t *);
    void print_required_extantions();
    void destroy_instance();
    void createLogicalDevice();
    void destroy_surface();
};