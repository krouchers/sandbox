#include <vulkan_context.h>
#include <swapchain.h>
#include <graphic_pipeline.h>
#include <application.h>
#include <buffer.h>
// std
#include <stdexcept>
#include <iostream>
#include <string.h>

extAndLayerInfo vulkan_context::getExtAndLayersInfo() noexcept
{
    extAndLayerInfo info{};
    info.extCount = required_extantions.size();
    info.ppExt = required_extantions.data();
    info.layersCount = required_layers.size();
    info.ppLayers = required_layers.data();
    return info;
}
void vulkan_context::destroy_device()
{
    _p_logical_device->~logical_device();
}
vulkan_context::~vulkan_context()
{
    _index_buffer = nullptr;
    _vertex_buffer = nullptr;
    _graphic_pipeline = nullptr;
    _renderpass = nullptr;
    debugMessenger = nullptr;
    _swapchain = nullptr;
    _p_logical_device = nullptr;
    _physical_device = nullptr;
    _swapchain = nullptr;
    destroy_instance();
}

const char **vulkan_context::enumerateExtations(uint32_t *extantion_count)
{
    return glfwGetRequiredInstanceExtensions(extantion_count);
}

bool vulkan_context::checkRequestedLayersSupport()
{
    uint32_t count;
    vkEnumerateInstanceLayerProperties(&count, nullptr);
    std::vector<VkLayerProperties> layers(count);
    vkEnumerateInstanceLayerProperties(&count, layers.data());

    bool is_layer_found = true;
    for (const auto layer : required_layers)
    {
        for (const auto &supported_layer : layers)
        {
            if (strcmp(layer, supported_layer.layerName) == 0)
            {
                is_layer_found = true;
                break;
            }
            is_layer_found = false;
        }
    }
#ifdef DEBUG
    std::cout << "Supported layers: \n";
    for (const auto &layer : layers)
    {
        std::cout << layer.layerName << std::endl;
    }
#endif
    return is_layer_found;
}

void vulkan_context::create_instance()
{
    uint32_t extantion_count{0};
    const char **extentions = enumerateExtations(&extantion_count);
    required_extantions = std::vector<const char *>{extentions, extentions + extantion_count};
#ifdef DEBUG
    required_extantions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif
    VkInstanceCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    if (is_debug_enabled)
    {
        if (!checkRequestedLayersSupport())
        {
            throw std::runtime_error("Required layers are not supported");
        }
        info.ppEnabledLayerNames = required_layers.data();
        info.enabledLayerCount = required_layers.size();
    }
    info.enabledExtensionCount = required_extantions.size();
    info.ppEnabledExtensionNames = required_extantions.data();
    VkDebugUtilsMessengerCreateInfoEXT debugInfo{};
    debug_messenger::populateDebugMessengerCreateInfo(debugInfo);
    info.pNext = &debugInfo;
    if (vkCreateInstance(&info, nullptr, &instance) != VK_SUCCESS)
    {
        std::runtime_error("Cant create VkInstance");
    }

#ifdef DEBUG
    print_required_extantions();
    print_supported_extantions();
#endif
}

void print_supported_layers(std::vector<VkLayerProperties> layers)
{
    for (const auto &layer : layers)
    {
        std::cout << layer.layerName << std::endl;
        std::cout << layer.description << std::endl
                  << std::endl;
    }
}
void vulkan_context::print_supported_extantions()
{
    printf("Supported extentions: \n");
    uint32_t count{0};
    vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr);
    std::vector<VkExtensionProperties> extentions(count);
    vkEnumerateInstanceExtensionProperties(nullptr, &count, extentions.data());
    for (auto extantion : extentions)
    {
        std::cout << extantion.extensionName << "\n";
    }
    std::cout << "\n";
}
void vulkan_context::print_required_extantions()
{
    printf("Required extentions: \n");
    if (required_extantions.size() != 0)
        for (const auto &extantion : required_extantions)
        {
            printf(extantion);
            printf("\n");
        }
    else
        printf("no one extention found");
    std::cout << std::endl;
}

vulkan_context::vulkan_context(Window &wnd, bool is_debug_en) : is_debug_enabled{is_debug_en}, window{wnd}
{
    create_instance();
#ifdef DEBUG
    debugMessenger = std::make_unique<debug_messenger>(instance);
#endif
    create_surface();
    initPhysicalDevice();
    createLogicalDevice();
    create_swapchain();
    graphic_pipeline_init();
    renderpass_init();
    create_renderpass();
    create_graphic_pipeline();
    create_command_buffers();
    create_sync_objects();
}
template <typename T>
void set_buffer_data(buffer<T> &buf, std::vector<T> data)
{
    buf.set_data(data);
}

