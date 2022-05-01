#include <interface.h>
#include <stdexcept>
#include <imgui_impl_vulkan.h>
#include <swapchain.h>
#include <vulkan/vulkan_core.h>
#include <graphic_pipeline.h>
#include <file_stream.h>
#include <application.h>
#include <mesh.h>
#include <string>

namespace gui
{
    void interface::create_command_buffer()
    {
        _command_buffers.resize(2);
        VkCommandBufferAllocateInfo alloc_info{};
        alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        alloc_info.commandPool = _vk_engine->get_vk_context()->get_swapchain().get_command_pool(GRAPHIC);
        alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        alloc_info.commandBufferCount = 2;
        vkAllocateCommandBuffers(_vk_engine->get_vk_context()->get_logical_device().get_vk_handler(),
                                 &alloc_info, _command_buffers.data());
    }
    interface::interface(Window &wnd, vk_engine *vk_eng)
        : _vk_engine{vk_eng},
          _wnd{wnd},
          main_window_active{true},
          first_problem{false},
          _current_problem{problems::CUBE},
          _previous_problem{problems::CUBE}
    {
        init_interface();
    }

    void interface::init_interface()
    {
        ImGui::CreateContext();
        ImGui::StyleColorsDark();

        ImGui_ImplGlfw_InitForVulkan(_wnd.getglfwWindow(), true);

        create_descriptor_pool();
        create_renderpass();
        create_frame_buffers();
        create_command_buffer();

        ImGui_ImplVulkan_InitInfo init_info{};
        init_info.Instance = _vk_engine->get_vk_context()->get_instance();
        init_info.PhysicalDevice = _vk_engine->get_vk_context()->get_physical_device().getVkHandler();
        init_info.Device = _vk_engine->get_vk_context()->get_logical_device().get_vk_handler();
        init_info.QueueFamily = _vk_engine->get_vk_context()->get_physical_device().queueFamilies.graphicFamily.value();
        init_info.Queue = *_vk_engine->get_vk_context()->get_swapchain().get_grapchic_queue();
        init_info.DescriptorPool = _descriptor_pool;
        init_info.Subpass = 0;
        init_info.MinImageCount = 2;                   // >= 2
        init_info.ImageCount = 2;                      // >= MinImageCount
        init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT; // >= VK_SAMPLE_COUNT_1_BIT (0 -> default to VK_SAMPLE_COUNT_1_BIT)

        ImGui_ImplVulkan_Init(&init_info, _renderpass);

        ImGuiIO io = ImGui::GetIO();
        _font = io.Fonts->AddFontFromFileTTF("../misc/LiberationMono-Italic.ttf", 20.0f, NULL, io.Fonts->GetGlyphRangesCyrillic());
        upload_fonts();
    }

    void interface::create_frame_buffers()
    {
        VkFramebufferCreateInfo framebuffer_info{};
        framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebuffer_info.renderPass = _renderpass;
        framebuffer_info.attachmentCount = 1;
        framebuffer_info.width = _vk_engine->get_vk_context()->get_swapchain().get_extent().width;
        framebuffer_info.height = _vk_engine->get_vk_context()->get_swapchain().get_extent().height;
        framebuffer_info.layers = 1;

        _frame_buffers.resize(2);
        for (size_t i = 0; i < 2; ++i)
        {
            framebuffer_info.pAttachments = &_vk_engine->get_vk_context()->get_swapchain().get_image_view(i);
            if (vkCreateFramebuffer(
                    _vk_engine->get_vk_context()->get_logical_device().get_vk_handler(),
                    &framebuffer_info, nullptr, &_frame_buffers[i]) != VK_SUCCESS)
                throw std::runtime_error("failed to create gui framebuffer");
        }
    }

    void interface::create_renderpass()
    {
        VkAttachmentDescription attachment{};
        attachment.format = _vk_engine->get_vk_context()->get_swapchain().get_surface_format().format;
        attachment.samples = VK_SAMPLE_COUNT_1_BIT;
        attachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
        attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentReference color_attachment{};
        color_attachment.attachment = 0;
        color_attachment.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &color_attachment;

        VkSubpassDependency dependency{};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        VkRenderPassCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        info.attachmentCount = 1;
        info.pAttachments = &attachment;
        info.subpassCount = 1;
        info.pSubpasses = &subpass;
        info.dependencyCount = 1;
        info.pDependencies = &dependency;
        if (vkCreateRenderPass(_vk_engine->get_vk_context()->get_logical_device().get_vk_handler(),
                               &info, nullptr, &_renderpass) != VK_SUCCESS)
            throw std::runtime_error("failed to create gui renderpass");
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
        if (vkCreateDescriptorPool(_vk_engine->get_vk_context()->get_logical_device().get_vk_handler(),
                                   &pool_info, nullptr, &_descriptor_pool) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create descriptor pool for interface");
        }
    }

    interface::~interface()
    {
        vkDeviceWaitIdle(_vk_engine->get_vk_context()->get_logical_device().get_vk_handler());
        vkDestroyDescriptorPool(_vk_engine->get_vk_context()->get_logical_device().get_vk_handler(),
                                _descriptor_pool, nullptr);
        for (auto framebuffer : _frame_buffers)
        {
            vkDestroyFramebuffer(_vk_engine->get_vk_context()->get_logical_device().get_vk_handler(), framebuffer, nullptr);
        }
        vkDestroyRenderPass(_vk_engine->get_vk_context()->get_logical_device().get_vk_handler(),
                            _renderpass, nullptr);
        ImGui_ImplGlfw_Shutdown();
        ImGui_ImplVulkan_Shutdown();
    }

