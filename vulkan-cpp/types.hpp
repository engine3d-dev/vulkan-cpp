#pragma once
#include <cstdint>
#include <vulkan/vulkan.h>
#include <string>
#include <span>
#include <array>
#include <glm/glm.hpp>

#include <vulkan/vulkan.hpp>

namespace vk {
    // Alias for VkFormat
    // using format = VkFormat;
    // enum format {
    //     rgb32_sfloat = VK_FORMAT_R32G32B32_SFLOAT,
    //     rg32_sfloat = VK_FORMAT_R32G32_SFLOAT
    // };

    enum format {
        rgb32_sfloat = VK_FORMAT_R32G32B32_SFLOAT,
        rg32_sfloat = VK_FORMAT_R32G32_SFLOAT
    };

    enum image_aspect_flags : uint8_t {
        color_bit,
        depth_bit,
        stencil_bit,
        metadata_bit,
        plane0_bit,
        plane1_bit,
        plane2_bit,
        none,
        memory_plane0_bit_ext,
        memory_plane1_bit_ext,
        memory_plane2_bit_ext,
        plane1_bit_khr,
        plane2_bit_khr,
        none_khr,
        bits_max_enum
    };

    /**
     * @brief message sevierity explicitly to max size of a byte
     */
    enum message : uint8_t { verbose = 0x1, warning = 0x2, error = 0x3 };

    enum debug : uint8_t { general = 0x1, validation = 0x2, performance = 0x3 };

    enum class api_version : uint32_t {
        vk_1_2,
        vk_1_3, // vulkan version 1.3
        // vk_1_2, // vulkan version 1.4
    };

    struct debug_message_utility {
        uint32_t severity;
        uint32_t message_type;
        uint32_t (*callback)(VkDebugUtilsMessageSeverityFlagBitsEXT,
                             VkDebugUtilsMessageTypeFlagsEXT,
                             const VkDebugUtilsMessengerCallbackDataEXT*,
                             void*) = nullptr;
    };

    struct application_configuration {
        std::string name = "";
        api_version version;
        std::span<const char*> validations;
        std::span<const char*> extensions;
    };

    struct swapchain_enumeration {
        uint32_t width;
        uint32_t height;
        uint32_t present_index = -1;

        VkFormat depth; // depth format
    };

    struct filter_range {
        VkFilter min;
        VkFilter max;
    };

    struct layer_properties {
        std::string name;
        uint32_t version;
        uint32_t impl_version;
        std::string description;
    };

    //! @brief vk::physical defines what kinds of physical device specification
    //! to use that is available based on your current physical hardware
    //! specifications.
    enum class physical {
        integrated,
        discrete,
        virtualized,
        cpu,
        max_enum,
        other
    };

    /**
     * @brief Enumeration represents configuration for the physical device
     *
     * Defines the configuration for the VkPhysicalDevice handler to be created
     * with
     */
    struct physical_enumeration {
        physical device_type;
    };

    struct physical_queue_enumeration {
        uint32_t queue_count = 0;
    };

    struct surface_enumeration {
        VkSurfaceCapabilitiesKHR capabilities;
        VkSurfaceFormatKHR format;
    };

    struct queue_enumeration {
        uint32_t family = -1;
        uint32_t index = -1;
    };

    struct queue_indices {
        uint32_t graphics = -1;
        uint32_t compute = -1;
        uint32_t transfer = -1;
    };

    struct device_enumeration {
        std::span<float> queue_priorities{};
        std::span<const char*>
          extensions{}; // Can add VK_KHR_SWAPCHAIN_EXTENSION_NAME to this
                        // extension
        uint8_t queue_family_index = 0;
    };

    // raw image handlers
    // struct image {
    //     VkImage image = nullptr;
    //     VkImageView view = nullptr;
    // };

    // sampler + raw image handlers
    // struct sampled_image {
    //     VkImage image = nullptr;
    //     VkImageView view = nullptr;
    //     VkSampler sampler = nullptr;
    //     VkDeviceMemory device_memory = nullptr;
    // };

    //! @brief enumeration if an image is provided
    // struct swapchain_image_enumeration {
    //     VkImage image = nullptr;
    //     VkFormat format;
    //     // VkImageAspectFlags aspect;
    //     image_aspect_flags aspect;
    //     uint32_t layer_count = 0;
    //     uint32_t mip_levels = 1;
    // };

