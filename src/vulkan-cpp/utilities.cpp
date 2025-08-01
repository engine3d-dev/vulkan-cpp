#include <vulkan-cpp/utilities.hpp>
#include <vulkan/vulkan.h>
#include <filesystem>
#include <print>
#include <vulkan-cpp/command_buffer.hpp>

namespace vk {

    void vk_check(const VkResult& p_result,
                  const std::string& p_name,
                  const std::source_location& p_source) {
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

    VkDebugUtilsMessageSeverityFlagsEXT to_debug_message_severity(
      uint32_t p_flag) {
        VkDebugUtilsMessageSeverityFlagsEXT flag;

        if (p_flag & vk::message::verbose) {
            flag |= VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT;
        }
        else if (p_flag & vk::message::warning) {
            flag |= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
        }
        else if (p_flag & vk::message::error) {
            flag |= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        }

        return flag;
    }

    VkDebugUtilsMessageTypeFlagsEXT to_message_type(uint32_t p_flag) {
        VkDebugUtilsMessageTypeFlagsEXT flag;

        if (p_flag & vk::debug::general) {
            flag |= VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT;
        }
        else if (p_flag & vk::debug::validation) {
            flag |= VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
        }
        else if (p_flag & vk::debug::performance) {
            flag |= VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        }

        return flag;
    }

    uint32_t vk_api_version(const api_version& p_version) {
        switch (p_version) {
            case api_version::vk_1_2:
                return VK_API_VERSION_1_2;
            case api_version::vk_1_3:
                return VK_API_VERSION_1_3;
        }
    }

    VkPhysicalDeviceType vk_physical_device_type(physical p_physical_type) {
        switch (p_physical_type){
        case physical::integrated:
            return VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU;
        case physical::discrete:
            return VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
        case physical::virtualized:
            return VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU;
        case physical::cpu:
            return VK_PHYSICAL_DEVICE_TYPE_CPU;
        case physical::max_enum:
            return VK_PHYSICAL_DEVICE_TYPE_MAX_ENUM;
        case physical::other:
            return VK_PHYSICAL_DEVICE_TYPE_OTHER;
        }

        throw std::runtime_error("Invalid physical device!");
    }

    struct physical_device_handler {
        VkPhysicalDevice handler=nullptr;
    };

    VkPhysicalDevice enumerate_physical_devices(const VkInstance& p_instance, const physical& p_physical_device_type) {
        uint32_t device_count=0;
        vkEnumeratePhysicalDevices(p_instance, &device_count, nullptr);

        if(device_count == 0) {
            throw std::runtime_error("device_count is zero!");
        }

        // std::vector<vk::physical_device> hardware_physical_devices(device_count);

        std::vector<VkPhysicalDevice> physical_devices(device_count);
        vkEnumeratePhysicalDevices(p_instance, &device_count, physical_devices.data());
        VkPhysicalDevice physical_device=nullptr;

        for(const auto& device : physical_devices) {
            VkPhysicalDeviceProperties device_properties;
            vkGetPhysicalDeviceProperties(device, &device_properties);

            if (device_properties.deviceType == vk_physical_device_type(p_physical_device_type)) {
                physical_device = device;
            }
        }
        return physical_device;
    }

    std::vector<VkQueueFamilyProperties> enumerate_queue_family_properties(const VkPhysicalDevice& p_physical) {
        uint32_t queue_family_count = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(p_physical, &queue_family_count, nullptr);
        std::vector<VkQueueFamilyProperties> queue_family_properties(queue_family_count);

        vkGetPhysicalDeviceQueueFamilyProperties(p_physical,&queue_family_count,queue_family_properties.data());

        return queue_family_properties;
    }

    VkFormat select_compatible_formats(const VkPhysicalDevice& p_physical, const std::span<VkFormat>& p_format_selection, VkImageTiling p_tiling, VkFormatFeatureFlags p_feature_flag) {
        VkFormat format = VK_FORMAT_UNDEFINED;

        for (size_t i = 0; i < p_format_selection.size(); i++) {
            VkFormat current_format = p_format_selection[i];
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

    VkFormat select_depth_format(const VkPhysicalDevice& p_physical, const std::span<VkFormat>& p_format_selection) {

        VkFormat format = select_compatible_formats(
          p_physical,
          p_format_selection,
          VK_IMAGE_TILING_OPTIMAL,
          VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
        return format;
    }

    uint32_t physical_memory_properties(const VkPhysicalDevice& p_physical, uint32_t p_type_filter, VkMemoryPropertyFlags p_property_flag) {
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

    VkMemoryPropertyFlags to_memory_property_flags(memory_property p_flag) {
        VkMemoryPropertyFlags flags = 0;
        if (p_flag & memory_property::device_local_bit) {
            flags |= VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        }
        if (p_flag & memory_property::host_visible_bit) {
            flags |= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
        }
        if (p_flag & memory_property::host_coherent_bit) {
            flags |= VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
        }
        if (p_flag & memory_property::host_cached_bit) {
            flags |= VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
        }
        if (p_flag & memory_property::lazily_allocated_bit) {
            flags |= VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT;
        }
        if (p_flag & memory_property::device_protected_bit) {
            flags |= VK_MEMORY_PROPERTY_PROTECTED_BIT;
        }
        if (p_flag & memory_property::device_coherent_bit_amd) {
            flags |= VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD;
        }
        if (p_flag & memory_property::device_uncached_bit_amd) {
            flags |= VK_MEMORY_PROPERTY_DEVICE_UNCACHED_BIT_AMD;
        }
        if (p_flag & memory_property::rdma_capable_bit_nv) {
            flags |= VK_MEMORY_PROPERTY_RDMA_CAPABLE_BIT_NV;
        }



        // switch (p_flag) {
        // case memory_property::device_local_bit:
        //     return VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        // case memory_property::host_visible_bit:
        //     return VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
        // case memory_property::host_coherent_bit:
        //     return VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
        // case memory_property::host_cached_bit:
        //     return VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
        // case memory_property::lazily_allocated_bit:
        //     return VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT;
        // case memory_property::device_protected_bit:
        //     return VK_MEMORY_PROPERTY_PROTECTED_BIT;
        // case memory_property::device_coherent_bit_amd:
        //     return VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD;
        // case memory_property::device_uncached_bit_amd:
        //     return VK_MEMORY_PROPERTY_DEVICE_UNCACHED_BIT_AMD;
        // case memory_property::rdma_capable_bit_nv:
        //     return VK_MEMORY_PROPERTY_RDMA_CAPABLE_BIT_NV;
        // case memory_property::flag_bits_max_enum:
        //     return VK_MEMORY_PROPERTY_FLAG_BITS_MAX_ENUM;
        // }

        // throw std::runtime_error("Invalid memory property flag set!");
        return flags;
    }

    surface_enumeration enumerate_surface(const VkPhysicalDevice& p_physical, const VkSurfaceKHR& p_surface) {
        surface_enumeration enumerate_surface_properties{};
        vk_check(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
                   p_physical,
                   p_surface,
                   &enumerate_surface_properties.capabilities),
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

    uint32_t surface_image_size(const VkSurfaceCapabilitiesKHR& p_capabilities) {
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

    VkCommandBufferUsageFlags to_command_usage_flag_bits(command_usage p_command_usage_flag) {
        switch(p_command_usage_flag) {
        case command_usage::one_time_submit:
            return VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        case command_usage::renderpass_continue_bit:
            return VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
        case command_usage::simulatneous_use_bit:
            return VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
        case command_usage::max_bit:
            return VK_COMMAND_BUFFER_USAGE_FLAG_BITS_MAX_ENUM;
        }

        throw std::runtime_error("Invalid command_usage specified");
    }

    VkImageAspectFlags to_image_aspect_flags(image_aspect_flags p_flag) {
        switch (p_flag){
        case image_aspect_flags::color_bit:
            return VK_IMAGE_ASPECT_COLOR_BIT;
        case image_aspect_flags::depth_bit:
            return VK_IMAGE_ASPECT_DEPTH_BIT;
        case image_aspect_flags::stencil_bit:
            return VK_IMAGE_ASPECT_STENCIL_BIT;
        case image_aspect_flags::metadata_bit:
            return VK_IMAGE_ASPECT_METADATA_BIT;
        case image_aspect_flags::plane0_bit:
            return VK_IMAGE_ASPECT_PLANE_0_BIT;
        case image_aspect_flags::plane1_bit:
            return VK_IMAGE_ASPECT_PLANE_1_BIT;
        case image_aspect_flags::plane2_bit:
            return VK_IMAGE_ASPECT_PLANE_2_BIT;
        case image_aspect_flags::none:
            return VK_IMAGE_ASPECT_NONE;
        case image_aspect_flags::memory_plane0_bit_ext:
            return VK_IMAGE_ASPECT_MEMORY_PLANE_0_BIT_EXT;
        case image_aspect_flags::memory_plane1_bit_ext:
            return VK_IMAGE_ASPECT_MEMORY_PLANE_1_BIT_EXT;
        case image_aspect_flags::memory_plane2_bit_ext:
            return VK_IMAGE_ASPECT_MEMORY_PLANE_2_BIT_EXT;
        case image_aspect_flags::plane1_bit_khr:
            return VK_IMAGE_ASPECT_PLANE_1_BIT_KHR;
        case image_aspect_flags::plane2_bit_khr:
            return VK_IMAGE_ASPECT_PLANE_2_BIT_KHR;
        case image_aspect_flags::none_khr:
            return VK_IMAGE_ASPECT_NONE_KHR;
        case image_aspect_flags::bits_max_enum:
            return VK_IMAGE_ASPECT_FLAG_BITS_MAX_ENUM;
        }

        throw std::runtime_error("Invalid image aspect flags specified!!!");
    }

    VkSampler create_sampler(const VkDevice& p_device, const filter_range& p_range, VkSamplerAddressMode p_address_mode) {
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

        VkSampler sampler=nullptr;
        VkResult res =
          vkCreateSampler(p_device, &sampler_info, nullptr, &sampler);
        vk_check(res, "vkCreateSampler");
        return sampler;
    }

    image create_image2d_view(const VkDevice& p_device, const swapchain_image_enumeration& p_enumerate_image) {
        VkImageViewCreateInfo image_view_ci = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .image = p_enumerate_image.image,
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = p_enumerate_image.format,
            .components = { .r = VK_COMPONENT_SWIZZLE_IDENTITY,
                            .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                            .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                            .a = VK_COMPONENT_SWIZZLE_IDENTITY },
            .subresourceRange = { .aspectMask = to_image_aspect_flags(p_enumerate_image.aspect),
                                  .baseMipLevel = 0,
                                  .levelCount = p_enumerate_image.mip_levels,
                                  .baseArrayLayer = 0,
                                  .layerCount = p_enumerate_image.layer_count },
        };

        image image2d{};
        image2d.image = p_enumerate_image.image;

        VkImageView image_view;
        vk_check(
          vkCreateImageView(p_device, &image_view_ci, nullptr, &image2d.view),
          "vkCreateImageView");
        return image2d;
    }

    sampled_image create_depth_image2d(const VkDevice& p_device, const image_enumeration& p_enumerate_image, uint32_t p_memory_type_index) {

        VkImageUsageFlags usage =
              VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

        VkImageCreateInfo image_ci = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .imageType = VK_IMAGE_TYPE_2D,
            .format = p_enumerate_image.format,
            .extent = { .width = p_enumerate_image.width, .height = p_enumerate_image.height, .depth = 1 },
            .mipLevels = p_enumerate_image.mip_levels,
            .arrayLayers = 1,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .tiling = VK_IMAGE_TILING_OPTIMAL,
            .usage = usage,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .queueFamilyIndexCount = 0,
            .pQueueFamilyIndices = nullptr,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
        };

        sampled_image depth_image{};

        vk_check(vkCreateImage(p_device, &image_ci, nullptr, &depth_image.image),
                 "vkCreateImage");

        // 2. get buffer memory requirements
        VkMemoryRequirements memory_requirements;
        vkGetImageMemoryRequirements(p_device, depth_image.image, &memory_requirements);

        // 3. get memory type index
        // uint32_t memory_type_index = driver.select_memory_type(
        //   memory_requirements.memoryTypeBits, property_flags);

        // 4. Allocate info
        VkMemoryAllocateInfo memory_alloc_info = {
            .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
            .pNext = nullptr,
            .allocationSize = memory_requirements.size,
            .memoryTypeIndex = p_memory_type_index
        };

        vk_check(vkAllocateMemory(
                   p_device, &memory_alloc_info, nullptr, &depth_image.device_memory),
                 "vkAllocateMemory");

        // 5. bind image memory
        vk_check(vkBindImageMemory(p_device, depth_image.image, depth_image.device_memory, 0),
                 "vkBindImageMemory");
        
        // Needs to create VkImageView after VkImage
        // because VkImageView expects a VkImage to be binded to a singl VkDeviceMemory beforehand
        VkImageViewCreateInfo image_view_ci = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .image = depth_image.image,
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = p_enumerate_image.format,
            .components = { .r = VK_COMPONENT_SWIZZLE_IDENTITY,
                            .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                            .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                            .a = VK_COMPONENT_SWIZZLE_IDENTITY },
            .subresourceRange = { .aspectMask = p_enumerate_image.aspect,
                                  .baseMipLevel = 0,
                                  .levelCount = p_enumerate_image.mip_levels,
                                  .baseArrayLayer = 0,
                                  .layerCount = p_enumerate_image.layer_count },
        };

        vk_check(vkCreateImageView(p_device, &image_view_ci, nullptr, &depth_image.view),"vkCreateImage");

        return depth_image;
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

    void free_image(const VkDevice& p_driver, sampled_image p_image) {
        if (p_image.view != nullptr) {
            vkDestroyImageView(p_driver, p_image.view, nullptr);
        }

        if (p_image.image != nullptr) {
            vkDestroyImage(p_driver, p_image.image, nullptr);
        }
        if (p_image.sampler != nullptr) {
            vkDestroySampler(p_driver, p_image.sampler, nullptr);
        }

        if (p_image.device_memory != nullptr) {
            vkFreeMemory(p_driver, p_image.device_memory, nullptr);
        }
    }

    void free_image(const VkDevice& p_driver, image p_image) {
        if (p_image.view != nullptr) {
            vkDestroyImageView(p_driver, p_image.view, nullptr);
        }

        if (p_image.image != nullptr) {
            vkDestroyImage(p_driver, p_image.image, nullptr);
        }
    }

    uint32_t image_memory_requirements(const VkPhysicalDevice& p_physical, const VkDevice& p_device, const image& p_image, memory_property p_property) {
        VkMemoryRequirements memory_requirements;
        vkGetImageMemoryRequirements(p_device, p_image.image, &memory_requirements);

        uint32_t type_filter = memory_requirements.memoryTypeBits;
        VkMemoryPropertyFlags property_flag = to_memory_property_flags(p_property);

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

    uint32_t select_memory_requirements(const VkPhysicalDevice& p_physical, VkMemoryRequirements p_memory_requirements, memory_property p_property) {
        // VkMemoryRequirements memory_requirements;
        // vkGetImageMemoryRequirements(p_device, p_image.image, &memory_requirements);

        uint32_t type_filter = p_memory_requirements.memoryTypeBits;
        VkMemoryPropertyFlags property_flag = to_memory_property_flags(p_property);

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

    uint32_t buffer_memory_requirement(const VkPhysicalDevice& p_physical, const VkDevice& p_device, const buffer_handle& p_buffer, memory_property p_property) {
        VkMemoryRequirements memory_requirements;
        // vkGetImageMemoryRequirements(p_device, p_buffer.handle, &memory_requirements);
        vkGetBufferMemoryRequirements(p_device, p_buffer.handle, &memory_requirements);

        uint32_t type_filter = memory_requirements.memoryTypeBits;
        VkMemoryPropertyFlags property_flag = to_memory_property_flags(p_property);

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



    VkCommandBufferLevel to_vk_command_buffer_level(
      const command_levels& p_level) {
        switch (p_level) {
            case command_levels::primary:
                return VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            case command_levels::secondary:
                return VK_COMMAND_BUFFER_LEVEL_SECONDARY;
            case command_levels::max_enum:
                return VK_COMMAND_BUFFER_LEVEL_MAX_ENUM;
        }

        throw std::runtime_error("Invalid command buffer levels");
    }

    VkCommandPoolCreateFlagBits to_command_buffer_pool_flags(command_pool_flags p_command_pool_flag) {
        switch (p_command_pool_flag){
        case command_pool_flags::protected_bit:
            return VK_COMMAND_POOL_CREATE_PROTECTED_BIT;
        case command_pool_flags::reset:
            return VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        case command_pool_flags::transient:
            return VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
        case command_pool_flags::max_enum_bit:
            return VK_COMMAND_POOL_CREATE_FLAG_BITS_MAX_ENUM;
        }

        return (VkCommandPoolCreateFlagBits)0;
    }

    VkSubpassContents to_subpass_contents(subpass_contents p_content) {
        switch (p_content){
        case subpass_contents::inline_bit:
            return VK_SUBPASS_CONTENTS_INLINE;
        case subpass_contents::secondary_command:
            return VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS;
        case subpass_contents::inline_and_secondary_command_khr:
            return VK_SUBPASS_CONTENTS_INLINE_AND_SECONDARY_COMMAND_BUFFERS_KHR;
        case subpass_contents::max_enum_content:
            return VK_SUBPASS_CONTENTS_MAX_ENUM;
        default:
            break;
        }
    }

    VkPipelineBindPoint to_pipeline_bind_point(pipeline_bind_point p_bind_point) {
        switch (p_bind_point) {
            case pipeline_bind_point::graphics:
                return VK_PIPELINE_BIND_POINT_GRAPHICS;
            case pipeline_bind_point::compute:
                return VK_PIPELINE_BIND_POINT_COMPUTE;
            case pipeline_bind_point::ray_tracing_khr:
                return VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR;
            case pipeline_bind_point::subpass_shading_hauwei:
                return VK_PIPELINE_BIND_POINT_SUBPASS_SHADING_HUAWEI;
            case pipeline_bind_point::ray_tracing_nv:
                return VK_PIPELINE_BIND_POINT_RAY_TRACING_NV;
            case pipeline_bind_point::max_enum:
                return VK_PIPELINE_BIND_POINT_MAX_ENUM;
            default:
                break;
        }
    }

    VkAttachmentLoadOp to_attachment_load(attachment_load p_attachment_type) {
        switch (p_attachment_type) {
            case attachment_load::load:
                return VK_ATTACHMENT_LOAD_OP_LOAD;
            case attachment_load::clear:
                return VK_ATTACHMENT_LOAD_OP_CLEAR;
            case attachment_load::dont_care:
                return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            case attachment_load::none_khr:
                return VK_ATTACHMENT_LOAD_OP_NONE_KHR;
            case attachment_load::none_ext:
                return VK_ATTACHMENT_LOAD_OP_NONE_EXT;
            case attachment_load::max_enum:
                return VK_ATTACHMENT_LOAD_OP_MAX_ENUM;
        }
    }

    VkAttachmentStoreOp to_attachment_store(attachment_store p_attachment_type) {
        switch (p_attachment_type) {
            case attachment_store::store:
                return VK_ATTACHMENT_STORE_OP_STORE;
            case attachment_store::dont_care:
                return VK_ATTACHMENT_STORE_OP_DONT_CARE;
            case attachment_store::none_khr:
                return VK_ATTACHMENT_STORE_OP_NONE_KHR;
            case attachment_store::none_qcom:
                return VK_ATTACHMENT_STORE_OP_NONE_QCOM;
            case attachment_store::none_ext:
                return VK_ATTACHMENT_STORE_OP_NONE_EXT;
            case attachment_store::max_enum:
                return VK_ATTACHMENT_STORE_OP_MAX_ENUM;
            default:
                break;
        }
    }

    VkSampleCountFlagBits to_sample_count_bits(sample_bit p_sample_count_bit) {
        switch (p_sample_count_bit) {
            case sample_bit::count_1:
                return VK_SAMPLE_COUNT_1_BIT;
            case sample_bit::count_2:
                return VK_SAMPLE_COUNT_2_BIT;
            case sample_bit::count_4:
                return VK_SAMPLE_COUNT_4_BIT;
            case sample_bit::count_8:
                return VK_SAMPLE_COUNT_8_BIT;
            case sample_bit::count_16:
                return VK_SAMPLE_COUNT_16_BIT;
            case sample_bit::count_32:
                return VK_SAMPLE_COUNT_32_BIT;
            case sample_bit::count_64:
                return VK_SAMPLE_COUNT_64_BIT;
            case sample_bit::max_enum:
                return VK_SAMPLE_COUNT_FLAG_BITS_MAX_ENUM;
        }
    }

    VkImageLayout to_image_layout(image_layout p_layout) {
        switch (p_layout) {
            case image_layout::undefined:
                return VK_IMAGE_LAYOUT_UNDEFINED;
            case image_layout::general:
                return VK_IMAGE_LAYOUT_GENERAL;
            case image_layout::color_optimal:
                return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            case image_layout::depth_stencil_optimal:
                return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            case image_layout::depth_stencil_read_only_optimal:
                return VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL;
            case image_layout::present_src_khr:
                return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        }
    }

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
        if(p_layout == image_layout::depth_stencil_optimal) {
            return true;
        }

        if(p_layout == image_layout::depth_stencil_read_only_optimal) {
            return true;
        }
        return false;
    }

    VkShaderStageFlagBits to_shader_stage(const shader_stage& p_stage) {
        switch (p_stage) {
            case shader_stage::vertex:
                return VK_SHADER_STAGE_VERTEX_BIT;
            case shader_stage::fragment:
                return VK_SHADER_STAGE_FRAGMENT_BIT;
            default:
                return (VkShaderStageFlagBits)0;
        }
    }

    VkFormat to_format(const format& p_format) {
        switch (p_format) {
            case format::rg32_sfloat:
                return VK_FORMAT_R32G32_SFLOAT;
            case format::rgb32_sfloat:
                return VK_FORMAT_R32G32B32A32_SFLOAT;
            default:
                return VK_FORMAT_UNDEFINED;
        }
    }

    buffer_handle create_buffer(const VkDevice& p_device, const buffer_configuration& p_info) {
        buffer_handle handler = {};

        handler.allocation_size = p_info.device_size;

        VkBufferCreateInfo buffer_ci = {
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .size = handler.allocation_size, // size in bytes
            .usage = p_info.usage,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE
        };

        vk_check(vkCreateBuffer(p_device, &buffer_ci, nullptr, &handler.handle),"vkCreateBuffer");

        // 2. retrieving buffer memory requirements
        VkMemoryRequirements memory_requirements = {};
        vkGetBufferMemoryRequirements(p_device, handler.handle, &memory_requirements);

        // 3. selecting memory requirements from current physical device
        // memory_property property;
        uint32_t memory_type_index = select_memory_requirements(p_info.physical, memory_requirements, p_info.property_flags);

        // 4. allocatring the necessary memory based on memory requirements for the buffer handles
        VkMemoryAllocateInfo memory_alloc_info = {
            .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
            .allocationSize = memory_requirements.size,
            .memoryTypeIndex = memory_type_index
        };

        vk_check(vkAllocateMemory(p_device, &memory_alloc_info, nullptr, &handler.device_memory),"vkAllocateMemory");

        // 5. bind memory resource of this buffer handle
        vk_check(vkBindBufferMemory(p_device, handler.handle, handler.device_memory, 0),"vkBindBufferMemory");

        return handler;
    }

    void write(const VkDevice& p_device, const buffer_handle& p_buffer, const std::span<vertex_input>& p_in_buffer) {
        // does equivalent to doing sizeof(p_in_buffer[0]) * p_in_buffer.size();
        VkDeviceSize buffer_size =
          p_in_buffer
            .size_bytes();
        void* mapped = nullptr;
        vk_check(vkMapMemory(
                   p_device, p_buffer.device_memory, 0, buffer_size, 0, &mapped),
                 "vkMapMemory");
        memcpy(mapped, p_in_buffer.data(), buffer_size);
        vkUnmapMemory(p_device, p_buffer.device_memory);
    }

    VkCommandPool create_single_command_pool(const VkDevice& p_device, uint32_t p_queue_family_index) {
        // uint32_t graphics_queue_index = physical.read_queue_family_indices().graphics;
        VkCommandPoolCreateInfo pool_ci = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .queueFamilyIndex = p_queue_family_index
        };

        VkCommandPool command_pool = nullptr;
        vk_check(vkCreateCommandPool(p_device, &pool_ci, nullptr, &command_pool),
                 "vkCreateCommandPool");

        return command_pool;
    }

    void copy(const VkDevice& p_device, const buffer_copy_info& p_info, size_t p_size_of_bytes) {

        // 1. Retrieve the first queue
        // TODO: Use vk::device_queue for this
        VkQueue temp_graphics_queue = nullptr;
        uint32_t queue_family_index = 0;
        uint32_t queue_index = 0;
        vkGetDeviceQueue(p_device, queue_family_index, queue_index, &temp_graphics_queue);

        // command_buffer_info
        command_enumeration enumerate_command_info = {
            .levels = command_levels::primary,
            .queue_index = 0,
        };
        command_buffer copy_command_buffer(p_device, enumerate_command_info);

        copy_command_buffer.begin(command_usage::one_time_submit);
        VkBufferCopy copy_region{};
        copy_region.size = (VkDeviceSize)p_size_of_bytes;
        vkCmdCopyBuffer(
          copy_command_buffer, p_info.src.handle, p_info.dst.handle, 1, &copy_region);
        copy_command_buffer.end();
        VkCommandBuffer temp = copy_command_buffer;
        VkSubmitInfo submit_info{};
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &temp;

        vkQueueSubmit(temp_graphics_queue, 1, &submit_info, nullptr);
        vkQueueWaitIdle(temp_graphics_queue);

        // vkFreeCommandBuffers(, command_pool, 1, &copy_cmd_buffer);
        // vkDestroyCommandPool(driver, command_pool, nullptr);
        copy_command_buffer.destroy();
    }

    void free_buffer(const VkDevice& p_driver, buffer_handle& p_buffer) {
        if (p_buffer.handle != nullptr) {
            vkDestroyBuffer(p_driver, p_buffer.handle, nullptr);
        }

        if (p_buffer.device_memory != nullptr) {
            vkFreeMemory(p_driver, p_buffer.device_memory, nullptr);
        }
    }


}