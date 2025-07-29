#pragma once
#include <cstdint>
#include <vulkan/vulkan.h>
#include <string>
#include <span>
#include <array>

namespace vk {

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
        uint32_t queue_family_index = 0;
    };

    // raw image handlers
    struct image {
        VkImage image = nullptr;
        VkImageView view = nullptr;
    };

    // sampler + raw image handlers
    struct sampled_image {
        VkImage image = nullptr;
        VkImageView view = nullptr;
        VkSampler sampler = nullptr;
        VkDeviceMemory device_memory = nullptr;
    };

    //! @brief enumeration if an image is provided
    struct swapchain_image_enumeration {
        VkImage image = nullptr;
        VkFormat format;
        VkImageAspectFlags aspect;
        uint32_t layer_count = 0;
        uint32_t mip_levels = 1;
    };

    // Image enumeration for creating a brand new VkImage/VkImageView handlers
    struct image_enumeration {
        uint32_t width = -1;
        uint32_t height = -1;
        VkFormat format;
        VkImageAspectFlags aspect;
        uint32_t layer_count = 1;
        uint32_t mip_levels = 1;
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
        command_enumeration(uint32_t p_queue_family,
                            const command_levels& p_levels,
                            const command_pool_flags& p_pool_flags)
          : levels(p_levels)
          , queue_index(p_queue_family)
          , flags(p_pool_flags) {}

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
    };

    enum class format : uint64_t {
        rgb32_sfloat, // Represent R32G32B32_SFLOAT
        rg32_sfloat,  // Represent R32G32_SFLOAT
    };

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
    enum class pipeline_bind_point : uint8_t {
        graphics = 0,           // VK_PIPELINE_BIND_POINT_GRAPHICS
        compute = 1,            // VK_PIPELINE_BIND_POINT_COMPUTE
        ray_tracing_khr,        // VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR
        subpass_shading_hauwei, // VK_PIPELINE_BIND_POINT_SUBPASS_SHADING_HUAWEI
        ray_tracing_nv,         // VK_PIPELINE_BIND_POINT_RAY_TRACING_NV
        max_enum                // VK_PIPELINE_BIND_POINT_MAX_ENUM
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

    enum command_usage : uint8_t {
        one_time_submit = 0x00000001,
        renderpass_continue_bit = 0x00000002,
        simulatneous_use_bit = 0x00000004,
        max_bit = 0x7F // represents max_enum
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
        device_local_bit = 0x01,
        host_visible_bit = 0x2,
        host_coherent_bit = 0x4,
        host_cached_bit = 0x8,
        lazily_allocated_bit = 0x10,
        device_protected_bit,
        device_coherent_bit_amd = 0x20,
        device_uncached_bit_amd = 0x40,
        rdma_capable_bit_nv = 0x80,
        flag_bits_max_enum = 0x7f
    };

};