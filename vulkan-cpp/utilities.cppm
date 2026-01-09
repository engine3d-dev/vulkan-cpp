module;

#include <vulkan/vulkan.h>
#include <filesystem>
#include <print>
#include <span>
#include <source_location>
#include <vector>

export module vk:utilities;

export import :types;

export namespace vk {
    inline namespace v1 {

    void vk_check(const VkResult& p_result,
                  const std::string& p_name,
                  const std::source_location& p_source={}) {
        if (p_result != VK_SUCCESS) {
            std::println(
              "File {} on line {} failed VkResult check",
              std::filesystem::relative(p_source.file_name()).string(),
              p_source.line());
            std::println("Current Function Location = {}",
                         p_source.function_name());
            std::println("{} VkResult returned: {}", p_name, (int)p_result);
        }
    }

    // VkDebugUtilsMessageSeverityFlagsEXT to_debug_message_severity(
    //   uint32_t p_flag) {
    //     VkDebugUtilsMessageSeverityFlagsEXT flag;

    //     if (p_flag & vk::message::verbose) {
    //         flag |= VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT;
    //     }
    //     else if (p_flag & vk::message::warning) {
    //         flag |= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
    //     }
    //     else if (p_flag & vk::message::error) {
    //         flag |= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    //     }

    //     return flag;
    // }

    // VkDebugUtilsMessageTypeFlagsEXT to_message_type(uint32_t p_flag) {
    //     VkDebugUtilsMessageTypeFlagsEXT flag;

    //     if (p_flag & vk::debug::general) {
    //         flag |= VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT;
    //     }
    //     else if (p_flag & vk::debug::validation) {
    //         flag |= VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
    //     }
    //     else if (p_flag & vk::debug::performance) {
    //         flag |= VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    //     }

    //     return flag;
    // }

    // uint32_t vk_api_version(const api_version& p_version) {
    //     switch (p_version) {
    //         case api_version::vk_1_2:
    //             return VK_API_VERSION_1_2;
    //         case api_version::vk_1_3:
    //             return VK_API_VERSION_1_3;
    //     }
    // }

    // VkPhysicalDeviceType vk_physical_device_type(physical p_physical_type) {
    //     switch (p_physical_type) {
    //         case physical::integrated:
    //             return VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU;
    //         case physical::discrete:
    //             return VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
    //         case physical::virtualized:
    //             return VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU;
    //         case physical::cpu:
    //             return VK_PHYSICAL_DEVICE_TYPE_CPU;
    //         case physical::max_enum:
    //             return VK_PHYSICAL_DEVICE_TYPE_MAX_ENUM;
    //         case physical::other:
    //             return VK_PHYSICAL_DEVICE_TYPE_OTHER;
    //     }

    //     throw std::runtime_error("Invalid physical device!");
    // }

    // struct physical_device_handler {
    //     VkPhysicalDevice handler = nullptr;
    // };

    std::vector<VkQueueFamilyProperties> enumerate_queue_family_properties(
      const VkPhysicalDevice& p_physical) {
        uint32_t queue_family_count = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(
          p_physical, &queue_family_count, nullptr);
        std::vector<VkQueueFamilyProperties> queue_family_properties(
          queue_family_count);

        vkGetPhysicalDeviceQueueFamilyProperties(
          p_physical, &queue_family_count, queue_family_properties.data());

        return queue_family_properties;
    }

    VkFormat select_compatible_formats(
      const VkPhysicalDevice& p_physical,
      std::span<const vk::format> p_format_selection,
      VkImageTiling p_tiling,
      VkFormatFeatureFlags p_feature_flag) {
        VkFormat format = VK_FORMAT_UNDEFINED;

        for (size_t i = 0; i < p_format_selection.size(); i++) {
            VkFormat current_format = static_cast<VkFormat>(p_format_selection[i]);
            VkFormatProperties format_properties;
            vkGetPhysicalDeviceFormatProperties(
              p_physical, current_format, &format_properties);

            if (p_tiling == VK_IMAGE_TILING_LINEAR) {
                if (format_properties.linearTilingFeatures & p_feature_flag) {
                    format = current_format;
                }
            }
            else if (p_tiling == VK_IMAGE_TILING_OPTIMAL and
                     format_properties.optimalTilingFeatures & p_feature_flag) {
                format = current_format;
            }
        }

        return format;
    }

    VkFormat select_depth_format(const VkPhysicalDevice& p_physical,
                                 std::span<const vk::format> p_format_selection) {

        VkFormat format = select_compatible_formats(
          p_physical,
          p_format_selection,
          VK_IMAGE_TILING_OPTIMAL,
          VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
        return format;
    }

    uint32_t physical_memory_properties(const VkPhysicalDevice& p_physical,
                                        uint32_t p_type_filter,
                                        VkMemoryPropertyFlags p_property_flag) {
        VkPhysicalDeviceMemoryProperties mem_props;
        vkGetPhysicalDeviceMemoryProperties(p_physical, &mem_props);

        for (uint32_t i = 0; i < mem_props.memoryTypeCount; i++) {
            if ((p_type_filter & (1 << i)) and
                (mem_props.memoryTypes[i].propertyFlags & p_property_flag) ==
                  p_property_flag) {
                return i;
            }
        }

        return -1;
    }

    // VkMemoryPropertyFlags to_memory_property_flags(memory_property p_flag) {
    //     VkMemoryPropertyFlags flags = 0;
    //     if (p_flag & memory_property::device_local_bit) {
    //         flags |= VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    //     }
    //     if (p_flag & memory_property::host_visible_bit) {
    //         flags |= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
    //     }
    //     if (p_flag & memory_property::host_coherent_bit) {
    //         flags |= VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    //     }
    //     if (p_flag & memory_property::host_cached_bit) {
    //         flags |= VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
    //     }
    //     if (p_flag & memory_property::lazily_allocated_bit) {
    //         flags |= VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT;
    //     }
    //     if (p_flag & memory_property::device_protected_bit) {
    //         flags |= VK_MEMORY_PROPERTY_PROTECTED_BIT;
    //     }
    //     if (p_flag & memory_property::device_coherent_bit_amd) {
    //         flags |= VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD;
    //     }
    //     if (p_flag & memory_property::device_uncached_bit_amd) {
    //         flags |= VK_MEMORY_PROPERTY_DEVICE_UNCACHED_BIT_AMD;
    //     }
    //     if (p_flag & memory_property::rdma_capable_bit_nv) {
    //         flags |= VK_MEMORY_PROPERTY_RDMA_CAPABLE_BIT_NV;
    //     }

    //     return flags;
    // }

    surface_params enumerate_surface(const VkPhysicalDevice& p_physical,
                                          const VkSurfaceKHR& p_surface) {
        surface_params enumerate_surface_properties{};
        vk_check(
          vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
            p_physical, p_surface, &enumerate_surface_properties.capabilities),
          "vkGetPhysicalDeviceSurfaceCapabilitiesKHR");

        uint32_t format_count = 0;
        std::vector<VkSurfaceFormatKHR> formats;
        vk_check(vkGetPhysicalDeviceSurfaceFormatsKHR(
                   p_physical, p_surface, &format_count, nullptr),
                 "vkGetPhysicalDeviceSurfaceFormatsKHR");

        formats.resize(format_count);

        vk_check(vkGetPhysicalDeviceSurfaceFormatsKHR(
                   p_physical, p_surface, &format_count, formats.data()),
                 "vkGetPhysicalDeviceSurfaceFormatsKHR");

        for (const auto& format : formats) {
            if (format.format == VK_FORMAT_B8G8R8A8_SRGB &&
                format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                enumerate_surface_properties.format = format;
            }
        }

        enumerate_surface_properties.format = formats[0];

        return enumerate_surface_properties;
    }

    uint32_t surface_image_size(
      const VkSurfaceCapabilitiesKHR& p_capabilities) {
        uint32_t requested_images = p_capabilities.minImageCount + 1;

        uint32_t final_image_count = 0;

        if ((p_capabilities.maxImageCount > 0) and
            (requested_images > p_capabilities.maxImageCount)) {
            final_image_count = p_capabilities.maxImageCount;
        }
        else {
            final_image_count = requested_images;
        }

        return final_image_count;
    }

    // VkCommandBufferUsageFlags to_command_usage_flag_bits(
    //   command_usage p_command_usage_flag) {
    //     VkCommandBufferUsageFlags command_usage_flags;
    //     if (command_usage_flags & command_usage::one_time_submit) {
    //         command_usage_flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    //     }

    //     if (command_usage_flags & command_usage::renderpass_continue_bit) {
    //         command_usage_flags |=
    //           VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    //     }

    //     if (command_usage_flags & command_usage::simulatneous_use_bit) {
    //         command_usage_flags |= VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
    //     }

    //     if (command_usage_flags & command_usage::max_bit) {
    //         command_usage_flags |= VK_COMMAND_BUFFER_USAGE_FLAG_BITS_MAX_ENUM;
    //     }

    //     return command_usage_flags;
    // }

    // VkImageAspectFlags to_image_aspect_flags(image_aspect_flags p_flag) {
    //     switch (p_flag) {
    //         case image_aspect_flags::color_bit:
    //             return VK_IMAGE_ASPECT_COLOR_BIT;
    //         case image_aspect_flags::depth_bit:
    //             return VK_IMAGE_ASPECT_DEPTH_BIT;
    //         case image_aspect_flags::stencil_bit:
    //             return VK_IMAGE_ASPECT_STENCIL_BIT;
    //         case image_aspect_flags::metadata_bit:
    //             return VK_IMAGE_ASPECT_METADATA_BIT;
    //         case image_aspect_flags::plane0_bit:
    //             return VK_IMAGE_ASPECT_PLANE_0_BIT;
    //         case image_aspect_flags::plane1_bit:
    //             return VK_IMAGE_ASPECT_PLANE_1_BIT;
    //         case image_aspect_flags::plane2_bit:
    //             return VK_IMAGE_ASPECT_PLANE_2_BIT;
    //         case image_aspect_flags::none:
    //             return VK_IMAGE_ASPECT_NONE;
    //         case image_aspect_flags::memory_plane0_bit_ext:
    //             return VK_IMAGE_ASPECT_MEMORY_PLANE_0_BIT_EXT;
    //         case image_aspect_flags::memory_plane1_bit_ext:
    //             return VK_IMAGE_ASPECT_MEMORY_PLANE_1_BIT_EXT;
    //         case image_aspect_flags::memory_plane2_bit_ext:
    //             return VK_IMAGE_ASPECT_MEMORY_PLANE_2_BIT_EXT;
    //         case image_aspect_flags::plane1_bit_khr:
    //             return VK_IMAGE_ASPECT_PLANE_1_BIT_KHR;
    //         case image_aspect_flags::plane2_bit_khr:
    //             return VK_IMAGE_ASPECT_PLANE_2_BIT_KHR;
    //         case image_aspect_flags::none_khr:
    //             return VK_IMAGE_ASPECT_NONE_KHR;
    //         case image_aspect_flags::bits_max_enum:
    //             return VK_IMAGE_ASPECT_FLAG_BITS_MAX_ENUM;
    //     }

    //     throw std::runtime_error("Invalid image aspect flags specified!!!");
    // }

    VkSampler create_sampler(const VkDevice& p_device,
                             const filter_range& p_range,
                             VkSamplerAddressMode p_address_mode) {
        VkSamplerCreateInfo sampler_info = {
            .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .magFilter = p_range.min,
            .minFilter = p_range.max,
            .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
            .addressModeU = p_address_mode,
            .addressModeV = p_address_mode,
            .addressModeW = p_address_mode,
            .mipLodBias = 0.0f,
            .anisotropyEnable = false,
            .maxAnisotropy = 1,
            .compareEnable = false,
            .compareOp = VK_COMPARE_OP_ALWAYS,
            .minLod = 0.0f,
            .maxLod = 0.0f,
            .borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
            .unnormalizedCoordinates = false
        };

        VkSampler sampler = nullptr;
        VkResult res =
          vkCreateSampler(p_device, &sampler_info, nullptr, &sampler);
        vk_check(res, "vkCreateSampler");
        return sampler;
    }

    VkSemaphore create_semaphore(const VkDevice& p_device) {
        // creating semaphores
        VkSemaphoreCreateInfo semaphore_ci = {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0
        };

        VkSemaphore semaphore;
        vk_check(
          vkCreateSemaphore(p_device, &semaphore_ci, nullptr, &semaphore),
          "vkCreateSemaphore");
        return semaphore;
    }

    uint32_t image_memory_requirements(const VkPhysicalDevice& p_physical,
                                       const VkDevice& p_device,
                                       const VkImage& p_image,
                                       memory_property p_property) {
        VkMemoryRequirements memory_requirements;
        vkGetImageMemoryRequirements(p_device, p_image, &memory_requirements);

        uint32_t type_filter = memory_requirements.memoryTypeBits;
        VkMemoryPropertyFlags property_flag = static_cast<VkMemoryPropertyFlags>(p_property);

        VkPhysicalDeviceMemoryProperties mem_props;
        vkGetPhysicalDeviceMemoryProperties(p_physical, &mem_props);

        for (uint32_t i = 0; i < mem_props.memoryTypeCount; i++) {
            if ((type_filter & (1 << i)) and
                (mem_props.memoryTypes[i].propertyFlags & property_flag) ==
                  property_flag) {
                return i;
            }
        }

        return -1;
    }

    // VkCommandBufferLevel to_vk_command_buffer_level(
    //   const command_levels& p_level) {
    //     switch (p_level) {
    //         case command_levels::primary:
    //             return VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    //         case command_levels::secondary:
    //             return VK_COMMAND_BUFFER_LEVEL_SECONDARY;
    //         case command_levels::max_enum:
    //             return VK_COMMAND_BUFFER_LEVEL_MAX_ENUM;
    //     }

    //     throw std::runtime_error("Invalid command buffer levels");
    // }

    // VkCommandPoolCreateFlagBits to_command_buffer_pool_flags(
    //   command_pool_flags p_command_pool_flag) {
    //     switch (p_command_pool_flag) {
    //         case command_pool_flags::protected_bit:
    //             return VK_COMMAND_POOL_CREATE_PROTECTED_BIT;
    //         case command_pool_flags::reset:
    //             return VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    //         case command_pool_flags::transient:
    //             return VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
    //         case command_pool_flags::max_enum_bit:
    //             return VK_COMMAND_POOL_CREATE_FLAG_BITS_MAX_ENUM;
    //     }

    //     return (VkCommandPoolCreateFlagBits)0;
    // }

    // VkSubpassContents to_subpass_contents(subpass_contents p_content) {
    //     switch (p_content) {
    //         case subpass_contents::inline_bit:
    //             return VK_SUBPASS_CONTENTS_INLINE;
    //         case subpass_contents::secondary_command:
    //             return VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS;
    //         case subpass_contents::inline_and_secondary_command_khr:
    //             return VK_SUBPASS_CONTENTS_INLINE_AND_SECONDARY_COMMAND_BUFFERS_KHR;
    //         case subpass_contents::max_enum_content:
    //             return VK_SUBPASS_CONTENTS_MAX_ENUM;
    //         default:
    //             break;
    //     }
    // }

    // VkPipelineBindPoint to_pipeline_bind_point(
    //   pipeline_bind_point p_bind_point) {
    //     switch (p_bind_point) {
    //         case pipeline_bind_point::graphics:
    //             return VK_PIPELINE_BIND_POINT_GRAPHICS;
    //         case pipeline_bind_point::compute:
    //             return VK_PIPELINE_BIND_POINT_COMPUTE;
    //         case pipeline_bind_point::ray_tracing_khr:
    //             return VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR;
    //         case pipeline_bind_point::subpass_shading_hauwei:
    //             return VK_PIPELINE_BIND_POINT_SUBPASS_SHADING_HUAWEI;
    //         // case pipeline_bind_point::ray_tracing_nv:
    //         //     return VK_PIPELINE_BIND_POINT_RAY_TRACING_NV;
    //         case pipeline_bind_point::max_enum:
    //             return VK_PIPELINE_BIND_POINT_MAX_ENUM;
    //         default:
    //             break;
    //     }
    // }


    // VkSampleCountFlagBits to_sample_count_bits(sample_bit p_sample_count_bit) {
    //     switch (p_sample_count_bit) {
    //         case sample_bit::count_1:
    //             return VK_SAMPLE_COUNT_1_BIT;
    //         case sample_bit::count_2:
    //             return VK_SAMPLE_COUNT_2_BIT;
    //         case sample_bit::count_4:
    //             return VK_SAMPLE_COUNT_4_BIT;
    //         case sample_bit::count_8:
    //             return VK_SAMPLE_COUNT_8_BIT;
    //         case sample_bit::count_16:
    //             return VK_SAMPLE_COUNT_16_BIT;
    //         case sample_bit::count_32:
    //             return VK_SAMPLE_COUNT_32_BIT;
    //         case sample_bit::count_64:
    //             return VK_SAMPLE_COUNT_64_BIT;
    //         case sample_bit::max_enum:
    //             return VK_SAMPLE_COUNT_FLAG_BITS_MAX_ENUM;
    //     }
    // }

    // VkImageLayout to_image_layout(image_layout p_layout) {
    //     switch (p_layout) {
    //         case image_layout::undefined:
    //             return VK_IMAGE_LAYOUT_UNDEFINED;
    //         case image_layout::general:
    //             return VK_IMAGE_LAYOUT_GENERAL;
    //         case image_layout::color_optimal:
    //             return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    //         case image_layout::depth_stencil_optimal:
    //             return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    //         case image_layout::depth_stencil_read_only_optimal:
    //             return VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL;
    //         case image_layout::present_src_khr:
    //             return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    //         case image_layout::shader_read_only_optimal:
    //             return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    //     }
    // }

    VkVertexInputRate to_input_rate(input_rate p_input_rate) {
        switch (p_input_rate) {
            case input_rate::vertex:
                return VK_VERTEX_INPUT_RATE_VERTEX;
            case input_rate::instance:
                return VK_VERTEX_INPUT_RATE_INSTANCE;
            default:
                return VK_VERTEX_INPUT_RATE_MAX_ENUM;
        }
    }

    bool has_depth_specified(image_layout p_layout) {
        if (p_layout == image_layout::depth_stencil_optimal) {
            return true;
        }

        if (p_layout == image_layout::depth_stencil_read_only_optimal) {
            return true;
        }
        return false;
    }

    // VkShaderStageFlags to_shader_stage(const shader_stage& p_stage) {
    //     switch (p_stage) {
    //         case shader_stage::vertex:
    //             return VK_SHADER_STAGE_VERTEX_BIT;
    //         case shader_stage::fragment:
    //             return VK_SHADER_STAGE_FRAGMENT_BIT;
    //         default:
    //             return (VkShaderStageFlagBits)0;
    //     }
    // }

    // VkFormat to_format(const format& p_format) {
    //     switch (p_format) {
    //         case format::rg32_sfloat:
    //             return VK_FORMAT_R32G32_SFLOAT;
    //         case format::rgb32_sfloat:
    //             return VK_FORMAT_R32G32B32A32_SFLOAT;
    //         default:
    //             return VK_FORMAT_UNDEFINED;
    //     }
    // }

    // VkCommandPool create_single_command_pool(const VkDevice& p_device,
    //                                          uint32_t p_queue_family_index) {
    //     // uint32_t graphics_queue_index =
    //     // physical.read_queue_family_indices().graphics;
    //     VkCommandPoolCreateInfo pool_ci = {
    //         .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
    //         .pNext = nullptr,
    //         .flags = 0,
    //         .queueFamilyIndex = p_queue_family_index
    //     };

    //     VkCommandPool command_pool = nullptr;
    //     vk_check(
    //       vkCreateCommandPool(p_device, &pool_ci, nullptr, &command_pool),
    //       "vkCreateCommandPool");

    //     return command_pool;
    // }

    // void copy(const VkDevice& p_device,
    //           const buffer_copy_info& p_info,
    //           size_t p_size_of_bytes) {

    //     // 1. Retrieve the first queue
    //     // TODO: Use vk::device_queue for this
    //     VkQueue temp_graphics_queue = nullptr;
    //     uint32_t queue_family_index = 0;
    //     uint32_t queue_index = 0;
    //     vkGetDeviceQueue(
    //       p_device, queue_family_index, queue_index, &temp_graphics_queue);

    //     // command_buffer_info
    //     command_params enumerate_command_info = {
    //         .levels = command_levels::primary,
    //         .queue_index = 0,
    //     };
    //     command_buffer copy_command_buffer(p_device, enumerate_command_info);

    //     copy_command_buffer.begin(command_usage::one_time_submit);
    //     VkBufferCopy copy_region{};
    //     copy_region.size = (VkDeviceSize)p_size_of_bytes;
    //     vkCmdCopyBuffer(
    //       copy_command_buffer, p_info.src, p_info.dst, 1, &copy_region);
    //     copy_command_buffer.end();
    //     VkCommandBuffer temp = copy_command_buffer;
    //     VkSubmitInfo submit_info{};
    //     submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    //     submit_info.commandBufferCount = 1;
    //     submit_info.pCommandBuffers = &temp;

    //     vkQueueSubmit(temp_graphics_queue, 1, &submit_info, nullptr);
    //     vkQueueWaitIdle(temp_graphics_queue);

    //     // vkFreeCommandBuffers(, command_pool, 1, &copy_cmd_buffer);
    //     // vkDestroyCommandPool(driver, command_pool, nullptr);
    //     copy_command_buffer.destroy();
    // }

    // VkDescriptorType to_descriptor_type(const buffer& p_type) {
    //     switch (p_type) {
    //         case buffer::storage:
    //             return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    //         case buffer::uniform:
    //             return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    //         case buffer::combined_image_sampler:
    //             return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    //         case buffer::sampled_only_image:
    //             return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    //     }
    // }

    int bytes_per_texture_format(VkFormat p_format) {
        switch (p_format) {
            case VK_FORMAT_R8_SINT:
            case VK_FORMAT_R8_UNORM:
                return 1;
            case VK_FORMAT_R16_SFLOAT:
                return 2;
            case VK_FORMAT_R16G16_SFLOAT:
            case VK_FORMAT_B8G8R8A8_UNORM:
            case VK_FORMAT_R8G8B8A8_UNORM:
                return 4;
            case VK_FORMAT_R16G16B16A16_SFLOAT:
                return 4 * sizeof(uint16_t);
            case VK_FORMAT_R32G32B32A32_SFLOAT:
                return 4 * sizeof(float);
            case VK_FORMAT_R8G8B8A8_SRGB:
                return 4 * sizeof(uint8_t);
            default:
                return 0;
        }

        return 0;
    }

    bool has_stencil_attachment(VkFormat p_format) {
        return ((p_format == VK_FORMAT_D32_SFLOAT_S8_UINT) ||
                (p_format == VK_FORMAT_D24_UNORM_S8_UINT));
    }

    // VkImageView create_image2d_view(
    //   const VkDevice& p_device,
    //   const VkImage& p_image,
    //   const image_params& p_config) {
    //     VkImageViewCreateInfo view_info = {
    //         .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
    //         .pNext = nullptr,
    //         .flags = 0,
    //         .image = p_image,
    //         .viewType = VK_IMAGE_VIEW_TYPE_2D,
    //         .format = p_config.format,
    //         .components = { .r = VK_COMPONENT_SWIZZLE_IDENTITY,
    //                         .g = VK_COMPONENT_SWIZZLE_IDENTITY,
    //                         .b = VK_COMPONENT_SWIZZLE_IDENTITY,
    //                         .a = VK_COMPONENT_SWIZZLE_IDENTITY },
    //         .subresourceRange = { .aspectMask =
    //                                 to_image_aspect_flags(p_config.aspect),
    //                               .baseMipLevel = 0,
    //                               .levelCount = 1,
    //                               .baseArrayLayer = 0,
    //                               .layerCount = 1 }
    //     };

    //     VkImageView image_view = nullptr;
    //     vk_check(vkCreateImageView(p_device, &view_info, nullptr, &image_view),
    //              "vkCreateImageView");

    //     return image_view;
    // }

    uint32_t select_memory_requirements(
      VkPhysicalDeviceMemoryProperties p_physical_memory_props,
      VkMemoryRequirements p_memory_requirements,
      memory_property p_property) {
        uint32_t memory_bits = p_memory_requirements.memoryTypeBits;
        VkMemoryPropertyFlags property_flag =
          static_cast<VkMemoryPropertyFlags>(p_property);

        for (uint32_t i = 0; i < p_physical_memory_props.memoryTypeCount; i++) {
            if ((memory_bits & (1 << i)) and
                (p_physical_memory_props.memoryTypes[i].propertyFlags &
                 property_flag) == property_flag) {
                return i;
            }
        }

        return -1;
    }

}; // end of v1 namespace
};