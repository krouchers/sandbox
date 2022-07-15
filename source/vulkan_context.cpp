#include <vulkan_context.h>
#include <swapchain.h>
#include <graphic_pipeline.h>
#include <application.h>
#include <buffer.h>
#include <texture.h>
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <mesh.h>
#include <interface.h>
// std
#include <stdexcept>
#include <iostream>
#include <string.h>
#include <chrono>

extAndLayerInfo
vulkan_context::getExtAndLayersInfo() noexcept
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

void vulkan_context::destroy_ubos()
{
    for (auto &ubo : _ubos)
    {
        ubo = nullptr;
    }
}

void vulkan_context::destroy_final_index_buffer()
{
    _index_buffer = nullptr;
}
vulkan_context::~vulkan_context()
{
    _sampler = nullptr;
    _texture = nullptr;
    destroy_descriptor_sets();
    destroy_ubos();
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
            printf("%s", extantion);
            printf("\n");
        }
    else
        printf("no one extention found");
    std::cout << std::endl;
}

vulkan_context::vulkan_context(Window &wnd, bool is_debug_en)
    : window{wnd}, is_debug_enabled{is_debug_en}
{
    create_instance();
#ifdef DEBUG
    debugMessenger = std::make_unique<debug_messenger>(instance);
#endif
    cam = {0.0f, 0.0f, -2.5f, 1};
    create_surface();
    initPhysicalDevice();
    createLogicalDevice();
    create_swapchain();
    create_image_views();
    graphic_pipeline_init();
    create_command_buffers();
    renderpass_init();
    create_renderpass();
    create_graphic_pipeline();
    ubos_init();
    sampler_init();
    create_sync_objects();
    // texture_init("../textures/texture.jpg");
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
        this, data,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
            VK_MEMORY_PROPERTY_HOST_COHERENT_BIT};
    return staged_vertex_buffer;
}
buffer<Vertex> vulkan_context::create_staged_vertex_buffer(std::vector<Vertex> &data)
{
    buffer<Vertex> staged_vertex_buffer{
        this, data,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
            VK_MEMORY_PROPERTY_HOST_COHERENT_BIT};

    return staged_vertex_buffer;
}