    // Image enumeration for creating a brand new VkImage/VkImageView handlers
    // struct image_enumeration {
    //     uint32_t width = -1;
    //     uint32_t height = -1;
    //     VkFormat format;
    //     // VkImageAspectFlags aspect;
    //     image_aspect_flags aspect;
    //     uint32_t layer_count = 1;
    //     uint32_t mip_levels = 1;
    // };

    /**
     * @param renderpass vulkan requires framebuffers to know renderpasses up
     * front
     * @param views framebuffers provide the actual image views that will serve
     * as attachments to the renderpass handle
     * @param extent are for framebuffers to pass in frame image views to the
     * screen. Should be the size of the window viewport specified
     */
    struct framebuffer_settings {
        VkRenderPass renderpass = nullptr;
        std::span<VkImageView> views;
        VkExtent2D extent;
    };

    /**
     * @param Primary
     * submission: immediately to vulkan queue for executing on device
     * execution: vkQueueSubmit primarily submits primary command buffers
     *
     * @param Seconary
     * submission: Cannot be submitted directly to vulkan queue
     * execution: Indirect execution by being called from primary command
     * buffers using `vkCmdExecuteCommands`
     *
     * When executed within render-pass instance, they inherit
     * renderpass/subpass state from primary command buffer
     *
     * Can be allocated and recorded in parallel, which can leverage
     * devices that support multi-core processing
     *
     * Have independent lifetimes that can be managed independently of
     * primary command buffers, allowing for more flexible resource management
     */
    enum command_levels : uint8_t {
        primary = 0,
        secondary = 1,
        max_enum = 2,
    };

    /**
     * @param transient represents VK_COMMAND_POOL_CREATE_TRANSIENT_BIT
     *
     * Usage: specifies command buffers allocated from pool are short-lived,
     * meaning they will reset or be freed in a short time-frame. Flag may also
     * be used by implementation to control memory allocation behavior within
     * the pool
     *
     * @param reset represents VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT
     *
     * Usage: Allows command buffers allocated from the pool to be individually
     * reset to their initial state; either by calling vkResetCommandBuffer or
     * implicit reset when calling vkBeginCommandBuffer
     *
     * @param protected_bit represents VK_COMMAND_POOL_CREATE_PROTECTED_BIT
     *
     * Usage: Specifies command buffers allocated from pool are protected
     * command buffers; meaning the the memory allocated with the command pool
     * is protected
     *
     *
     */
    enum command_pool_flags : uint8_t {
        transient = 0x01,     // represents VK_COMMAND_POOL_CREATE_TRANSIENT_BIT
        reset = 0x02,         // represents
                              // VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT
        protected_bit = 0x04, // represents VK_COMMAND_POOL_CREATE_PROTECTED_BIT
        max_enum_bit =
          0x7F, // represents VK_COMMAND_POOL_CREATE_FLAG_BITS_MAX_ENUM
    };

    /**
     * @brief settings for specifying command buffers to construct
     *
     * @param Levels are for specifying the kind of command buffer being
     * constructed
     *
     * queue_index specifies the queue family associated with this
     * command buffer
     *
     * pool_flag in the context of the command buffers are used to
     * control memory allocation behavior within the command buffer pool to
     * allocate
     *
     * @param VK_COMMAND_POOL_CREATE_TRANSIENT_BIT Indicates command buffers
     * allocated from pool will be short-lived, meaning they'll reset or freed
     * quickly
     * @param VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT Allows individual
     * command buffers allocated from the pool to be reset to their initial
     * state using `vkResetCommandBuffer`
     * @param VK_COMMAND_POOL_CREATE_PROTECTED_BIT Designates command buffers
     * allocated from the pool as protective command buffers, implying they are
     * stored in protected memory and access to them is restricted to authorized
     * operations
     */
    struct command_enumeration {
        // command_enumeration(uint32_t p_queue_family,
        //                     const command_levels& p_levels,
        //                     const command_pool_flags& p_pool_flags)
        //   : levels(p_levels)
        //   , queue_index(p_queue_family)
        //   , flags(p_pool_flags) {}

        command_levels levels;
        uint32_t queue_index = -1;
        // VkCommandPoolCreateFlagBits pool_flag;
        command_pool_flags flags;
    };

    struct renderpass_attachments {
        std::span<VkClearColorValue> clear_values{};
        std::span<VkAttachmentDescription> attachments{};
        std::span<VkSubpassDescription> subpass_descriptions{};
        std::span<VkSubpassDependency> dependencies{};
    };

