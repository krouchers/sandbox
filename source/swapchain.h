#pragma once
#include <window.h>
#include <vulkan/vulkan_core.h>
#include <vulkan_context.h>
#include <swapchain.h>

enum pool_type
{
    GRAPHIC,
    TRANSFER,

};

class swapchain
{
    const size_t _max_frames_in_flight = 2;

    std::vector<VkCommandBuffer> _command_buffers;

    std::vector<VkSemaphore> _is_render_finished_semaphores;
    std::vector<VkSemaphore> _is_image_available_semaphores;

    std::vector<VkFence> is_can_submit_work_to_GPU;

    VkSurfaceKHR _surface;
    vulkan_context &_vk_context;
    VkSwapchainKHR _swapchain;
    VkExtent2D _extent;
    VkViewport m_viewport;
    VkRect2D m_scissors;
    uint32_t _image_count;
    VkPresentModeKHR _present_mode;
    std::vector<uint32_t> _family_indices;

    VkPresentModeKHR choose_present_mode(std::vector<VkPresentModeKHR> mods);
    VkSurfaceFormatKHR choose_surface_format(std::vector<VkSurfaceFormatKHR> formats);
    VkExtent2D choose_extent(VkSurfaceCapabilitiesKHR capabilities);

    VkSurfaceFormatKHR _surface_format;

    std::vector<VkImage> swapchainImages;
    std::vector<VkImageView> swapchainImageViews;

    std::vector<buffer<uniform_buffer_object>> _uniform_buffers;

    VkCommandPool _graphic_command_pool;
    VkCommandPool _transfer_command_pool;

    VkQueue graphic_queue;
    VkQueue present_queue;
    VkQueue transfer_queue;

public:
    swapchain(vulkan_context &context);
    ~swapchain();

    swapchain(const swapchain &) = delete;
    swapchain(const swapchain &&) = delete;
    swapchain &operator=(const swapchain &) = delete;

    void create_image_views();
    void destroy_image_views();
    void destroy_swapchain();
    void record_buffer(VkCommandBuffer command_buffer, uint32_t image_index);
    void draw_frame();
    void create_sync_objects();
    void create_command_pools();
    void create_command_buffers();
    void create_uniform_buffers();
    void create_swapchain(const VkExtent2D &new_extent = {0, 0});
    void recreate_swapchain();
    void create_surface();
    void record_all_command_buffers();
    // geters
    VkQueue *get_grapchic_queue();
    VkQueue *get_present_queue();
    VkQueue *get_transfer_queue();
    size_t get_max_frames_in_flight();
    VkCommandPool &get_command_pool(pool_type type);
    std::vector<VkImageView> &get_swapchain_imageveiws();
    VkSurfaceKHR get_surface();
    VkExtent2D get_extent();
    VkSurfaceFormatKHR get_surface_format();
    VkImageView &get_image_view(uint32_t image_index);
    //
};