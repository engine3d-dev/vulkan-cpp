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

    // TODO: Eventually have a free-standing function to return the enumerated
    // physical devices std::span<vk::physical_device>
    // enumerate_physical_devices(const VkInstance& p_instance);

    std::vector<VkQueueFamilyProperties> enumerate_queue_family_properties(
      const VkPhysicalDevice& p_physical);

    //! @return a selected format with specific tiling and feature flags
    VkFormat select_compatible_formats(
      const VkPhysicalDevice& p_physical,
      const std::span<VkFormat>& p_format_selection,
      VkImageTiling p_tiling,
      VkFormatFeatureFlags p_feature_flag);

    //! @return the depth format which checks for compatible formats and is
    //! specific to the depth stencil attachment specified
    VkFormat select_depth_format(const VkPhysicalDevice& p_physical,
                                 const std::span<VkFormat>& p_format_selection);

    //! @return surface_enumeration which contains VkSurfaceCapabilities and
    //! VkSurfaceFormatKHR for the swapchain
    surface_enumeration enumerate_surface(const VkPhysicalDevice& p_physical,
                                          const VkSurfaceKHR& p_surface);

    //! @return image size the surface requires
    uint32_t surface_image_size(const VkSurfaceCapabilitiesKHR& p_capabilities);

    //! @return -1 if there are no flags available/compatible/valid
    uint32_t physical_memory_properties(const VkPhysicalDevice& p_physical,
                                        uint32_t p_type_filter,
                                        VkMemoryPropertyFlags p_property_flag);

    //! @return image view handler that is creating a gpu resource for 2d image
    //! @return this returns a created image view handler if the image is
    //! already created for you
    image create_image2d_view(
      const VkDevice& p_device,
      const swapchain_image_enumeration& p_enumerate_image);
    sampled_image create_depth_image2d(
      const VkDevice& p_device,
      const image_enumeration& p_enumerate_image,
      uint32_t p_memory_type_index);

    VkSampler create_sampler(const VkDevice& p_device,
                             const filter_range& p_range,
                             VkSamplerAddressMode p_address_mode);

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
      const image& p_image,
	  memory_property p_property = memory_property::device_local_bit);

    void free_image(const VkDevice& p_driver, sampled_image p_image);

    void free_image(const VkDevice& p_driver, image p_image);

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

    VkImageLayout to_image_layout(image_layout p_layout);

    VkVertexInputRate to_input_rate(input_rate p_input_rate);

    bool has_depth_specified(image_layout p_layout);

    VkCommandBufferUsageFlags to_command_usage_flag_bits(command_usage p_command_usage_flag);
	
	// TODO: Use this to do bitwise checks rather then; since this only does switch-case statement checks
	VkMemoryPropertyFlags to_memory_property_flags(memory_property p_flag);

	VkShaderStageFlagBits to_shader_stage(const shader_stage& p_stage);

  VkFormat to_format(const format& p_format);
};