    enum subpass_contents : uint32_t {
        inline_bit = 0, // represents VK_SUBPASS_CONTENTS_INLINE
        secondary_command =
          1, // represents VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS
        inline_and_secondary_command_khr =
          1000451000, // represents
                      // VK_SUBPASS_CONTENTS_INLINE_AND_SECONDARY_COMMAND_BUFFERS_KHR
                      // and
                      // VK_SUBPASS_CONTENTS_INLINE_AND_SECONDARY_COMMAND_BUFFERS_EXT
        max_enum_content = 0x7F // represents VK_SUBPASS_CONTENTS_MAX_ENUM
    };

    enum class image_layout : uint8_t {
        undefined = 0,     // VK_IMAGE_LAYOUT_UNDEFINED
        general = 1,       // VK_IMAGE_LAYOUT_GENERAL
        color_optimal = 2, // VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
        depth_stencil_optimal =
          3, // VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
        depth_stencil_read_only_optimal =
          4, // VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_READ_ONLY_OPTIMAL
        present_src_khr = 5, // VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
        shader_read_only_optimal = 6
    };

    // enum class format : uint64_t {
    //     rgb32_sfloat, // Represent R32G32B32_SFLOAT
    //     rg32_sfloat,  // Represent R32G32_SFLOAT
    // };

    enum buffer : uint8_t {
        uniform = 0,
        storage = 1,
        combined_image_sampler = 2,
        sampled_only_image = 3
    };

    /**
     * @brief Refers to the input rate
     *
     * vertex - refers to most common rate. Indicates GPU to move next data
     * entry (next vertex data) into a buffer for every single vertex that's
     * processed.
     *       - Used for vertex attributes that change per vertex on the mesh
     *       - vertex attribute-only data
     *       - Per-object based specification in the next data entry
     *
     * instance - refers to data entry per-instance. Specifying to the GPU that
     *           the data entry in the buffer is to be after the instance of the
     * object itself.
     *         - Typically used for instanced rendering. Specfying next entry of
     * data to be after instanced drawn, could be shared. Therefore instance is
     * an option to choose from if vertex data is across as a per-instance
     * basis.
     *        - instance-based specification next data entry
     *
     */
    enum class input_rate : uint8_t {
        vertex,
        instance,
        max_enum,
    };

    //! @brief Equivalent to doing VkSampleCountFlagBits but simplified
    enum class sample_bit : uint8_t {
        count_1,
        count_2,
        count_4,
        count_8,
        count_16,
        count_32,
        count_64,
        max_enum
    };

    //! @brief Equivalent to VkAttachmentLoadOp
    enum class attachment_load : uint8_t {
        load = 0,  // LOAD_OP_LOAD
        clear,     // LOAD_OP_CLEAR
        dont_care, // lOAD_OP_DONT_CARE
        none_khr,  // LOAD_OP_NONE_KHR
        none_ext,  // LOAD_OP_NONE_EXT
        max_enum,  // LOAD_OP_MAX_ENUM
    };

    //! @brief Equivalent to VkAttachmentStoreOp
    enum class attachment_store : uint8_t {
        store = 0, // STORE_OP_STORE
        dont_care, // STORE_OP_CLEAR
        none_khr,  // STORE_OP_NONE
        none_qcom, // STORE_OP_NONE_EXT
        none_ext,  // STORE_OP_NONE_KHR
        max_enum,  // STORE_OP_MAX_ENUM
    };

    //! @brief Equivalent to VkPipelineBindPoint
    enum class pipeline_bind_point : uint64_t {
        graphics = VK_PIPELINE_BIND_POINT_GRAPHICS,                             // VK_PIPELINE_BIND_POINT_GRAPHICS
        compute = VK_PIPELINE_BIND_POINT_COMPUTE,                               // VK_PIPELINE_BIND_POINT_COMPUTE
        ray_tracing_khr = VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR,               // VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR
        subpass_shading_hauwei = VK_PIPELINE_BIND_POINT_SUBPASS_SHADING_HUAWEI, // VK_PIPELINE_BIND_POINT_SUBPASS_SHADING_HUAWEI
        ray_tracing_nv = VK_PIPELINE_BIND_POINT_RAY_TRACING_NV,                 // VK_PIPELINE_BIND_POINT_RAY_TRACING_NV
        max_enum = VK_PIPELINE_BIND_POINT_MAX_ENUM                              // VK_PIPELINE_BIND_POINT_MAX_ENUM
    };

