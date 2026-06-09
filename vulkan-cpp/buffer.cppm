module;

#include <vulkan/vulkan.h>
#include <span>
#include <vector>
#include <bit>
#include <limits>

export module vk:buffer;

export import :types;
export import :utilities;

export namespace vk {
    inline namespace v6 {
        /**
         * @brief Represents a VkBuffer handler for creating VkBuffer handle
         *
         * Purpose for using VkBuffer handle to streaming bytes of data into the
         * GPU memory
         *
         */
        class buffer {
        public:
            buffer() = default;

            /**
             * @brief constructs a buffer to write streams of data to GPU
             * memory
             *
             * @param p_device is the logical device to construct the buffer
             * handles
             * @param p_device_size is size in bytes of the buffer to be created
             * @param p_params are additional parameters for the buffer
             * handles
             */
            buffer(const VkDevice& p_device,
                   uint64_t p_device_size,
                   const buffer_parameters& p_params)
              : m_device(p_device) {
                construct(p_device_size, p_params);
            }

            ~buffer() = default;

            void construct(uint64_t p_device_size,
                           const buffer_parameters& p_params) {
                VkBufferCreateInfo buffer_ci = {
                    .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
                    .pNext = nullptr,
                    .flags = 0,
                    .size = p_device_size, // size in bytes
                    .usage = static_cast<VkBufferUsageFlags>(p_params.usage),
                    .sharingMode = p_params.share_mode,
                };

                vk_check(
                  vkCreateBuffer(m_device, &buffer_ci, nullptr, &m_handle),
                  "vkCreateBuffer");

                // retrieving buffer memory requirements
                VkMemoryRequirements memory_requirements = {};
                vkGetBufferMemoryRequirements(
                  m_device, m_handle, &memory_requirements);
                uint32_t mapped_memory_requirements =
                  memory_requirements.memoryTypeBits & p_params.memory_mask;
                uint32_t memory_index = std::numeric_limits<uint32_t>::max();
                if (mapped_memory_requirements != 0) {
                    memory_index = std::countr_zero(mapped_memory_requirements);
                }
                else {
                    memory_index =
                      std::countr_zero(memory_requirements.memoryTypeBits);
                }

                VkMemoryAllocateInfo memory_alloc_info = {
                    .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
                    .allocationSize = memory_requirements.size,
                    .memoryTypeIndex = memory_index
                };

#if _DEBUG
                // 1. Define the structure
                VkDebugUtilsObjectNameInfoEXT debug_info = {
                    .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
                    .pNext = nullptr,
                    .objectType = VK_OBJECT_TYPE_BUFFER,
                    .objectHandle = (uint64_t)
                      m_handle, // specify vulkan to what object handle this is
                    .pObjectName =
                      p_params.debug_name // specify what type of buffer this is
                };

                if (p_params.vkSetDebugUtilsObjectNameEXT != nullptr) {
                    // vkSetDebugUtilsObjectNameEXT(m_device, &debug_info);
                    p_params.vkSetDebugUtilsObjectNameEXT(m_device,
                                                          &debug_info);
                }
#endif
                vk_check(
                  vkAllocateMemory(
                    m_device, &memory_alloc_info, nullptr, &m_device_memory),
                  "vkAllocateMemory");

                // 5. bind memory resource of this buffer handle
                vk_check(
                  vkBindBufferMemory(m_device, m_handle, m_device_memory, 0),
                  "vkBindBufferMemory");
            }

            /**
             * @brief writes an arbitrary amount of uniforms of type T
             *
             * Performs runtime assertion checks if the bytes of type T are
             * valid before mapping the uniforms for GPU to access
             *
             * Example Usage:
             *
             * ```C++
             * vk::uniform_buffer test_ubo(...);
             * std::array<global_ubo, 2> ubo = { ... };
             *
             * test_ubo.transfer<global_ubo>(ubo);
             * ```
             */
            template<typename T>
            void transfer(std::span<const T> p_in_data, uint32_t p_offset = 0) {
                void* mapped = nullptr;
                vk_check(vkMapMemory(m_device,
                                     m_device_memory,
                                     p_offset,
                                     p_in_data.size_bytes(),
                                     0,
                                     &mapped),
                         "vkMapMemory");
                memcpy(mapped, p_in_data.data(), p_in_data.size_bytes());
                vkUnmapMemory(m_device, m_device_memory);
            }

            /**
             * @brief writing uniforms that are represented into bytes
             * @param p_data are the bytes to allow GPU to access
             * Example Usage:
             *
             * ```C++
             * buffers staging_buffer(logical_device, ...);
             *
             * std::array<uint8_t, 4> white_color = { 0xFF, 0xFF, 0xFF, 0xFF };
             * staging_buffer.transfer(white_color);
             * ```
             *
             */
            void transfer(std::span<const uint8_t> p_data,
                          uint32_t p_offset = 0) {
                void* mapped = nullptr;
                vk_check(vkMapMemory(m_device,
                                     m_device_memory,
                                     p_offset,
                                     p_data.size_bytes(),
                                     0,
                                     &mapped),
                         "vkMapMemory");
                memcpy(mapped, p_data.data(), p_data.size_bytes());
                vkUnmapMemory(m_device, m_device_memory);
            }

            /**
             * @brief Transfers CPU-accessible data from the staging buffer to
             * the GPU resource image.
             *
             * This API requires a command buffer to perform memory copying from
             * a staging buffer to an image, performing any necessary format
             * conversion or sizzling.
             *
             * Where swizzling is the transformation of pixel data to a more
             * linear format in the given staging buffer to a more optimized
             * non-linear layout using the vk::image_tiling_optimal
             *
             * @param p_command is the current command to record to perform this
             * operation with
             * @param p_image is the CPU-accessible data to be transferred to
             * @param p_copies regions defining copy operations. Each entry
             * specify:
             * - .offset: starting byte for the staging buffer.
             * - .row_length: Length of pixels in the buffer (0 tightly packed).
             * - .aspect_mask: Part of the image target (color/depth/stencil).
             * - .mip_level: Mipmap levels to update for LOD (level-of-detail).
             * - .base_array_layer: Starting layer (face index for cubemaps).
             * - .layer_count: Count of layers to copy into.
             * - .image_offset: {x, y, z} starting coordinate within the image.
             * - .image_extent: {w, h, d} size of the region to be updated.
             *
             * @brief Requirements when performing this operation
             *
             * - p_image: Must be in vk::image_layout::transfer_dst_optimal
             * before command is in record state. Use pipeline barriers to
             * transition before performing this operation.
             * - source buffer: must be specified with
             * vk::buffer_usage::transfer_src_bit,
             * - dst image: must have vk::image_usage::transfer_dst_bit
             * specified.
             * - alignment: .offset must be a multiple of texel size (e.g., 4
             * for RGBA). Other hardware require to be a multiple of 4.
             * - (.image_offset + .image_extent): Must not exceed actual
             * dimensions of the image handle.
             * - Command buffer must be from a queue family which supports
             * transfer and graphics operation.
             *
             * @brief Pixels of data stored in the staging buffer before
             * transferring to GPU-accessible memory.
             *
             * [ bytes 0 .......................................... bytes N ]
             * |--- Region[0] Data --- | (Unused) | --- Region[1] --- | (etc.)
             * \______________________/ \__________________________/  |
             * | Copy Instruction 0                    | Copy Instruction 1
             * | (p_copies[0])                         | (p_copies[1])
             * V                                       V
             * @brief Destination for the GPU-resource Image
             *
             * Each vk::buffer_image_copy defines the specific parts of an image
             * that is being transferred for being accessible to the GPU
             *
             * Each designated copy region is designated to different parts of a
             * given image. Which wou
             *
             * +----------------------------------------------------+
             * | Mip Level 0:                                       |
             * | +--------------+                                   |
             * | | [ Region 0 ] | <-- Maps from p_copies[0].offset  |
             * | +--------------+                                   |
             * |                                                    |
             * | Mip Level 1:                                       |
             * | +-------------                                     |
             * | | [ Region 1]  | <-- Maps from p_copies[1].offset  |
             * | +--------------+                                   |
             * +----------------------------------------------------+
             *
             *
             * Example Usage:
             *
             * ```C++
             *
             * vk::buffer staging_buffer(logical_device, {...});
             *
             * std::array<vk::buffer_image_copy, 1> copy_regions = {
             *      vk::buffer_image_copy{
             *          .image_extent = { width, height, .depth=1 },
             *      },
             * };
             * staging_buffer.copy_to_image(command, image, copy_regions);
             *
             * ```
             */
            void copy_to_image(const VkCommandBuffer& p_command,
                               const VkImage& p_image,
                               std::span<const buffer_image_copy> p_copies) {
                std::vector<VkBufferImageCopy> image_copies(p_copies.size());

                for (uint32_t i = 0; i < image_copies.size(); i++) {
                    const buffer_image_copy image_copy = p_copies[i];
                    image_copies[i] = {
                        .bufferOffset = image_copy.offset,
                        .bufferRowLength = image_copy.row_length,
                        .bufferImageHeight = image_copy.image_height,
                        .imageSubresource = {
                            .aspectMask = static_cast<VkImageAspectFlags>(image_copy.aspect_mask),
                            .mipLevel = image_copy.mip_level,
                            .baseArrayLayer = image_copy.base_array_layer,
                            .layerCount = image_copy.layer_count,
                        },
                        .imageOffset = { static_cast<int32_t>(image_copy.image_offset.width), static_cast<int32_t>(image_copy.image_offset.height), static_cast<int32_t>(image_copy.image_offset.depth), },
                        .imageExtent = { image_copy.image_extent.width, image_copy.image_extent.height, image_copy.image_extent.depth, },
                    };
                }

                vkCmdCopyBufferToImage(
                  p_command,
                  m_handle,
                  p_image,
                  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                  static_cast<uint32_t>(image_copies.size()),
                  image_copies.data());
            }

            void destruct() {
                if (m_handle != nullptr) {
                    vkDestroyBuffer(m_device, m_handle, nullptr);
                }

                if (m_device_memory != nullptr) {
                    vkFreeMemory(m_device, m_device_memory, nullptr);
                }
            }

            operator VkBuffer() const { return m_handle; }

            operator VkBuffer() { return m_handle; }

        private:
            VkDevice m_device = nullptr;
            VkBuffer m_handle;
            VkDeviceMemory m_device_memory;
        };
    };
};