void vulkan_context::create_vertex_buffer(size_t size)
{
    _vertex_buffer = std::make_unique<buffer<Vertex>>(
        this, size,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
}
void vulkan_context::transfer_to_local_memory_vertex_data(std::vector<Vertex> &&data)
{

    buffer<Vertex> staged_vertex_buffer{
        this, data, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
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
    _renderpass = std::make_unique<renderpass>(this);
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
        this, data,
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

void vulkan_context::create_index_buffer(size_t size)
{
    _index_buffer = std::make_unique<buffer<uint32_t>>(
        this, size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
}

std::vector<std::unique_ptr<buffer<uniform_buffer_object>>> &vulkan_context::get_ubos()
{
    return _ubos;
}

void vulkan_context::ubos_init()
{
    std::vector<uniform_buffer_object> ubos{uniform_buffer_object()};
    for (size_t i = 0; i < _swapchain->get_max_frames_in_flight(); ++i)
    {
        _ubos.push_back(std::make_unique<buffer<uniform_buffer_object>>(
            this, ubos, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT));
    }
}

void vulkan_context::update_ubo(uint32_t current_frame)
{
    // static auto start_time = std::chrono::high_resolution_clock::now();
    uniform_buffer_object ubo{};
    // auto current_time = std::chrono::high_resolution_clock::now();
    // float time = std::chrono::duration<float, std::chrono::seconds::period>(current_time - start_time).count();
    ubo.model = glm::rotate(glm::mat4(1.0f), glm::radians(_interface->get_rotation_state().z), glm::vec3(0.0f, 0.0f, 1.0f)) *
                glm::rotate(glm::mat4(1.0f), glm::radians(_interface->get_rotation_state().y), glm::vec3(0.0f, 1.0f, 0.0f)) *
                glm::rotate(glm::mat4(1.0f), glm::radians(_interface->get_rotation_state().x), glm::vec3(1.0f, 0.0f, 0.0f));
    // ubo.model = glm::mat4(1.f);
    auto c_delta = window.delta;
    auto camera_rotation = glm::mat4(1.0f);
    camera_rotation = glm::rotate(camera_rotation, glm::radians(c_delta.x), glm::vec3(1.0f, 0.0f, 0.0f));
    camera_rotation = glm::rotate(camera_rotation, glm::radians(c_delta.y), glm::vec3(0.0f, 1.0f, 0.0f));
    camera_rotation = glm::rotate(camera_rotation, glm::radians(c_delta.z), glm::vec3(0.0f, 0.0f, 1.0f));

    // ubo.view = glm::lookAt(glm::vec3((camera_rotation * this->cam)), glm::vec3(.8f, 0.0f, 0.0f), glm::vec3(0, 0, 1));
    ubo.view = glm::translate(glm::mat4(1.0f), {0.5f + -window.translation.y / 100.f, -window.translation.x / 100.f, window.scroll}) * camera_rotation;
    ubo.proj = glm::perspective(glm::radians(45.0f), _swapchain->get_extent().width / (float)_swapchain->get_extent().height, 0.1f, 10.0f);
    ubo.proj[1][1] *= -1;
    auto data = std::vector<uniform_buffer_object>{ubo};
    _ubos[current_frame]->set_data(data);
}

void vulkan_context::create_descriptor_pool()
{
    VkDescriptorPoolSize ubo_size{};
    ubo_size.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    ubo_size.descriptorCount = static_cast<uint32_t>(_swapchain->get_max_frames_in_flight());

    VkDescriptorPoolSize combined_image_sampler_size{};
    combined_image_sampler_size.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    combined_image_sampler_size.descriptorCount = static_cast<uint32_t>(_swapchain->get_max_frames_in_flight());

    std::array<VkDescriptorPoolSize, 2> desciptor_pool_sizes = {ubo_size, combined_image_sampler_size};
    VkDescriptorPoolCreateInfo pool_create_info{};
    pool_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_create_info.maxSets = static_cast<uint32_t>(_swapchain->get_max_frames_in_flight());
    pool_create_info.poolSizeCount = desciptor_pool_sizes.size();
    pool_create_info.pPoolSizes = desciptor_pool_sizes.data();

    if (vkCreateDescriptorPool(_p_logical_device->get_vk_handler(), &pool_create_info, nullptr, &_descriptor_pool) != VK_SUCCESS)
        throw std::runtime_error("failes to create descriptor pool");
}

void vulkan_context::allocate_descriptor_sets()
{
    std::vector<VkDescriptorSetLayout> layouts(_swapchain->get_max_frames_in_flight(), _graphic_pipeline->get_descriptor_set_layout());
    VkDescriptorSetAllocateInfo alloc_info{};
    alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    alloc_info.descriptorPool = _descriptor_pool;
    alloc_info.descriptorSetCount = static_cast<uint32_t>(_swapchain->get_max_frames_in_flight());
    alloc_info.pSetLayouts = layouts.data();

    _descriptor_sets.resize(_swapchain->get_max_frames_in_flight());
    if (vkAllocateDescriptorSets(_p_logical_device->get_vk_handler(), &alloc_info, _descriptor_sets.data()) != VK_SUCCESS)
        throw std::runtime_error("failed to allocate descriptor sets");

    for (size_t i = 0; i < _swapchain->get_max_frames_in_flight(); ++i)
    {
        VkDescriptorBufferInfo buffer_info{};
        buffer_info.buffer = _ubos[i]->get_vk_handler();
        buffer_info.offset = 0;
        buffer_info.range = sizeof(uniform_buffer_object);

        VkDescriptorImageInfo image_info{};
        image_info.sampler = _sampler->get_vk_handle();
        image_info.imageView = _texture->get_image_view();
        image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        std::array<VkWriteDescriptorSet, 2> write_infos{};
        write_infos[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write_infos[0].dstSet = _descriptor_sets[i];
        write_infos[0].dstBinding = 0;
        write_infos[0].dstArrayElement = 0;
        write_infos[0].descriptorCount = 1;
        write_infos[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        write_infos[0].pBufferInfo = &buffer_info;

        write_infos[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write_infos[1].dstSet = _descriptor_sets[i];
        write_infos[1].dstBinding = 1;
        write_infos[1].dstArrayElement = 0;
        write_infos[1].descriptorCount = 1;
        write_infos[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        write_infos[1].pImageInfo = &image_info;
        vkUpdateDescriptorSets(_p_logical_device->get_vk_handler(), write_infos.size(), write_infos.data(), 0, nullptr);
    }
}

std::vector<VkDescriptorSet> &vulkan_context::get_descriptor_sets()
{
    return _descriptor_sets;
}

void vulkan_context::destroy_descriptor_sets()
{
    vkDestroyDescriptorPool(_p_logical_device->get_vk_handler(), _descriptor_pool, nullptr);
}

void vulkan_context::sampler_init()
{
    _sampler = std::make_unique<sampler>(this);
}

sampler &vulkan_context::get_sampler()
{
    return *_sampler.get();
}

void vulkan_context::texture_init(const std::string tex_path)
{
    _texture = std::make_unique<texture>(this, _sampler.get(), tex_path);
}

texture &vulkan_context::get_texture()
{
    return *_texture.get();
}

void vulkan_context::create_image(
    uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling,
    VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage &image, VkDeviceMemory &memory)
{
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = tiling;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = usage;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateImage(_p_logical_device->get_vk_handler(), &imageInfo, nullptr, &image) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create image!");
    }

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(_p_logical_device->get_vk_handler(), image, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = find_memory_type(memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(_p_logical_device->get_vk_handler(), &allocInfo, nullptr, &memory) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate image memory!");
    }

    vkBindImageMemory(_p_logical_device->get_vk_handler(), image, memory, 0);
}

uint32_t vulkan_context::find_memory_type(uint32_t memory_type_filter, VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties physical_device_memory_properties;
    vkGetPhysicalDeviceMemoryProperties(_physical_device->getVkHandler(), &physical_device_memory_properties);

    for (uint32_t i = 0; i < physical_device_memory_properties.memoryTypeCount; ++i)
    {
        if (memory_type_filter & (1 << i) && (physical_device_memory_properties.memoryTypes[i].propertyFlags & properties) == properties)
        {
            return i;
        }
    }

    throw std::runtime_error("failed to find suitable memory type");
}

VkFormat vulkan_context::find_supported_format(const std::vector<VkFormat> &candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
{
    for (VkFormat format : candidates)
    {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(_physical_device->getVkHandler(), format, &props);
        if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
        {
            return format;
        }
        else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
        {
            return format;
        }
    }
    throw std::runtime_error("failed to find supported format");
}

VkFormat vulkan_context::find_depth_format()
{
    return find_supported_format(
        {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

VkImageView vulkan_context::create_image_view(VkImage &image, VkFormat format, VkImageAspectFlags aspect)
{
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;
    viewInfo.subresourceRange.aspectMask = aspect;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    VkImageView imageView;
    if (vkCreateImageView(_p_logical_device->get_vk_handler(), &viewInfo, nullptr, &imageView) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create texture image view!");
    }

    return imageView;
}

VkCommandBuffer vulkan_context::begin_single_time_commands()
{
    VkCommandBufferAllocateInfo alloc_info{};
    alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    alloc_info.commandPool = _swapchain->get_command_pool(GRAPHIC);
    alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    alloc_info.commandBufferCount = 1;

    VkCommandBuffer command_buffer;

    vkAllocateCommandBuffers(_p_logical_device->get_vk_handler(), &alloc_info, &command_buffer);

    VkCommandBufferBeginInfo begin_info{};

    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(command_buffer, &begin_info);
    return command_buffer;
}

void vulkan_context::end_single_time_commands(VkCommandBuffer command_buffer)
{
    vkEndCommandBuffer(command_buffer);

    VkSubmitInfo sub_info{};
    sub_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    sub_info.commandBufferCount = 1;
    sub_info.pCommandBuffers = &command_buffer;

    vkQueueSubmit(
        *_swapchain->get_grapchic_queue(),
        1, &sub_info, VK_NULL_HANDLE);
    vkQueueWaitIdle(*_swapchain->get_grapchic_queue());
    vkFreeCommandBuffers(_p_logical_device->get_vk_handler(),
                         _swapchain->get_command_pool(GRAPHIC), 1, &command_buffer);
}

void vulkan_context::transition_image_layout(VkImage image, VkFormat format, VkImageAspectFlags aspect, VkImageLayout oldLayout, VkImageLayout newLayout)
{
    VkCommandBuffer commandBuffer = begin_single_time_commands();
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = aspect;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = 0;

    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
    {
        if (hasStencilComponent(format))
        {
            barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
        }
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    }
    else
    {
        throw std::invalid_argument("unsupported layout transition!");
    }

    vkCmdPipelineBarrier(
        commandBuffer,
        sourceStage, destinationStage,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier);

    end_single_time_commands(commandBuffer);
}

bool vulkan_context::hasStencilComponent(VkFormat format)
{
    return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}
void vulkan_context::load_mesh(mesh &mesh)
{
    create_vertex_buffer(mesh.get_vertices().size());
    get_vertex_buffer().set_data(mesh.get_vertices());
    create_index_buffer(mesh.get_indices().size());
    get_index_buffer().set_data(mesh.get_indices());
    texture_init(mesh.get_texture_path());
    _texture->set_data();
    create_descriptor_pool();
    allocate_descriptor_sets();
}

void vulkan_context::init_interface(gui::interface *inter, rotation_state *rot_state)
{
    _interface = inter;
    _interface->set_rotation_state(rot_state);
}

gui::interface *vulkan_context::get_interface()
{
    return _interface;
}

void vulkan_context::recreate_swapchain()
{
    vkDeviceWaitIdle(_p_logical_device->get_vk_handler());
    _renderpass->destroy_framebuffers();
    _swapchain->destroy_image_views();
    _renderpass->destroy_depth_buffer();
    _interface->destroy_framebuffers();
    _swapchain->destroy_swapchain();
    _swapchain->create_swapchain({window.get_width(), window.get_height()});
    _swapchain->create_image_views();
    _renderpass->create_depth_buffer();
    _renderpass->create_framebuffers();
    _interface->create_framebuffers();
}

void vulkan_context::create_image_views()
{ // TODO get to be more specific about sort of image views
    _swapchain->create_image_views();
}