    /**
     * @brief Specifies a specific attachment that a renderpass may operate
     * using
     *
     * Renderpasses can consist of multiple attachment that can be specified
     * under this one struct to represent both the VkAttachmentDescription and
     * VkAttachmentReference
     */
    struct attachment {
        VkFormat format;
        image_layout layout; // specify for VkAttachmentReference
        sample_bit samples;
        attachment_load load;
        attachment_store store;
        attachment_load stencil_load;
        attachment_store stencil_store;
        image_layout initial_layout;
        image_layout final_layout;
    };

    struct renderpass_begin_info {
        VkCommandBuffer current_command = nullptr;
        VkExtent2D extent;
        VkFramebuffer current_framebuffer = nullptr;
        std::array<float, 4> color;
        subpass_contents subpass;
    };

    //! @brief Bits that 1-to-1 map to VkCommandUsageFlags
    enum command_usage : uint8_t {
        one_time_submit =
          0x00000001, // Represents VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
        renderpass_continue_bit =
          0x00000002, // Represents
                      // VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT
        simulatneous_use_bit =
          0x00000004, // Represents VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT
        max_bit = 0x7F // Represents VK_COMMAND_BUFFER_USAGE_FLAG_BITS_MAX_ENUM
    };

    /**
     * @brief memory_property is a representation of vulkan's
     * VkMemoryPropertyFlags.
     *
     * @param device_local_bit
     *
     * Meaning: indicates memory allocated with this type is most efficient for
     * the GPU to access. \n
     *
     * Implications: The memory with this bit typically
     * resides on the GPU's VRAM. Accessing memory directly from GPU's since its
     * faster. \n
     *
     * Usage: For resources that are primarily accessed by the GPU in the
     * case of textures, vertex buffers, and framebuffers. If a memory type has
     * this bit associated with it, the heap memory will also have be set along
     * with VK_MEMORY_HEAP_DEVICE_LOCAL_BIT. \n
     *
     * @param host_visible_bit
     *
     * Meaning: Indicates memory alloated can be mapped to host's (CPU) address
     * space using the vkMapMemory API. \n
     *
     * Implications: ALlows CPU to directly
     * read from and write to memory. Crucial for transferring data between CPU
     * to GPU. \n
     *
     * Usage: Use-case is for staging buffers, where data initially
     * uploaded from CPU before being copied to device-local memory or for
     * resourcfes that need frequent CPU updates. \n
     *
     * @param host_coherent_bit
     *
     * Meaning: Indicates host cache managemnet commands
     * (vkFlushMappedMemoryRanges  and vkInvalidateMappedMemoryRanges) are not
     * needed. Writes made by host will automatically become visible to the
     * device, and writes made by device will automatically be visible to the
     * host. \n
     *
     * Implications: Simplifies memory synchronization between CPU and GPU.
     * Though can lead to slower CPU access if it means bypassing the CPU caches
     * or involving more complex cache coherence protocols. \n
     *
     * Usage: Used with 'VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT' for easy data
     * transfers, especially for frequent updated data where manual flushing
     * would be cumbersome. \n
     *
     *
     * @param host_cached_bit
     *
     * Meaning: Indicates memory allocated with this type is cached on the host
     * (CPU). \n
     *
     * Implications: Host memory accesses (read/writes) to this memory
     * type will go through CPU cache heirarchy. Significantly improves
     * performance where random access patterns. If not set on `HOST_VISIBLE`
     * memory, CPU accesses are often uncached and write-combined, meanming
     * writes should be sequential and reads should be avoided for good
     * performance. \n
     *
     * Usage: Does well for CPU-side reading of data written to GPU (screenshots
     * or feedback data) and for CPU-side writing of data to be accessed
     * randomly. Flag usually implies explicit cache management
     * (flushing/invalidating) is required if `HOST_COHERENT_BIT` is not also
     * set. \n
     *
     *
     */
    enum memory_property : uint8_t {
        device_local_bit = 0x00000001,
        host_visible_bit = 0x00000002,
        host_coherent_bit = 0x00000004,
        host_cached_bit = 0x00000008,
        lazily_allocated_bit = 0x00000010,
        device_protected_bit = 0x00000020,
        device_coherent_bit_amd = 0x20,
        device_uncached_bit_amd = 0x40,
        rdma_capable_bit_nv = 0x80,
        flag_bits_max_enum = 0x7f
    };

