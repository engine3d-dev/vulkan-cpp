#include <array>
#include <print>

// This is required to select the correct extension for specific platform
#include <vulkan-cpp/imports.hpp>

#include <vulkan-cpp/utilities.hpp>
#include <vulkan-cpp/instance.hpp>
#include <vulkan-cpp/physical_device.hpp>
#include <vulkan-cpp/device.hpp>
#include <vulkan-cpp/device_queue.hpp>
#include <vulkan-cpp/surface.hpp>
#include <vulkan-cpp/swapchain.hpp>
#include <vulkan-cpp/device_present_queue.hpp>
#include <vulkan-cpp/command_buffer.hpp>
#include <vulkan-cpp/renderpass.hpp>
#include <vulkan-cpp/framebuffer.hpp>
#include <vulkan-cpp/sample_image.hpp>

#include <vulkan-cpp/shader_resource.hpp>
#include <vulkan-cpp/pipeline.hpp>
#include <vulkan-cpp/vertex_buffer.hpp>
#include <vulkan-cpp/index_buffer.hpp>
#include <vulkan-cpp/uniform_buffer.hpp>
#include <vulkan-cpp/descriptor_resource.hpp>
#include <vulkan-cpp/texture.hpp>

#include <chrono>
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>
#include "obj_model.hpp"

static VKAPI_ATTR VkBool32 VKAPI_CALL
debug_callback(
  [[maybe_unused]] VkDebugUtilsMessageSeverityFlagBitsEXT p_message_severity,
  [[maybe_unused]] VkDebugUtilsMessageTypeFlagsEXT p_message_type,
  const VkDebugUtilsMessengerCallbackDataEXT* p_callback_data,
  [[maybe_unused]] void* p_user_data) {
    std::print("validation layer:\t\t{}\n\n", p_callback_data->pMessage);
    return false;
}

std::vector<const char*>
initialize_instance_extensions() {
    std::vector<const char*> extension_names;

    extension_names.emplace_back(VK_KHR_SURFACE_EXTENSION_NAME);
    extension_names.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

    // An additional surface extension needs to be loaded. This extension is
    // platform-specific so needs to be selected based on the platform the
    // example is going to be deployed to. Preprocessor directives are used
    // here to select the correct platform.
#ifdef VK_USE_PLATFORM_WIN32_KHR
    extension_names.emplace_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#endif
#ifdef VK_USE_PLATFORM_XLIB_KHR
    extensionNames.emplace_back(VK_KHR_XLIB_SURFACE_EXTENSION_NAME);
#endif
#ifdef VK_USE_PLATFORM_XCB_KHR
    extensionNames.emplace_back(VK_KHR_XCB_SURFACE_EXTENSION_NAME);
#endif
#ifdef VK_USE_PLATFORM_ANDROID_KHR
    extensionNames.emplace_back(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME);
#endif
#ifdef VK_USE_PLATFORM_WAYLAND_KHR
    extensionNames.emplace_back(VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME);
#endif
#ifdef VK_USE_PLATFORM_MACOS_MVK
    extensionNames.emplace_back(VK_MVK_MACOS_SURFACE_EXTENSION_NAME);
#endif
#ifdef USE_PLATFORM_NULLWS
    extensionNames.emplace_back(VK_KHR_DISPLAY_EXTENSION_NAME);
#endif
    return extension_names;
}


struct global_ubo {
    glm::mat4 proj_view;
};

struct geometry_ubo {
    glm::mat4 model;
    glm::vec4 color;
};

