#pragma once
#include <debug.h>
#include <physicalDevice.h>
#include <logical_device.h>
#include <renderpass.h>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <interface.h>

// std
#include <vector>
#include <string>
#include <memory>
//

class Window;
class mesh;
class sampler;
class texture;
class swapchain;
class interface;
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
    glm::vec3 position;
    glm::vec3 color;
    glm::vec2 texture_coord;
    glm::vec3 normal;

    Vertex() = default;
    ~Vertex() = default;
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
    Window &window;
    VkInstance instance;
    std::vector<const char *> required_extantions;
    std::vector<const char *> required_layers{"VK_LAYER_KHRONOS_validation"};
    bool checkRequestedLayersSupport();
    std::unique_ptr<debug_messenger> debugMessenger;
    std::unique_ptr<physicalDevice> _physical_device;
    std::unique_ptr<logical_device> _p_logical_device;
    std::unique_ptr<graphic_pipeline> _graphic_pipeline;
    std::unique_ptr<swapchain> _swapchain;
    std::unique_ptr<renderpass> _renderpass;
    std::unique_ptr<buffer<Vertex>> _vertex_buffer;
    std::unique_ptr<buffer<uint32_t>> _index_buffer;
    std::unique_ptr<texture> _texture;
    std::unique_ptr<sampler> _sampler;

    gui::interface *_interface;

    const bool is_debug_enabled;
    std::vector<std::unique_ptr<buffer<uniform_buffer_object>>> _ubos;
    std::vector<VkDescriptorSet> _descriptor_sets;
    VkDescriptorPool _descriptor_pool;

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
    gui::interface *get_interface();
    sampler &get_sampler();
    texture &get_texture();
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
    void destroy_current_swapchain();
    //
    // seters
    template <typename T>
    void set_buffer_data(buffer<T> &buf, std::vector<T> data);
    //
    void device_idle();
    void create_texture(const std::string path);
    void ubos_init();
    void interface_init();
    void update_ubo(uint32_t current_frame);
    void destroy_staged_vertex_buffer();
    void destroy_final_vertex_buffer();
    void destroy_final_index_buffer();
    void sampler_init();
    void create_vertex_buffer(size_t size);
    buffer<Vertex> create_staged_vertex_buffer(std::vector<Vertex> &data);
    buffer<uint32_t> create_staged_index_buffer(std::vector<uint32_t> &data);
    void create_final_vertex_buffer(size_t);
    void create_sync_objects();
    void create_image_views();
    void create_descriptor_pool();
    void populate_descriptors();
    void allocate_descriptor_sets();
    void create_command_buffers();
    void create_surface();
    void renderpass_init();
    void create_renderpass();
    void create_swapchain();
    void recreate_swapchain();
    void write_descriptor_sets();
    void init_interface(gui::interface *inter, rotation_state *rot_state);
    void destroy_device();
    void create_index_buffer(size_t size);
    extAndLayerInfo getExtAndLayersInfo() noexcept;
    void print_supported_extantions();
    void create_instance();
    void draw_frame();
    void create_image(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling,
                      VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage &image, VkDeviceMemory &memory);
    uint32_t find_memory_type(uint32_t memory_type_filter, VkMemoryPropertyFlags properties);
    VkFormat find_supported_format(const std::vector<VkFormat> &candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
    VkFormat find_depth_format();
    VkImageView create_image_view(VkImage &image, VkFormat format, VkImageAspectFlags aspect);
    const char **enumerateExtations(uint32_t *);
    VkCommandBuffer begin_single_time_commands();
    void end_single_time_commands(VkCommandBuffer command_buffer);
    void transition_image_layout(VkImage image, VkFormat format, VkImageAspectFlags, VkImageLayout oldLayout, VkImageLayout newLayout);
    bool hasStencilComponent(VkFormat format);
    void print_required_extantions();
    void load_mesh(mesh &);
    void destroy_instance();
    void createLogicalDevice();
    void destroy_surface();
    void transfer_to_local_memory_vertex_data(std::vector<Vertex> &&data);
    void transfer_to_local_memory_index_buffer(std::vector<uint32_t> &data);
    void create_texture_image_view();
    void texture_init(const std::string tex_path);
};