    void interface::upload_fonts()
    {
        VkCommandBuffer command_buffer = _vk_engine->get_vk_context()->begin_single_time_commands();
        ImGui_ImplVulkan_CreateFontsTexture(command_buffer);
        _vk_engine->get_vk_context()->end_single_time_commands(command_buffer);
    }

    void interface::draw(uint32_t current_frame)
    {
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        create_gui_layout();
        record_command_buffer(_command_buffers[current_frame], current_frame);
    }

    void interface::record_command_buffer(VkCommandBuffer command_buffer, uint32_t image_index)
    {
        VkCommandBufferBeginInfo begin_info{};
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        if (vkBeginCommandBuffer(command_buffer, &begin_info) != VK_SUCCESS)
            throw std::runtime_error("failed to begin command buffer");

        VkRenderPassBeginInfo renderpass_befin_info{};
        renderpass_befin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderpass_befin_info.renderPass = _renderpass;
        renderpass_befin_info.framebuffer = _frame_buffers[image_index];
        renderpass_befin_info.renderArea.offset = {0, 0};
        renderpass_befin_info.renderArea.extent = _vk_engine->get_vk_context()->get_swapchain().get_extent();

        std::array<VkClearValue, 1> clear_values;
        clear_values[0] = {0.8, 0.8, 0.8, 1.0};

        renderpass_befin_info.clearValueCount = clear_values.size();
        renderpass_befin_info.pClearValues = clear_values.data();
        vkCmdBeginRenderPass(command_buffer, &renderpass_befin_info, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _vk_engine->get_vk_context()->get_pipeline().get_vk_handle());

        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), command_buffer);

        vkCmdEndRenderPass(command_buffer);
        if (vkEndCommandBuffer(command_buffer) != VK_SUCCESS)
            throw std::runtime_error("failed end gui command buffer");
    }

    VkCommandBuffer interface::get_command_buffer(uint32_t image_index)
    {
        return _command_buffers[image_index];
    }

    void interface::create_gui_layout()
    {
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize({(float)1280, (float)0});
        if (main_window_active)
        {
            ImGui::Begin(
                "Привет,мир!",
                &main_window_active,
                ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoBackground |
                    ImGuiWindowFlags_NoTitleBar);
            if (ImGui::BeginMenuBar())
                if (ImGui::BeginMenu("Задачи"))
                {
                    ImGui::SetNextWindowPos(ImVec2(0, 27));
                    ImGui::SetNextWindowSize(ImVec2(400, 502));
                    if (ImGui::MenuItem("Куб"))
                    {
                        _vk_engine->destroy_loaded_mesh();
                        mesh cube{};
                        _vk_engine->load_mesh(cube);
                        _current_problem = problems::CUBE;
                    }
                    if (ImGui::MenuItem("Параллелограм"))
                    {
                        _vk_engine->destroy_loaded_mesh();
                        mesh paral{
                            std::string("../models/paral.obj"),
                            std::string("../textures/viking_room.png")};
                        _vk_engine->load_mesh(paral);
                        _current_problem = problems::PARAL;
                    }
                    if (ImGui::MenuItem("Октаэдр"))
                    {
                        main_window_active = false;
                    }
                    if (ImGui::MenuItem("Гексаэдр"))
                    {
                        main_window_active = false;
                    }
                    if (ImGui::MenuItem("Выход"))
                    {
                    }
                    ImGui::EndMenu();
                }

            ImGui::EndMenuBar();
            ImGui::End();

            if (1)
            {
                if (ImGui::Begin("Условие", &first_problem, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize))
                {
                    std::string file_read_from{};
                    switch (_current_problem)
                    {
                    case problems::CUBE:
                    {
                        _current_problem = problems::CUBE;
                        if (_current_problem != _previous_problem)
                        {
                        }
                        file_read_from = "../misc/problems/1.txt";
                        break;
                    }
                    case problems::PARAL:
                    {
                        _current_problem = problems::PARAL;
                        if (_current_problem != _previous_problem)
                        {
                        }
                        file_read_from = "../misc/problems/2.txt";
                        break;
                    }
                    default:
                        break;
                    }
                    auto text = IO::read_text_from_file(file_read_from);
                    for (const auto &str : text)
                        ImGui::Text(str.c_str());
                    ImGui::End();
                }

                ImGui::SetNextWindowPos(ImVec2(0, 530));
                ImGui::SetNextWindowSize(ImVec2(400, 500));
                ImGui::Begin("Повернуть фигуру: ", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);
                ImGui::Text("Повернуть фигуру");
                ImGui::SliderFloat("по оси X", &(*_rot_state).x, 0, 360);
                ImGui::SliderFloat("по оси Y", &(*_rot_state).y, 0, 360);
                ImGui::SliderFloat("по оси Z", &(*_rot_state).z, 0, 360);
                ImGui::End();
            }

            // ImGui::Text(IO::read_text_from_file("../misc/problems/1.txt").c_str()); // Display some text (you can use a format strings too)
            // ImGui::Begin("Привет,мир!");             // Create a window called "Hello, world!" and append into it.
            // ImGui::End();
        }
        ImGui::Render();
    }

    void interface::set_rotation_state(rotation_state *rot_state)
    {
        _rot_state = rot_state;
    }

    const rotation_state &interface::get_rotation_state()
    {
        return *_rot_state;
    }
}