void vulkan_context::draw_frame()
{

    buffer staged_vertex_buffer = create_staged_vertex_buffer(get_vertex_buffer().data());
    staged_vertex_buffer.dispatch_vertex_data();
    get_vertex_buffer().copy_buffer(staged_vertex_buffer);

    buffer staged_index_buffer = create_staged_index_buffer(get_index_buffer().data());
    staged_index_buffer.dispatch_vertex_data();
    get_index_buffer().copy_buffer(staged_index_buffer);

    _swapchain->draw_frame();
}
buffer<uint32_t> vulkan_context::create_staged_index_buffer(std::vector<uint32_t> &data)
{
    buffer<uint32_t> staged_vertex_buffer{
        *this, data,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
            VK_MEMORY_PROPERTY_HOST_COHERENT_BIT};
    return staged_vertex_buffer;
}
buffer<Vertex> vulkan_context::create_staged_vertex_buffer(std::vector<Vertex> &data)
{
    buffer<Vertex> staged_vertex_buffer{
        *this, data,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
            VK_MEMORY_PROPERTY_HOST_COHERENT_BIT};

    return staged_vertex_buffer;
}

void vulkan_context::create_vertex_buffer(std::vector<Vertex> &data)
{
    _vertex_buffer = std::make_unique<buffer<Vertex>>(
        *this, data,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
}
void vulkan_context::transfer_to_local_memory_vertex_data(std::vector<Vertex> &&data)
{

    buffer<Vertex> staged_vertex_buffer{
        *this, data, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT};

    staged_vertex_buffer.dispatch_vertex_data();

    _vertex_buffer->copy_buffer(staged_vertex_buffer);
}
void vulkan_context::create_sync_objects()
{
    _swapchain->create_sync_objects();
}
void vulkan_context::create_graphic_pipeline()
{
    _graphic_pipeline->create_graphic_pipeline();
}
void vulkan_context::graphic_pipeline_init()
{
    _graphic_pipeline = std::make_unique<graphic_pipeline>(*this);
}

void vulkan_context::create_surface()
{
    _swapchain = std::make_unique<swapchain>(*this);
    _swapchain->create_surface();
}

void vulkan_context::create_swapchain()
{
    _swapchain->create_swapchain();
}

void vulkan_context::destroy_instance()
{
    vkDestroyInstance(instance, nullptr);
}

void vulkan_context::initPhysicalDevice() noexcept
{
    _physical_device = std::make_unique<physicalDevice>(*this);
}

void vulkan_context::createLogicalDevice()
{
    _p_logical_device = std::make_unique<logical_device>(*this);
}

VkInstance vulkan_context::get_instance()
{
    return instance;
}

Window &vulkan_context::getWindow()
{
    return window;
}

physicalDevice &vulkan_context::get_physical_device()
{
    return *_physical_device.get();
}

swapchain &vulkan_context::get_swapchain()
{
    return *_swapchain.get();
}

logical_device &vulkan_context::get_logical_device()
{
    return *_p_logical_device.get();
}

void vulkan_context::renderpass_init()
{
    _renderpass = std::make_unique<renderpass>(*this);
}

void vulkan_context::create_renderpass()
{
    _renderpass->create_renderpass();
    _renderpass->create_framebuffers();
}

renderpass &vulkan_context::get_renderpass()
{
    return *_renderpass.get();
}

void vulkan_context::create_command_buffers()
{
    _swapchain->create_command_pools();
    _swapchain->create_command_buffers();
}

graphic_pipeline &vulkan_context::get_pipeline()
{
    return *_graphic_pipeline.get();
}

void vulkan_context::device_idle()
{
    vkDeviceWaitIdle(_p_logical_device->get_vk_handler());
}

buffer<Vertex> &vulkan_context::get_vertex_buffer()
{
    return *_vertex_buffer.get();
}

void vulkan_context::destroy_staged_vertex_buffer()
{
    _vertex_buffer = nullptr;
}
void vulkan_context::destroy_final_vertex_buffer()
{
    _vertex_buffer = nullptr;
}

void vulkan_context::transfer_to_local_memory_index_buffer(std::vector<uint32_t> &data)
{
    buffer<uint32_t> staged_index_buffer{
        *this, data,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
            VK_MEMORY_PROPERTY_HOST_COHERENT_BIT};
    staged_index_buffer.dispatch_vertex_data();

    _index_buffer->copy_buffer(staged_index_buffer);
}

buffer<uint32_t> &vulkan_context::get_index_buffer()
{
    return *_index_buffer.get();
}

void vulkan_context::create_index_buffer(std::vector<uint32_t> &data)
{
    _index_buffer = std::make_unique<buffer<uint32_t>>(
        *this, data, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
}
