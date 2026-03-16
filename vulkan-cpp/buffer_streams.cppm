module;

#include <vulkan/vulkan.h>
#include <span>
#include <vector>

export module vk:buffer_streams;

export import :types;
export import :utilities;

export namespace vk {
    inline namespace v1 {
        /**
         * @brief Represents a VkBuffer handler for creating VkBuffer handle
         *
         * Purpose for using VkBuffer handle to streaming bytes of data into the
         * GPU memory
         *
         */
        class buffer_stream {
        public:
            buffer_stream() = default;

            /**
             * @brief constructs a buffer_stream to write streams of data to GPU
             * memory
             *
             * @param p_device is the logical device to construct the buffer
             * handles
             * @param p_device_size is size in bytes of the buffer to be created
             * @param p_settings are additional parameters for the buffer
             * handles
             */
            buffer_stream(const VkDevice& p_device,
                          uint64_t p_device_size,
                          const buffer_parameters& p_settings)
              : m_device(p_device) {

                VkBufferCreateInfo buffer_ci = {
                    .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
                    .pNext = nullptr,
                    .flags = 0,
                    .size = p_device_size, // size in bytes
                    .usage = p_settings.usage,
                    .sharingMode = p_settings.share_mode,
                };

                vk_check(
                  vkCreateBuffer(p_device, &buffer_ci, nullptr, &m_handle),
                  "vkCreateBuffer");

                // 2. retrieving buffer memory requirements
                VkMemoryRequirements memory_requirements = {};
                vkGetBufferMemoryRequirements(
                  p_device, m_handle, &memory_requirements);

                // 3. selects the required memory requirements for this specific
                // buffer allocations
                uint32_t memory_index = select_memory_requirements(
                  p_settings.physical_memory_properties,
                  memory_requirements,
                  p_settings.property_flags);

                // 4. allocatring the necessary memory based on memory
                // requirements for the buffer handles
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
                      p_settings
                        .debug_name // specify what type of buffer this is
                };

                if (p_settings.vkSetDebugUtilsObjectNameEXT != nullptr) {
                    // vkSetDebugUtilsObjectNameEXT(m_device, &debug_info);
                    p_settings.vkSetDebugUtilsObjectNameEXT(m_device,
                                                            &debug_info);
                }
#endif
                vk_check(
                  vkAllocateMemory(
                    p_device, &memory_alloc_info, nullptr, &m_device_memory),
                  "vkAllocateMemory");

                // 5. bind memory resource of this buffer handle
                vk_check(
                  vkBindBufferMemory(p_device, m_handle, m_device_memory, 0),
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
             * buffer_streams staging_buffer(logical_device, ...);
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
             *
             * @brief This function automatically assumes the destination image
             * layout is going to be set to VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
             *
             * @param p_command is the current command buffer to perform and
             * store this operation into
             * @param p_image is the destination to copy data from the buffer to
             * @param p_copies are the amount of buffer to image copy data to
             * transfer to the GPU
             *
             * ```C++
             *
             * buffer_streams staging_buffer(logical_device, ...);
             *
             * std::array<vk::buffer_image_copy, 1> region_copies = {
             *      vk::buffer_image_copy region1{
             *          .image_extent = { .width = width, .height = height,
             * .depth = 1, },
             *      }
             * };
             * staging_buffer.copy_to_image(command, sample_image,
             * region_copies);
             *
             * ```
             *
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

                // VkBufferImageCopy buffer_image_copy = {
                //     .bufferOffset = 0,
                //     .bufferRowLength = 0,
                //     .bufferImageHeight = 0,
                //     .imageSubresource = { .aspectMask =
                //                             VK_IMAGE_ASPECT_COLOR_BIT,
                //                           .mipLevel = 0,
                //                           .baseArrayLayer = 0,
                //                           .layerCount = 1 },
                //     .imageOffset = { .x = 0, .y = 0, .z = 0 },
                //     .imageExtent = { .width = p_extent.width,
                //                      .height = p_extent.height,
                //                      .depth = 1 }
                // };

                // vkCmdCopyBufferToImage(p_command,
                //                        m_handle,
                //                        p_image,
                //                        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                //                        1,
                //                        &buffer_image_copy);
                vkCmdCopyBufferToImage(
                  p_command,
                  m_handle,
                  p_image,
                  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                  static_cast<uint32_t>(image_copies.size()),
                  image_copies.data());
            }

            void destroy() {
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