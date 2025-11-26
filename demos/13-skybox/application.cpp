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

// loading tinyobjloader library here
#include <tiny_obj_loader.h>

#include <vulkan-cpp/sample_image.hpp>
#include <vulkan-cpp/skybox_texture.hpp>

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

struct global_uniform {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
};

struct skybox_camera_data {
    glm::vec4 forward;
    glm::vec4 right;
    glm::vec4 up;
};

template<typename T, typename... Rest>
void hash_combine(size_t& seed, const T& v, const Rest&... rest) {
    seed ^= std::hash<T>()(v) + 0x9e3779b9 + (seed << 6) + (seed << 2);
    (hash_combine(seed, rest), ...);
}


namespace std {

    template<>
    struct hash<vk::vertex_input> {
        size_t operator()(const vk::vertex_input& vertex) const {
            size_t seed = 0;
            hash_combine(
              seed, vertex.position, vertex.color, vertex.normals, vertex.uv);
            return seed;
        }
    };
}

// This is how we are going to load a .obj model for this demo
// Example of how you might want to have your own classes for loading geometry-meshes
class obj_model {
public:
    obj_model() = default;
    obj_model(const std::filesystem::path& p_filename, const VkDevice& p_device, const vk::physical_device& p_physical) {
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string warn, err;

        //! @note If we return the constructor then we can check if the mesh
        //! loaded successfully
        //! @note We also receive hints if the loading is successful!
        //! @note Return default constructor automatically returns false means
        //! that mesh will return the boolean as false because it wasnt
        //! successful
        if (!tinyobj::LoadObj(&attrib,
                              &shapes,
                              &materials,
                              &warn,
                              &err,
                              p_filename.string().c_str())) {
            std::println("Could not load model from path {}", p_filename.string());
            m_is_loaded = false;
            return;
        }

        std::vector<vk::vertex_input> vertices;
        std::vector<uint32_t> indices;
        std::unordered_map<vk::vertex_input, uint32_t> unique_vertices{};

        for (const auto& shape : shapes) {
            for (const auto& index : shape.mesh.indices) {
                vk::vertex_input vertex{};

                // vertices.push_back(vertex);
                if (!unique_vertices.contains(vertex)) {
                    unique_vertices[vertex] =
                      static_cast<uint32_t>(vertices.size());
                    vertices.push_back(vertex);
                }

                if (index.vertex_index >= 0) {
                    vertex.position = {
                        attrib.vertices[3 * index.vertex_index + 0],
                        attrib.vertices[3 * index.vertex_index + 1],
                        attrib.vertices[3 * index.vertex_index + 2]
                    };

                    vertex.color = {
                        attrib.colors[3 * index.vertex_index + 0],
                        attrib.colors[3 * index.vertex_index + 1],
                        attrib.colors[3 * index.vertex_index + 2]
                    };
                }

                if (index.normal_index >= 0) {
                    vertex.normals = {
                        attrib.normals[3 * index.normal_index + 0],
                        attrib.normals[3 * index.normal_index + 1],
                        attrib.normals[3 * index.normal_index + 2]
                    };
                }

                if (index.texcoord_index >= 0) {
                    vertex.uv = {
                        attrib.texcoords[2 * index.texcoord_index + 0],
                        1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
                    };
                }

                if (!unique_vertices.contains(vertex)) {
                    unique_vertices[vertex] =
                      static_cast<uint32_t>(vertices.size());
                    vertices.push_back(vertex);
                }

                indices.push_back(unique_vertices[vertex]);
            }
        }
        vk::vertex_params vertex_info = {
            .phsyical_memory_properties = p_physical.memory_properties(),
            .vertices = vertices
        };

        vk::index_params index_info = {
            .phsyical_memory_properties = p_physical.memory_properties(),
            .indices = indices
        };
        m_vertex_buffer = vk::vertex_buffer(p_device, vertex_info);
        m_index_buffer = vk::index_buffer(p_device, index_info);
        m_is_loaded = true;
    }

    [[nodiscard]] bool loaded() const { return m_is_loaded; }

    void bind(const VkCommandBuffer& p_command) {
        m_vertex_buffer.bind(p_command);
        if(m_index_buffer.size() > 0) {
            m_index_buffer.bind(p_command);
        }
    }

    void draw(const VkCommandBuffer& p_command) {
        if(m_index_buffer.size() > 0) {
            vkCmdDrawIndexed(p_command, static_cast<uint32_t>(m_index_buffer.size()), 1, 0, 0, 0);
        }
        else {
            vkCmdDraw(p_command, m_vertex_buffer.size(), 1, 0, 0);
        }
    }