int
main() {
    //! @note Just added the some test code to test the conan-starter setup code
    if (!glfwInit()) {
        std::println("glfwInit could not be initialized!");
        return -1;
    }

    if (!glfwVulkanSupported()) {
        std::println("GLFW: Vulkan is not supported!");
        return -1;
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    int width = 800;
    int height = 600;
    std::string title = "Hello Window";
    GLFWwindow* window =
      glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);

    glfwMakeContextCurrent(window);

    std::array<const char*, 1> validation_layers = {
        "VK_LAYER_KHRONOS_validation",
    };

    // setting up extensions
    std::vector<const char*> global_extensions =
      initialize_instance_extensions();

    vk::debug_message_utility debug_callback_info = {
        .severity =
          vk::message::verbose | vk::message::warning | vk::message::error,
        .message_type =
          vk::debug::general | vk::debug::validation | vk::debug::performance,
        .callback = debug_callback
    };

    vk::application_params config = {
        .name = "vulkan instance",
        .version = vk::api_version::vk_1_3, // using vulkan 1.3
        .validations =
          validation_layers,
        .extensions = global_extensions
    };

    // 1. Setting up vk instance
    vk::instance api_instance(config, debug_callback_info);

    if (api_instance.alive()) {
        std::println("\napi_instance alive and initiated!!!");
    }

    // TODO: Implement this as a way to setup physical devices
    // vk::enumerate_physical_devices(vk::instance) -> returns
    // std::span<vk::physical_device>

    // setting up physical device
	// TODO: Probably enforce the use of vk::enumerate_physical_device({.device_type = vk::physical::discrete})
    vk::physical_enumeration enumerate_devices{ .device_type =
                                                  vk::physical::discrete };
    vk::physical_device physical_device(api_instance, enumerate_devices);

    // selecting depth format
    std::array<VkFormat, 3> format_support = { VK_FORMAT_D32_SFLOAT,
                                               VK_FORMAT_D32_SFLOAT_S8_UINT,
                                               VK_FORMAT_D24_UNORM_S8_UINT };

    // We provide a selection of format support that we want to check is
    // supported on current hardware device.
    VkFormat depth_format =
      vk::select_depth_format(physical_device, format_support);

    vk::queue_indices queue_indices = physical_device.family_indices();
    std::println("Graphics Queue Family Index = {}", queue_indices.graphics);
    std::println("Compute Queue Family Index = {}", queue_indices.compute);
    std::println("Transfer Queue Family Index = {}", queue_indices.transfer);

    // setting up logical device
    std::array<float, 1> priorities = { 0.f };
    std::array<const char*, 2> extensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME, "VK_EXT_descriptor_indexing" };
    vk::device_enumeration logical_device_enumeration = {
        .queue_priorities = priorities,
        .extensions = extensions,
        .queue_family_index = 0,
    };

    vk::device logical_device(physical_device, logical_device_enumeration);

    vk::surface window_surface(api_instance, window);
    std::println("Starting implementation of the swapchain!!!");

    vk::surface_enumeration surface_properties =
      vk::enumerate_surface(physical_device, window_surface);

    if (surface_properties.format.format != VK_FORMAT_UNDEFINED) {
        std::println("Surface Format.format is not undefined!!!");
    }

    vk::swapchain_enumeration enumerate_swapchain_settings = {
        .width = (uint32_t)width,
        .height = (uint32_t)height,
        .present_index =
          physical_device.family_indices()
            .graphics, // presentation index just uses the graphics index
    };
    vk::swapchain main_swapchain(logical_device,
                                 window_surface,
                                 enumerate_swapchain_settings,
                                 surface_properties);

    // querying swapchain images
	// TODO: Make the images and framebuffers contained within the vk::swapchain
	// Considering if you have two display they will prob have their own set of images to display to the two separate screens
    uint32_t image_count = 0;
    vkGetSwapchainImagesKHR(logical_device,
                            main_swapchain,
                            &image_count,
                            nullptr); // used to get the amount of images
    std::vector<VkImage> images(image_count);
    vkGetSwapchainImagesKHR(logical_device,
                            main_swapchain,
                            &image_count,
                            images.data()); // used to store in the images

    // Creating Images
    std::vector<vk::sample_image> swapchain_images(image_count);
    std::vector<vk::sample_image> swapchain_depth_images(image_count);

    VkExtent2D swapchain_extent = surface_properties.capabilities.currentExtent;

    // Setting up the images
    uint32_t layer_count = 1;
    uint32_t mip_levels = 1;
    for (uint32_t i = 0; i < swapchain_images.size(); i++) {
        vk::image_params swapchain_image_config = {
            .extent = {swapchain_extent.width, swapchain_extent.width},
            .format = surface_properties.format.format,
            .aspect = vk::image_aspect_flags::color_bit,
            // .usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
            .usage = vk::image_usage::color_attachment_bit,
            .mip_levels = 1,
            .layer_count = 1,
            .phsyical_memory_properties = physical_device.memory_properties(),
        };


        swapchain_images[i] = vk::sample_image(logical_device, images[i], swapchain_image_config);


        // Creating Images for depth buffering
        vk::image_params image_config = {
            .extent = {swapchain_extent.width, swapchain_extent.width},
            .format = depth_format,
            .aspect = vk::image_aspect_flags::depth_bit,
            // .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
            .usage = vk::image_usage::depth_stencil_bit,
            .mip_levels = 1,
            .layer_count = 1,
            .phsyical_memory_properties = physical_device.memory_properties(),
        };
        swapchain_depth_images[i] = vk::sample_image(logical_device, image_config);
    }

    // setting up command buffers
    std::vector<vk::command_buffer> swapchain_command_buffers(image_count);
    for (size_t i = 0; i < swapchain_command_buffers.size(); i++) {
        vk::command_params settings = {
            .levels = vk::command_levels::primary,
            .queue_index = enumerate_swapchain_settings.present_index,
            .flags = vk::command_pool_flags::reset,
        };

        swapchain_command_buffers[i] =
          vk::command_buffer(logical_device, settings);
    }

    // setting up renderpass

    // setting up attachments for the renderpass
    std::array<vk::attachment, 2> renderpass_attachments = {
        vk::attachment{
          .format = surface_properties.format.format,
          .layout = vk::image_layout::color_optimal,
          .samples = vk::sample_bit::count_1,
          .load = vk::attachment_load::clear,
          .store = vk::attachment_store::store,
          .stencil_load = vk::attachment_load::dont_care,
          .stencil_store = vk::attachment_store::dont_care,
          .initial_layout = vk::image_layout::undefined,
          .final_layout = vk::image_layout::present_src_khr,
        },
        vk::attachment{
          .format = depth_format,
          .layout = vk::image_layout::depth_stencil_optimal,
          .samples = vk::sample_bit::count_1,
          .load = vk::attachment_load::clear,
          .store = vk::attachment_store::dont_care,
          .stencil_load = vk::attachment_load::dont_care,
          .stencil_store = vk::attachment_store::dont_care,
          .initial_layout = vk::image_layout::undefined,
          .final_layout = vk::image_layout::depth_stencil_read_only_optimal,
        },
    };

    vk::renderpass main_renderpass(logical_device, renderpass_attachments);

    std::println("renderpass created!!!");

    // Setting up swapchain framebuffers

	std::vector<vk::framebuffer> swapchain_framebuffers(image_count);
	for (uint32_t i = 0; i < swapchain_framebuffers.size(); i++) {
		std::array<VkImageView, renderpass_attachments.size()> image_view_attachments = {
			swapchain_images[i].image_view(),
			swapchain_depth_images[i].image_view()
		};

		vk::framebuffer_params framebuffer_info = {
			.renderpass = main_renderpass,
			.views = image_view_attachments,
			.extent = swapchain_extent
		};
		swapchain_framebuffers[i] = vk::framebuffer(logical_device, framebuffer_info);
	}

    // setting up presentation queue to display commands to the screen
    vk::queue_params enumerate_present_queue{
        .family = 0,
        .index = 0,
    };
    vk::device_present_queue presentation_queue(
      logical_device, main_swapchain, enumerate_present_queue);

    // gets set with the renderpass
    std::array<float, 4> color = { 0.f, 0.5f, 0.5f, 1.f };

	std::println("Start implementing graphics pipeline!!!");

	// Now creating a vulkan graphics pipeline for the shader loading
	std::array<vk::shader_source, 2> shader_sources = {
		vk::shader_source{
			.filename = "shader_samples/sandbox-shader-samples/test.vert.spv",
			.stage = vk::shader_stage::vertex
		},
		vk::shader_source{
			.filename = "shader_samples/sandbox-shader-samples/test.frag.spv",
			.stage = vk::shader_stage::fragment
		},
	};

    // Setting up vertex attributes in the test shaders
    std::array<vk::vertex_attribute_entry, 4> attribute_entries = {
        vk::vertex_attribute_entry{
            .location = 0,
            .format = vk::format::rgb32_sfloat,
            .stride = offsetof(vk::vertex_input, position)
        },
        vk::vertex_attribute_entry{
            .location = 1,
            .format = vk::format::rgb32_sfloat,
            .stride = offsetof(vk::vertex_input, color)
        },
        vk::vertex_attribute_entry{
            .location = 2,
            .format = vk::format::rg32_sfloat,
            .stride = offsetof(vk::vertex_input, uv)
        },
        vk::vertex_attribute_entry{
            .location = 3,
            .format = vk::format::rgb32_sfloat,
            .stride = offsetof(vk::vertex_input, normals)
        }
    };

    std::array<vk::vertex_attribute, 1> attributes = {
            vk::vertex_attribute{
              // layout (set = 0, binding = 0)
              .binding = 0,
              .entries = attribute_entries,
              .stride = sizeof(vk::vertex_input),
              .input_rate = vk::input_rate::vertex,
            },
        };

    // To render triangle, we do not need to set any vertex attributes
	vk::shader_resource_info shader_info = {
		.sources = shader_sources,
		.vertex_attributes = attributes // this is to explicitly set to none, but also dont need to set this at all regardless
	};
	vk::shader_resource geometry_resource(logical_device, shader_info);
    geometry_resource.vertex_attributes(attributes);

	if(geometry_resource.is_valid()) {
		std::println("geometry resource is valid!");
	}

    // Setting up descriptor sets for graphics pipeline
    std::vector<vk::descriptor_entry> entries = {
    vk::descriptor_entry{
            // specifies "layout (set = 0, binding = 0) uniform UniformBufferObject"
            .type = vk::buffer::uniform,
            .binding_point = {
                .binding = 0,
                .stage = vk::shader_stage::vertex,
            },
            .descriptor_count = 1,
        }
    };
    // uint32_t image_count = image_count;
    vk::descriptor_layout set0_layout = {
        .slot = 0, // indicate that this is descriptor set 0
        .max_sets = image_count, // max of descriptor sets able to allocate
        .entries = entries,      // specifies pool sizes and descriptor layout
    };
    vk::descriptor_resource set0_resource(logical_device, set0_layout);

    // Loading mesh
    obj_model test_model(std::filesystem::path("asset_samples/backpack/backpack.obj"), logical_device, physical_device);
    // obj_model test_model(std::filesystem::path("asset_samples/viking_room.obj"), logical_device, physical_device, true);

    // Setting up descriptor sets for handling uniforms
    vk::uniform_buffer_info global_uniform_info = {
        .phsyical_memory_properties = physical_device.memory_properties(),
        .size_bytes = sizeof(global_ubo),
        .debug_name = "\nglobal_uniforms\n",
        .vkSetDebugUtilsObjectNameEXT = api_instance.get_debug_object_name()
    };
    vk::uniform_buffer global_uniforms = vk::uniform_buffer(logical_device, global_uniform_info);

    std::array<vk::write_buffer, 1> set0_buffers = {
        vk::write_buffer{
            .buffer = global_uniforms,
            .offset = 0,
            .range = global_uniforms.size_bytes()
        }
    };
    std::array<vk::write_buffer_descriptor, 1> uniforms = {
        vk::write_buffer_descriptor{
            .dst_binding = 0,
            .uniforms = set0_buffers
        }
    };

    set0_resource.update(uniforms);

    std::vector<vk::descriptor_entry> set1_entries = {
    vk::descriptor_entry{
            // specifies "layout (set = 1, binding = 0) uniform geometry_ubo"
            .type = vk::buffer::uniform,
            .binding_point = {
                .binding = 0,
                .stage = vk::shader_stage::vertex,
            },
            .descriptor_count = 1,
        },
        vk::descriptor_entry{
            // layout (set = 1, binding = 1) uniform sampler2D
            .type = vk::buffer::combined_image_sampler,
            .binding_point = {
                .binding = 1,
                .stage = vk::shader_stage::fragment,
            },
            .descriptor_count = 1, 
        },
        vk::descriptor_entry{
            // layout (set = 1, binding = 2) uniform sampler2D
            .type = vk::buffer::combined_image_sampler,
            .binding_point = {
                .binding = 2,
                .stage = vk::shader_stage::fragment,
            },
            .descriptor_count = 1, 
        }
    };
    // uint32_t image_count = image_count;
    vk::descriptor_layout set1_layout = {
        .slot = 1, // indicate that this is descriptor set 0
        .max_sets = image_count, // max of descriptor sets able to allocate
        .entries = set1_entries
    };
    
    vk::descriptor_resource set1 = vk::descriptor_resource(logical_device, set1_layout);

    vk::uniform_buffer_info geo_uniform_info = {
        .phsyical_memory_properties = physical_device.memory_properties(),
        .size_bytes = sizeof(geometry_ubo)
    };
    vk::uniform_buffer geometry_uniform = vk::uniform_buffer(logical_device, geo_uniform_info);

    std::array<vk::write_buffer, 1> buffers = {
        vk::write_buffer{
            .buffer = geometry_uniform,
            .offset = 0,
            .range = geometry_uniform.size_bytes()
        }
    };
    std::array<vk::write_buffer_descriptor, 1> write_set1_buffers = {
        vk::write_buffer_descriptor{
            .dst_binding = 0,
            .uniforms = buffers
        }
    };

    // Loading a texture -- for testing
    vk::texture_info diffuse_config = {
        .phsyical_memory_properties = physical_device.memory_properties(),
        .filepath = std::filesystem::path("asset_samples/backpack/diffuse.jpg")
        // .filepath = std::filesystem::path("asset_samples/viking_room.png")
    };
    vk::texture diffuse_texture(logical_device, diffuse_config);

    vk::texture_info config_texture = {
        .phsyical_memory_properties = physical_device.memory_properties(),
        .filepath = std::filesystem::path("asset_samples/backpack/specular.jpg")
    };
    vk::texture specular_texture(logical_device, config_texture);

    // layout(set = 1, binding = 1) uniform sampler2D
    std::array<vk::write_image, 1> binding1_images = {
        vk::write_image{
            .sampler = diffuse_texture.image().sampler(),
            .view = diffuse_texture.image().image_view(),
            // .image_layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
            .layout = vk::image_layout::shader_read_only_optimal
        },
    };

    std::array<vk::write_image, 1> binding2_images2 = {
        vk::write_image{
            .sampler = specular_texture.image().sampler(),
            .view = specular_texture.image().image_view(),
            // .image_layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
            .layout = vk::image_layout::shader_read_only_optimal
        }
    };

    std::vector<vk::write_image_descriptor> sample_images = {
        // layout(set = 1, binding = 1) uniform sampler2D
        vk::write_image_descriptor{
            .dst_binding = 1,
            .sample_images = binding1_images
        },
        // layout(set = 1, binding = 2) uniform sampler2D
        vk::write_image_descriptor{
            .dst_binding = 2,
            .sample_images = binding2_images2
        }
    };
    set1.update(write_set1_buffers, sample_images);




    std::array<VkDescriptorSetLayout, 2> layouts = {
        set0_resource.layout(),
        set1.layout()
    };

	/*
		// This get_pipeline_configuration can work as an easy way for specfying the vulkan configurations as an ease of setting things up
		// TODO: Probably provide a shorthand - which could work as this:
		vk::pipeline_settings pipeline_configuration = vk::get_pipeline_configuration(main_renderpass, geometry_resource);
	*/
	vk::pipeline_settings pipeline_configuration = {
		.renderpass = main_renderpass,
		.shader_modules = geometry_resource.handles(),
		.vertex_attributes = geometry_resource.vertex_attributes(),
		.vertex_bind_attributes = geometry_resource.vertex_bind_attributes(),
        .descriptor_layouts = layouts
	};
	vk::pipeline main_graphics_pipeline(logical_device, pipeline_configuration);


    /*

        Descriptor Indexing

        glsl shader


        layout(set = 0, binding = 0) readonly buffer mesh_data {
            mesh_uniform goemetry_data[]; // unbounded mesh arrays that contain all of the meshes array
        };

        In Vulkan API for specifically doing descriptor resources

        // 1.) Setup descriptor variable count allocation parameters
        uint32_t actual_descriptor_count = 50;
        VkDescriptorSetVariableDescriptorCountjAllocateInfo descriptor_count_info = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO,
            .descriptorSetCount = 1,
            .pDescriptorCounts = &actual_descriptor_count, // current size of descriptors
        };

        2.) Setup descriptor layout bindings create infos to-be chained

        2.1.) Setting up parameter flags
        VkDescriptorBindingFlags flags = 
            VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT |
            VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT; // Allows only N buffers to be bound

        2.2.) setting up the actual binding flags for descriptors
        VkDescriptorSetLayoutBindingFlagsCreateInfo bindings_flags_ci = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO,
            .pNext = nullptr,
            .bindingCount = 1,
            .pBindingFlags = &flags,
        };

        3.) Once setting up the parameters to setup descriptor indexing then we pass that into descriptor layout
        VkDescriptorSetLayoutCreateInfo layout_ci = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            // Chain the binding flags struct
            .pNext = &bindingFlagsInfo,
            // Flag the set layout for update-after-bind usage (common for bindless)
            .flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT, // will be represnted as vk::descriptor_layout_flag::update_after_bind_pool
            .bindingCount = 1,
            .pBindings = &binding,
        };

    */

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        uint32_t current_frame = presentation_queue.acquire_next_image();
        vk::command_buffer current = swapchain_command_buffers[current_frame];

        current.begin(vk::command_usage::simulatneous_use_bit);

        // renderpass begin/end must be within a recording command buffer
        vk::renderpass_begin_params begin_renderpass = {
            .current_command = current,
            .extent = swapchain_extent,
            .current_framebuffer = swapchain_framebuffers[current_frame],
            .color = color,
            .subpass = vk::subpass_contents::inline_bit
        };
        main_renderpass.begin(begin_renderpass);

        // Bind before drawing
		main_graphics_pipeline.bind(current);

        // bind obj model before drawing
        test_model.bind(current);

        static auto start_time = std::chrono::high_resolution_clock::now();

        auto current_time = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration<float, std::chrono::seconds::period>(current_time - start_time).count();

        // We set the uniforms and then we offload that to the GPU
        glm::mat4 view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f),glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        glm::mat4 proj = glm::perspective(glm::radians(45.0f),(float)swapchain_extent.width / (float)swapchain_extent.height, 0.1f, 10.0f);

        glm::mat4 proj_view = proj * view;
        proj[1][1] *= -1;

        glm::mat4 model_test = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));

        global_ubo uniform = {
            .proj_view = proj * view
        };

        geometry_ubo uniform_geometry = {
            .model = model_test,
            .color = glm::vec4(1.f, 1.f, 1.f, 1.f)
        };

        // global_uniform ubo = {
        //     .model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
        //     .view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
        //     .proj = glm::perspective(glm::radians(45.0f), (float)swapchain_extent.width / (float)swapchain_extent.height, 0.1f, 10.0f)
        // };
        // ubo.proj[1][1] *= -1;
        global_uniforms.update(&uniform);

        geometry_uniform.update(&uniform_geometry);

        // Before we can send stuff to the GPU, since we already updated the descriptor set 0 beforehand, we must bind that descriptor resource before making any of the draw calls
        // Something to note: You cannot update descriptor sets in the process of a current-recording command buffers or else that becomes undefined behavior
        set0_resource.bind(current, main_graphics_pipeline.layout());
        set1.bind(current, main_graphics_pipeline.layout());

        // Draw call here
        test_model.draw(current);

        main_renderpass.end(current);
        current.end();

        // Submitting and then presenting to the screen
        std::array<const VkCommandBuffer, 1> commands = {current};
        presentation_queue.submit_async(commands);
        presentation_queue.present_frame(current_frame);
    }

    // TODO: Make the cleanup much saner. For now we are cleaning it up like
    // Potentially bring back submit_resource_free([this](){ .. free stuff ..
    // }); (???)
    // this to ensure they are cleaned up in the proper order
    logical_device.wait();
    main_swapchain.destroy();

    diffuse_texture.destroy();
    specular_texture.destroy();
    set0_resource.destroy();
    set1.destroy();
    global_uniforms.destroy();
    geometry_uniform.destroy();
    test_model.destroy();

    for (auto& command : swapchain_command_buffers) {
        command.destroy();
    }

	for (auto& fb : swapchain_framebuffers) {
		fb.destroy();
	}

    for (auto& image : swapchain_images) {
        image.destroy();
    }

    for (auto& depth_img : swapchain_depth_images) {
        depth_img.destroy();
    }

	main_graphics_pipeline.destroy();
	geometry_resource.destroy();
    main_renderpass.destroy();
    presentation_queue.destroy();

    logical_device.destroy();
    window_surface.destroy();
    glfwDestroyWindow(window);
    api_instance.destroy();
    return 0;
}