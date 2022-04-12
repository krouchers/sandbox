#include <interface.h>
#include <stdexcept>
#include <imgui_impl_vulkan.h>
#include <swapchain.h>

interface::interface(Window &wnd) : _wnd{wnd}
{
}

void interface::init_interface(vulkan_context *vk_cont)
{
    _vk_context = vk_cont;
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForVulkan(_wnd.getglfwWindow(), true);
    create_descriptor_pool();

    ImGui_ImplVulkan_InitInfo init_info{};
    init_info.Instance = _vk_context->get_instance();
    init_info.PhysicalDevice = _vk_context->get_physical_device().getVkHandler();
    init_info.Device = _vk_context->get_logical_device().get_vk_handler();
    init_info.QueueFamily = _vk_context->get_physical_device().queueFamilies.presentFamily.value();
    init_info.Queue = *_vk_context->get_swapchain().get_present_queue();
    init_info.DescriptorPool = _descriptor_pool;
    init_info.MinImageCount = 2;                   // >= 2
    init_info.ImageCount = 2;                      // >= MinImageCount
    init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT; // >= VK_SAMPLE_COUNT_1_BIT (0 -> default to VK_SAMPLE_COUNT_1_BIT)

    ImGui_ImplVulkan_Init(&init_info, _vk_context->get_renderpass().get_vk_handle());
}

void interface::create_descriptor_pool()
{
    VkDescriptorPoolSize pool_sizes[] = {
        {VK_DESCRIPTOR_TYPE_SAMPLER, 1000},
        {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000},
        {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000},
        {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000},
        {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000},
        {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000},
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000},
        {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000},
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000},
        {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000},
        {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000}};

    VkDescriptorPoolCreateInfo pool_info{};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    pool_info.maxSets = 1000 * std::size(pool_sizes);
    pool_info.poolSizeCount = std::size(pool_sizes);
    pool_info.pPoolSizes = pool_sizes;
    if (vkCreateDescriptorPool(_vk_context->get_logical_device().get_vk_handler(),
                               &pool_info, nullptr, &_descriptor_pool) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create descriptor pool for interface");
    }
}

interface::~interface()
{
    vkDestroyDescriptorPool(_vk_context->get_logical_device().get_vk_handler(),
                            _descriptor_pool, nullptr);
}