    enum class shader_stage {
        vertex = VK_SHADER_STAGE_VERTEX_BIT,
        fragment = VK_SHADER_STAGE_FRAGMENT_BIT,
        compute = VK_SHADER_STAGE_COMPUTE_BIT,
        geometry = VK_SHADER_STAGE_GEOMETRY_BIT,
        all_graphics = VK_SHADER_STAGE_ALL_GRAPHICS,
        all = VK_SHADER_STAGE_ALL,
        raygen_khr = VK_SHADER_STAGE_RAYGEN_BIT_KHR,
        raygen_nv = VK_SHADER_STAGE_RAYGEN_BIT_NV,
        any_hit_kht = VK_SHADER_STAGE_ANY_HIT_BIT_KHR,
        closest_hit_khr = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR,
        miss_bit_khr = VK_SHADER_STAGE_MISS_BIT_KHR,
        intersection_khr = VK_SHADER_STAGE_INTERSECTION_BIT_KHR,
        callable_bit_khr = VK_SHADER_STAGE_CALLABLE_BIT_KHR,
        task_bit_ext = VK_SHADER_STAGE_TASK_BIT_EXT,
        mesh_bit_ext = VK_SHADER_STAGE_MESH_BIT_EXT,
        supass_shading_huawei = VK_SHADER_STAGE_SUBPASS_SHADING_BIT_HUAWEI,
        undefined
    };

    enum class descriptor_layout_flag {
        update_after_bind_pool =
          VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT, // represents
                                                                      // VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT
        push_descriptor_khr =
          VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR, // represents
                                                                   // VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR
        descriptor_buffer_bit_ext =
          VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT, // represents
                                                                     // VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT
        embedded_immutable_samplers_bit_ext =
          VK_DESCRIPTOR_SET_LAYOUT_CREATE_EMBEDDED_IMMUTABLE_SAMPLERS_BIT_EXT, // represents VK_DESCRIPTOR_SET_LAYOUT_CREATE_EMBEDDED_IMMUTABLE_SAMPLERS_BIT_EXT
        indirect_bindable_bit_nv =
          VK_DESCRIPTOR_SET_LAYOUT_CREATE_INDIRECT_BINDABLE_BIT_NV, // represents
                                                                    // VK_DESCRIPTOR_SET_LAYOUT_CREATE_INDIRECT_BINDABLE_BIT_NV
        host_only_pool_bit_ext =
          VK_DESCRIPTOR_SET_LAYOUT_CREATE_HOST_ONLY_POOL_BIT_EXT, // represents
                                                                  // VK_DESCRIPTOR_SET_LAYOUT_CREATE_HOST_ONLY_POOL_BIT_EXT
        per_stage_bit_nv =
          VK_DESCRIPTOR_SET_LAYOUT_CREATE_PER_STAGE_BIT_NV, // represents
                                                            // VK_DESCRIPTOR_SET_LAYOUT_CREATE_PER_STAGE_BIT_NV
        update_after_bind_pool_bit_ext =
          VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT, // represents
                                                                          // VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT
        host_only_pool_bit_valve =
          VK_DESCRIPTOR_SET_LAYOUT_CREATE_HOST_ONLY_POOL_BIT_VALVE, // represents
                                                                    // VK_DESCRIPTOR_SET_LAYOUT_CREATE_HOST_ONLY_POOL_BIT_VALVE
        flag_bits_max_enum =
          VK_DESCRIPTOR_SET_LAYOUT_CREATE_FLAG_BITS_MAX_ENUM // represents
                                                             // VK_DESCRIPTOR_SET_LAYOUT_CREATE_FLAG_BITS_MAX_ENUM
    };

    //! @brief high-level specification for a shader source
    struct shader_source {
        std::string filename;
        shader_stage stage = shader_stage::undefined;
    };

    //! @brief Represent the vulkan shader module that will get utilized by
    //! VkPipeline
    struct shader_handle {
        VkShaderModule module = nullptr;
        shader_stage stage = shader_stage::undefined;
    };

    struct vertex_attribute_entry {
        uint32_t location;
        format format;
        uint32_t stride;
    };

    struct vertex_attribute {
        uint32_t binding;
        std::span<vertex_attribute_entry> entries;
        uint32_t stride;
        input_rate input_rate;
    };

    struct vertex_input {
        glm::vec3 position;
        glm::vec3 color;
        glm::vec3 normals;
        glm::vec2 uv;