    void destroy() {
        m_vertex_buffer.destroy();
        m_index_buffer.destroy();
    }

private:
    bool m_is_loaded=false;
    vk::vertex_buffer m_vertex_buffer{};
    vk::index_buffer m_index_buffer{};
};

// template<size_t arr_size>
// void write(const VkDevice& p_device, const vk::buffer_handle& p_buffer, const std::array<float, arr_size>& p_in_buffer) {
// }


// void write_array(const VkDevice& p_device, const vk::buffer_handle& p_buffer) {
//     std::array<float, 256> buffer_to_write;
//     write<256>(p_device, p_buffer, buffer_to_write);
// }

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
    std::string title = "Skybox Example";
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
        // .severity essentially takes in vk::message::verbose,
        // vk::message::warning, vk::message::error
        .severity =
          vk::message::verbose | vk::message::warning | vk::message::error,
        // .message_type essentially takes in vk::debug. Like:
        // vk::debug::general, vk::debug::validation, vk::debug::performance
        .message_type =
          vk::debug::general | vk::debug::validation | vk::debug::performance,
        .callback = debug_callback
    };

    vk::application_params config = {
        .name = "vulkan instance",
        .version = vk::api_version::vk_1_3, // specify to using vulkan 1.3
        .validations =
          validation_layers, // .validation takes in a std::span<const char*>
        .extensions =
          global_extensions // .extensions also takes in std::span<const char*>
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
    std::array<const char*, 1> extensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
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
    for (uint32_t i = 0; i < swapchain_images.size(); i++) {

        vk::image_params swapchain_image_config = {
            .extent = {swapchain_extent.width, swapchain_extent.width},
            .format = surface_properties.format.format,
            .aspect = vk::image_aspect_flags::color_bit,
            .usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
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
            .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
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
          .store = vk::attachment_store::dont_care,
          .stencil_load = vk::attachment_load::clear,
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
          .stencil_load = vk::attachment_load::clear,
          .stencil_store = vk::attachment_store::dont_care,
          .initial_layout = vk::image_layout::undefined,
          .final_layout = vk::image_layout::present_src_khr,
        },
    };

    vk::renderpass main_renderpass(logical_device, renderpass_attachments);

    std::println("renderpass created!!!");

    // Setting up swapchain framebuffers

	std::vector<vk::framebuffer> swapchain_framebuffers(image_count);
	for (uint32_t i = 0; i < swapchain_framebuffers.size(); i++) {
		// image_view_attachments.push_back(swapchain_images[i].view);
        // image_view_attachments.push_back(swapchain_depth_images[i].view);

		// NOTE: This must match the amount of attachments the renderpass also has to match the image_view attachment for per-framebuffers as well
		// I just set the size to whatever the renderpass attachment size are to ensure this is the case
		// Since you have an image for color attachment and another image for the depth atttachment to specify
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

    std::println("Created VkFramebuffer's with size = {}",
                 swapchain_framebuffers.size());

    // setting up presentation queue to display commands to the screen
    vk::queue_enumeration enumerate_present_queue{
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
			.filename = "shader_samples/sample6/test.vert.spv",
			.stage = vk::shader_stage::vertex
		},
		vk::shader_source{
			.filename = "shader_samples/sample6/test.frag.spv",
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
            // specifies "layout (set = 0, binding = 0) uniform GlobalUbo"
            .type = vk::buffer::uniform,
            .binding_point = {
                .binding = 0,
                .stage = vk::shader_stage::vertex,
            },
            .descriptor_count = 1,
        },
        vk::descriptor_entry{
            // layout (set = 0, binding = 1) uniform sampler2D
            .type = vk::buffer::combined_image_sampler,
            .binding_point = {
                .binding = 1,
                .stage = vk::shader_stage::fragment,
            },
            .descriptor_count = 1,
        }
    };
    // uint32_t image_count = image_count;
    vk::descriptor_layout set0_layout = {
        .slot = 0, // represents as set 0
        .max_sets = image_count, // max of descriptor sets able to allocate
        .entries = entries,      // specifies pool sizes and descriptor layout
    };
    vk::descriptor_resource set0_resource(logical_device, set0_layout);

    std::array<VkDescriptorSetLayout, 1> layouts = {
        set0_resource.layout()
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

	if(main_graphics_pipeline.alive()) {
		std::println("Main graphics pipeline alive() = {}", main_graphics_pipeline.alive());
	}

    // Loading mesh
    obj_model test_model(std::filesystem::path("asset_samples/viking_room.obj"), logical_device, physical_device);

    std::println("Obj Model Load Status = {}", test_model.loaded());

    // Setting up descriptor sets for handling uniforms
    vk::uniform_buffer_info test_ubo_info = {
        .phsyical_memory_properties = physical_device.memory_properties(),
        .size_bytes = sizeof(global_uniform)
    };
    vk::uniform_buffer test_ubo = vk::uniform_buffer(logical_device, test_ubo_info);
    std::println("uniform_buffer.alive() = {}", test_ubo.alive());

    std::array<vk::write_buffer, 1> uniforms0 = {
        vk::write_buffer{
            .buffer = test_ubo,
            .offset = 0,
            .range = test_ubo.size_bytes()
        }
    };

    std::array<vk::write_buffer_descriptor, 1> uniforms = {
        vk::write_buffer_descriptor{
            .dst_binding = 0,
            .uniforms = uniforms0,
        }
    };

    // Loading a texture -- for testing
    vk::texture_info config_texture = {
        .phsyical_memory_properties = physical_device.memory_properties(),
        .filepath = std::filesystem::path("asset_samples/viking_room.png")
    };
    vk::texture texture1(logical_device, config_texture);

    std::println("texture1.valid = {}", texture1.loaded());

    // Moving update call here because now we add textures to set0
    std::array<vk::write_image, 1> samplers = {
        vk::write_image{
            .sampler = texture1.image().sampler(),
            .view = texture1.image().image_view(),
            .layout = vk::image_layout::shader_read_only_optimal,
        }
    };
    std::array<vk::write_image_descriptor, 1> sample_images = {
        vk::write_image_descriptor{
            .dst_binding = 1,
            .sample_images = samplers,
        }
    };
    set0_resource.update(uniforms, sample_images);

    // ----------------------------------------
    // Creating Skybox Resources
    // 1. Creating Skybox Camera Uniform
    // 2. Loading Skybox Shader Resources
    // 3. Loading Skybox Descriptor for GPU Resource Lookup
    // 4. Create Graphics Pipeline with those three information up front
    // ----------------------------------------

    // 1. loading uniforms for skybox
    vk::uniform_buffer_info skybox_ubo_info = {
        .phsyical_memory_properties = physical_device.memory_properties(),
        .size_bytes = sizeof(skybox_camera_data)
    };
    vk::uniform_buffer skybox_ubo = vk::uniform_buffer(logical_device, skybox_ubo_info);

    std::println("skybox_ubo.alive() = {}", skybox_ubo.alive());


    // loading in skybox shaders, vertex attributes
    std::array<vk::shader_source, 2> skybox_shader_sources = {
		vk::shader_source{
			.filename = "shader_samples/sample6/test.vert.spv",
			.stage = vk::shader_stage::vertex
		},
		vk::shader_source{
			.filename = "shader_samples/sample6/test.frag.spv",
			.stage = vk::shader_stage::fragment
		},
	};

    // Setting up vertex attributes in the test shaders
    // To render triangle, we do not need to set any vertex attributes
	vk::shader_resource_info skybox_shader_info = {
		.sources = skybox_shader_sources,
	};
	vk::shader_resource skybox_resource(logical_device, skybox_shader_info);

    // for skybox no vertex attributes needed
    // geometry_resource.vertex_attributes(skybox_vertex_attributes);

    // Creating skybox descriptors
    std::vector<vk::descriptor_entry> skybox_descriptor_entries = {
    vk::descriptor_entry{
            // specifies "layout (set = 0, binding = 0) uniform CameraData" in skybox.vert shader
            .type = vk::buffer::uniform,
            .binding_point = {
                .binding = 0,
                .stage = vk::shader_stage::vertex,
            },
            .descriptor_count = 1,
        },
        vk::descriptor_entry{
            // specifies "layout (set = 0, binding = 0) uniform CameraData" in skybox.vert shader
            .type = vk::buffer::combined_image_sampler,
            .binding_point = {
                .binding = 1,
                .stage = vk::shader_stage::fragment,
            },
            .descriptor_count = 1,
        },
    };

    // in skybox shader, this descriptor set is for set 0 in the skybox shader
    vk::descriptor_layout skybox_layout = {
        .slot = 0, // indicate that this is descriptor set 1
        .max_sets = image_count, // max of descriptor sets able to allocate
        .entries = skybox_descriptor_entries,      // specifies pool sizes and descriptor layout
    };
    
    // descriptor for skybox-specific resources on the GPU
    vk::descriptor_resource skybox_descriptor(logical_device, skybox_layout);

    // Pass the layouts to the skybox graphics pipeline
    std::array<VkDescriptorSetLayout, 1> skybox_layouts = {
        skybox_descriptor.layout()
    };

    // Creating skybox graphics pipeline
	vk::pipeline_settings skybox_pipeline_configuration = {
		.renderpass = main_renderpass,
		.shader_modules = geometry_resource.handles(),
		.vertex_attributes = geometry_resource.vertex_attributes(),
		.vertex_bind_attributes = geometry_resource.vertex_bind_attributes(),
        .descriptor_layouts = skybox_layouts
	};

    // skybox renderpass and graphics pipeline specification
    
    // separate render operation for the skybox
    vk::renderpass skybox_renderpass;

    // separate graphics pipeline for loading the skybox shaders
    vk::pipeline skybox_graphics_pipeline(logical_device, skybox_pipeline_configuration);





    // Loading Skybox

    std::array<std::string, 6> faces = {
        "asset_samples/skybox/front.jpg",
        "asset_samples/skybox/back.jpg",
        "asset_samples/skybox/top.jpg",
        "asset_samples/skybox/bottom.jpg",
        "asset_samples/skybox/right.jpg",
        "asset_samples/skybox/left.jpg"
    };

    // std::array<vk::texture, 6> skybox_textures;
    // for(size_t i = 0; i < faces.size(); i++) {
    //     vk::texture_info skybox_texture_info = {
    //         .physical = physical_device,
    //         .filepath = std::filesystem::path(faces[i]),
    //     };
    //     skybox_textures[i] = vk::texture(logical_device, skybox_texture_info);

    //     if(skybox_textures[i].loaded()) {
    //         std::println("Skybox Texture[{}] {} loaded!", i, faces[i]);
    //     }
    //     else {
    //         std::println("Skybox Texture {} not loaded!!!", faces[i]);
    //     }
    // }


    // vk::skybox_texture_info skybox_properties = {
    //     .physical_handle = physical_device,
    //     .faces = faces,
    // };
    // vk::skybox_texture skybox_textures(logical_device, skybox_properties);

    // vk::buffer_handle test_buffer;
    // write_array(logical_device, test_buffer);


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

        // Binding a graphics pipeline -- before drawing stuff
		// Inside of this graphics pipeline bind, is where you want to do the drawing stuff to
		main_graphics_pipeline.bind(current);

        // Must be binded before descriptor resource gets binded
        test_model.bind(current);

        static auto start_time = std::chrono::high_resolution_clock::now();

        auto current_time = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration<float, std::chrono::seconds::period>(current_time - start_time).count();

        // We set the uniforms and then we offload that to the GPU
        global_uniform ubo = {
            .model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
            .view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
            .proj = glm::perspective(glm::radians(45.0f), (float)swapchain_extent.width / (float)swapchain_extent.height, 0.1f, 10.0f)
        };
        ubo.proj[1][1] *= -1;
        test_ubo.update(&ubo);

        // Last thing that we set is going to be the skybox
        // This is setting the skybox uniform within the skybox.vert shader
        skybox_camera_data skybox_uniform_data = {
            .forward = { 1.f, 0.f, 0.f, 0.f},
            .right = { 0.f, -1.f, 0.f, 0.f},
            .up = { 0.f, 0.f, 1.f, 0.f},
        };
        skybox_ubo.update(&skybox_uniform_data);

        // Before we can send stuff to the GPU, since we already updated the descriptor set 0 beforehand, we must bind that descriptor resource before making any of the draw calls
        // Something to note: You cannot update descriptor sets in the process of a current-recording command buffers or else that becomes undefined behavior
        set0_resource.bind(current, main_graphics_pipeline.layout());

        // Drawing-call to render actual triangle to the screen
        // vkCmdDrawIndexed(current, static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
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

    // for(auto& skybox_texture : skybox_textures) {
    //     if(skybox_texture.loaded()) {
    //         skybox_texture.destroy(); 
    //     }
    // }

    skybox_ubo.destroy();
    skybox_descriptor.destroy();
    skybox_resource.destroy();
    skybox_graphics_pipeline.destroy();
    // skybox_textures.destroy();

    texture1.destroy();
    set0_resource.destroy();
    test_ubo.destroy();
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