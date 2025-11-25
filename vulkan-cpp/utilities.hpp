#pragma once
#include <vulkan-cpp/types.hpp>
#include <source_location>
#include <vector>

namespace vk {

    void vk_check(
      const VkResult& p_result,
      const std::string& p_name,
      const std::source_location& p_location = std::source_location::current());

    /**
     * @brief converts vk::message to VkDebugUtilsMessageSeverityFlagsEXT
     *
     * Allows to set specific bit for setting up the message severity.
     *
     * Types of message severity for the specified debug callback
     * VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
     *
     * VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
     *
     * VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT
     */
    VkDebugUtilsMessageSeverityFlagsEXT to_debug_message_severity(
      uint32_t p_flag);

    VkDebugUtilsMessageTypeFlagsEXT to_message_type(uint32_t p_flag);

    uint32_t vk_api_version(const api_version& p_version);

    VkPhysicalDeviceType vk_physical_device_type(physical p_physical_type);

    VkPhysicalDevice enumerate_physical_devices(
      const VkInstance& p_instance,
      const physical& p_physical_device_type);

    std::vector<VkQueueFamilyProperties> enumerate_queue_family_properties(
      const VkPhysicalDevice& p_physical);

    //! @return a selected format with specific tiling and feature flags
    VkFormat select_compatible_formats(
      const VkPhysicalDevice& p_physical,
      std::span<const VkFormat> p_format_selection,
      VkImageTiling p_tiling,
      VkFormatFeatureFlags p_feature_flag);

    //! @return the depth format which checks for compatible formats and is
    //! specific to the depth stencil attachment specified
    VkFormat select_depth_format(const VkPhysicalDevice& p_physical,
                                 std::span<const VkFormat> p_format_selection);

    //! @return surface_enumeration which contains VkSurfaceCapabilities and
    //! VkSurfaceFormatKHR for the swapchain
    surface_enumeration enumerate_surface(const VkPhysicalDevice& p_physical,
                                          const VkSurfaceKHR& p_surface);

    //! @return image size the surface requires
    uint32_t surface_image_size(const VkSurfaceCapabilitiesKHR& p_capabilities);

    VkCommandBufferUsageFlags to_command_usage_flag_bits(
      command_usage p_command_usage_flag);

    VkImageAspectFlags to_image_aspect_flags(image_aspect_flags p_flag);

    //! @return -1 if there are no flags available/compatible/valid
    uint32_t physical_memory_properties(const VkPhysicalDevice& p_physical,
                                        uint32_t p_type_filter,
                                        VkMemoryPropertyFlags p_property_flag);

    VkSemaphore create_semaphore(const VkDevice& p_device);

    //! @brief Requests memory requirements from the physical hardware device
    //! @brief Using it to get the type_filter to return the
    //! image_memory_requirement
    /**
     * @param p_physical handle for physical hardware device
     * @param p_device handle for logical device represent as the virtual device
     * @param p_image the image context to retreive image memory requirements
     * from
     * @param p_property is memory_property for choosing a memory type for
     * a specific resource; in this case an image resource
     */
    uint32_t image_memory_requirements(
      const VkPhysicalDevice& p_physical,
      const VkDevice& p_device,
      const VkImage& p_image,
      memory_property p_property = memory_property::device_local_bit);

    VkCommandBufferLevel to_vk_command_buffer_level(
      const command_levels& p_level);

    VkCommandPoolCreateFlagBits to_command_buffer_pool_flags(
      command_pool_flags p_command_pool_flag);

    VkSubpassContents to_subpass_contents(subpass_contents p_content);

    VkPipelineBindPoint to_pipeline_bind_point(
      pipeline_bind_point p_bind_point);

    VkAttachmentLoadOp to_attachment_load(attachment_load p_attachment_type);

    VkAttachmentStoreOp to_attachment_store(attachment_store p_attachment_type);

    VkSampleCountFlagBits to_sample_count_bits(sample_bit p_sample_count_bit);

    // VkImageLayout to_image_layout(image_layout p_layout);

    VkVertexInputRate to_input_rate(input_rate p_input_rate);

    bool has_depth_specified(image_layout p_layout);

    bool has_stencil_attachment(VkFormat p_format);

    // TODO: Use this to do bitwise checks rather then; since this only does
    // switch-case statement checks
    VkMemoryPropertyFlags to_memory_property_flags(memory_property p_flag);

    VkShaderStageFlags to_shader_stage(const shader_stage& p_stage);

    VkFormat to_format(const format& p_format);

    //! @brief Copies from one buffer source into another buffer source with a
    //! specific size of bytes to be stored the buffer that is being copied to


    //! @return Returns the VkDescriptorType that represents a "handle" the
    //! shader resource is acecssing
    VkDescriptorType to_descriptor_type(const buffer& p_type);

    //! @brief passes a vulkan format
    //! @return the amount of bytes per vulkan format specification
    int bytes_per_texture_format(VkFormat p_format);

    uint32_t select_memory_requirements(
      VkPhysicalDeviceMemoryProperties p_physical_memory_props,
      VkMemoryRequirements p_memory_requirements,
      memory_property p_property);
};