        bool operator==(const vertex_input& other) const {
            return position == other.position and color == other.color and
                   uv == other.uv and normals == other.normals;
        }
    };

    //! @brief struct for copying from staging buffer to a destination
    struct buffer_copy_info {
        VkBuffer src;
        VkBuffer dst;
    };

    struct vertex_buffer_settings {
        VkPhysicalDeviceMemoryProperties phsyical_memory_properties;
        std::span<vertex_input> vertices;
        std::string debug_name;
        PFN_vkSetDebugUtilsObjectNameEXT vkSetDebugUtilsObjectNameEXT = nullptr;
    };

    struct index_buffer_settings {
        VkPhysicalDeviceMemoryProperties phsyical_memory_properties;
        std::span<uint32_t> indices;
        std::string debug_name;
        PFN_vkSetDebugUtilsObjectNameEXT vkSetDebugUtilsObjectNameEXT = nullptr;
    };

    struct uniform_buffer_info {
        // VkPhysicalDevice physical_handle=nullptr;
        VkPhysicalDeviceMemoryProperties phsyical_memory_properties;
        uint32_t size_bytes = 0;
        std::string debug_name;
        PFN_vkSetDebugUtilsObjectNameEXT vkSetDebugUtilsObjectNameEXT = nullptr;
    };

    struct descriptor_binding_point {
        uint32_t binding;
        shader_stage stage;
    };

    struct descriptor_entry {
        buffer type;
        descriptor_binding_point binding_point;
        uint32_t descriptor_count;
    };

    //! @brief vulkan-specification for creation of VkImage/VkImageView
    struct image_configuration {
        uint32_t width;
        uint32_t height;
        VkFormat format;
        uint32_t mip_levels = 1;
        uint32_t array_layers = 1;
        VkImageUsageFlags usage;
        memory_property property;
        VkPhysicalDevice physical_device = nullptr;
    };

    struct write_image {
        VkSampler sampler=nullptr;
        VkImageView view=nullptr;
        VkImageLayout image_layout=VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    };
    struct write_buffer {
        VkBuffer buffer=nullptr;
        uint32_t offset=0;
        uint32_t range=0;
    };

    struct write_buffer_descriptor {
        uint32_t dst_binding;
        std::span<const write_buffer> uniforms;
    };

    struct write_image_descriptor {
        uint32_t dst_binding;
        std::span<const write_image> sample_images;
    };

    struct image_extent {
        uint32_t width = 1;
        uint32_t height = 1;
    };

    struct image_configuration_information {
        image_extent extent;
        VkFormat format;
        memory_property property = memory_property::device_local_bit;
        image_aspect_flags aspect = image_aspect_flags::color_bit;
        VkImageUsageFlags usage;
        VkImageCreateFlags image_flags = 0;
        VkImageViewType view_type = VK_IMAGE_VIEW_TYPE_2D;
        uint32_t mip_levels = 1;
        uint32_t layer_count = 1;
        uint32_t array_layers = 1;
        // VkPhysicalDevice physical_device=nullptr;
        VkPhysicalDeviceMemoryProperties phsyical_memory_properties;
        filter_range range{ .min = VK_FILTER_LINEAR, .max = VK_FILTER_LINEAR };
        VkSamplerAddressMode addrses_mode_u = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        VkSamplerAddressMode addrses_mode_v = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        VkSamplerAddressMode addrses_mode_w = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    };

    struct buffer_parameters {
        VkDeviceSize device_size = 0;
        VkPhysicalDeviceMemoryProperties physical_memory_properties;
        memory_property property_flags;
        VkBufferUsageFlags usage;
        VkSharingMode share_mode = VK_SHARING_MODE_EXCLUSIVE;
        const char* debug_name="NA";
        PFN_vkSetDebugUtilsObjectNameEXT vkSetDebugUtilsObjectNameEXT = nullptr;
    };

    // Used by vk::copy(const VkCommandBuffer& p_current,  )
    struct copy_info {
        uint32_t width;
        uint32_t height;
        uint32_t array_layers = 1;
    };

    struct image_barrier_info {
        VkFormat format;
        VkImageLayout old_layout;
        VkImageLayout new_layout;
        uint32_t level_count = 1;
        VkImageAspectFlagBits aspect = VK_IMAGE_ASPECT_COLOR_BIT;
        uint32_t base_array_count = 0;
        uint32_t layer_count = 1;
    };

    struct write_info {
        uint64_t offset = 0;
        uint64_t size_bytes = 0;
    };

};