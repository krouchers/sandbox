#include <graphic_pipeline.h>
// std
#include <fstream>
#include <iostream>
#include <stdexcept>

graphic_pipeline::graphic_pipeline(vulkan_context &vk_cont) : _vk_context{vk_cont}
{
}

graphic_pipeline::~graphic_pipeline()
{
    vkDestroyPipelineLayout(_vk_context.get_logical_device().get_vk_handler(), _pipeline_layout, nullptr);
    vkDestroyPipeline(_vk_context.get_logical_device().get_vk_handler(), _pipeline, nullptr);
};

VkShaderModule graphic_pipeline::create_shader_module(std::vector<char> shader_binary_file)
{
    VkShaderModuleCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    info.codeSize = shader_binary_file.size();
    info.pCode = reinterpret_cast<const uint32_t *>(shader_binary_file.data());

    VkShaderModule shader_module;
    if (vkCreateShaderModule(_vk_context.get_logical_device().get_vk_handler(), &info, nullptr, &shader_module) != VK_SUCCESS)
        throw std::runtime_error("failed to create shader module");

    return shader_module;
}

void graphic_pipeline::create_graphic_pipeline()
{
    auto vertex_sh_binary = read_shader_file("C:/Users/filin/coding/sandbox/source/shaders/shader.vert.spv");
    auto frag_sh_binary = read_shader_file("C:/Users/filin/coding/sandbox/source/shaders/shader.frag.spv");
#ifdef DEBUG
    std::cout << "vertext shader binary file size: " << vertex_sh_binary.size() << std::endl;
    std::cout << "fragment shader binary file size: " << frag_sh_binary.size() << std::endl;
#endif

    VkShaderModule vertex_shader_module = create_shader_module(vertex_sh_binary);
    VkShaderModule frag_shader_module = create_shader_module(frag_sh_binary);

    VkPipelineShaderStageCreateInfo vertex_create_info{};

    vertex_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertex_create_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertex_create_info.module = vertex_shader_module;
    vertex_create_info.pName = "main";

    VkPipelineShaderStageCreateInfo frag_create_info{};
    frag_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    frag_create_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    frag_create_info.module = frag_shader_module;
    frag_create_info.pName = "main";

    VkPipelineShaderStageCreateInfo shader_infos[] = {frag_create_info, vertex_create_info};


    VkPipelineVertexInputStateCreateInfo ver_input_info{};

    auto vertex_binding_descriptions = _vk_context.get_vertex_buffer().get_binding_description();
    auto vertex_atribute_descriptions = _vk_context.get_vertex_buffer().get_atribute_description();
    ver_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    ver_input_info.vertexBindingDescriptionCount = 1;
    ver_input_info.pVertexBindingDescriptions = &vertex_binding_descriptions;
    ver_input_info.vertexAttributeDescriptionCount = 2;
    ver_input_info.pVertexAttributeDescriptions = vertex_atribute_descriptions.data();

    VkPipelineInputAssemblyStateCreateInfo input_assembly_info{};
    input_assembly_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    input_assembly_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    VkViewport view_port{};
    view_port.x = 0;
    view_port.y = 0;
    view_port.width = _vk_context.get_swapchain().get_extent().width;
    view_port.height = _vk_context.get_swapchain().get_extent().height;
    view_port.minDepth = 0.0;
    view_port.maxDepth = 1.0;

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = _vk_context.get_swapchain().get_extent();

    VkPipelineViewportStateCreateInfo viewport_info{};
    viewport_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewport_info.viewportCount = 1;
    viewport_info.pViewports = &view_port;
    viewport_info.scissorCount = 1;
    viewport_info.pScissors = &scissor;

    VkPipelineRasterizationStateCreateInfo rasterization_info{};
    rasterization_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterization_info.depthClampEnable = VK_FALSE;
    rasterization_info.rasterizerDiscardEnable = VK_FALSE;
    rasterization_info.polygonMode = VK_POLYGON_MODE_FILL;
    rasterization_info.cullMode = VK_CULL_MODE_NONE;
    rasterization_info.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterization_info.depthBiasEnable = VK_FALSE;
    rasterization_info.lineWidth = 1.0f;

    VkPipelineMultisampleStateCreateInfo multisample_info{};
    multisample_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisample_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisample_info.sampleShadingEnable = VK_FALSE;

    VkPipelineColorBlendAttachmentState color_blending_info_per_attachment{};
    color_blending_info_per_attachment.blendEnable = VK_FALSE;
    color_blending_info_per_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
                                                        VK_COLOR_COMPONENT_G_BIT |
                                                        VK_COLOR_COMPONENT_B_BIT |
                                                        VK_COLOR_COMPONENT_A_BIT;
    VkPipelineColorBlendStateCreateInfo color_blending{};
    color_blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    color_blending.attachmentCount = 1;
    color_blending.pAttachments = &color_blending_info_per_attachment;

    VkPipelineLayoutCreateInfo layout_info{};
    layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    if (vkCreatePipelineLayout(_vk_context.get_logical_device().get_vk_handler(), &layout_info, nullptr, &_pipeline_layout) != VK_SUCCESS)
        throw std::runtime_error("failed to create pipeline layout");

    VkGraphicsPipelineCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    info.pVertexInputState = &ver_input_info;
    info.pInputAssemblyState = &input_assembly_info;
    info.stageCount = 2;
    info.pStages = shader_infos;
    info.pViewportState = &viewport_info;
    info.pColorBlendState = &color_blending;
    info.layout = _pipeline_layout;
    info.renderPass = _vk_context.get_renderpass().get_vk_handle();
    info.subpass = 0;
    info.pRasterizationState = &rasterization_info;
    info.pMultisampleState = &multisample_info;

    if (vkCreateGraphicsPipelines(_vk_context.get_logical_device().get_vk_handler(), nullptr, 1, &info, nullptr, &_pipeline) != VK_SUCCESS)
        throw std::runtime_error("failed to create graphic pipeline");

    vkDestroyShaderModule(_vk_context.get_logical_device().get_vk_handler(), vertex_shader_module, nullptr);
    vkDestroyShaderModule(_vk_context.get_logical_device().get_vk_handler(), frag_shader_module, nullptr);
}

std::vector<char> graphic_pipeline::read_shader_file(const char *file_name)
{
    std::ifstream file(file_name, std::ios::ate | std::ios::binary);
    if (!file.is_open())
        throw std::runtime_error("shader file is not found");
    std::vector<char> shader_binary_file(file.tellg());
    file.seekg(0);
    file.read(shader_binary_file.data(), shader_binary_file.size());
    return shader_binary_file;
}

VkPipeline graphic_pipeline::get_vk_handle()
{
    return _pipeline;
}