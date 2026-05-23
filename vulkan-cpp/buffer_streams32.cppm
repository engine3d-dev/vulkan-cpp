module;

#include <vulkan/vulkan.h>
#include <span>
#include <array>
#include <cstring>
#include <bit>
#include <memory>

export module vk:buffer_streams32;

import :types;
import :utilities;
import :command_buffer;

export namespace vk {
    inline namespace v1 {
        /**
         * @brief buffer stream for streaming arbitrary buffers of 32-bytes
         */
        class buffer_stream32 {
        public:
            buffer_stream32() = default;
            buffer_stream32(std::shared_ptr<device> p_device,
                            uint64_t p_device_size,
                            const buffer_parameters& p_params)
              : m_device(p_device) {
                VkBufferCreateInfo buffer_ci = {
                    .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
                    .pNext = nullptr,
                    .flags = 0,
                    .size = p_device_size, // size in bytes
                    .usage = static_cast<VkBufferUsageFlags>(p_params.usage),
                    .sharingMode = p_params.share_mode,
                };

                vk_check(
                  vkCreateBuffer(*m_device, &buffer_ci, nullptr, &m_handle),
                  "vkCreateBuffer");

                // 2. retrieving buffer memory requirements
                VkMemoryRequirements memory_requirements = {};
                vkGetBufferMemoryRequirements(
                  *m_device, m_handle, &memory_requirements);
                uint32_t mapped_memory_requirements =
                  memory_requirements.memoryTypeBits & p_params.memory_mask;
                uint32_t memory_index =
                  std::countr_zero(mapped_memory_requirements);

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
                    p_params.vkSetDebugUtilsObjectNameEXT(*m_device,
                                                          &debug_info);
                }
#endif
                vk_check(
                  vkAllocateMemory(
                    *m_device, &memory_alloc_info, nullptr, &m_device_memory),
                  "vkAllocateMemory");

                // 5. bind memory resource of this buffer handle
                vk_check(
                  vkBindBufferMemory(*m_device, m_handle, m_device_memory, 0),
                  "vkBindBufferMemory");
            }

            ~buffer_stream32() {
                destruct();
            }

            /**
             * @brief write arbitrary buffer of 32-bytes to GPU-memory
             */
            void write(std::span<const uint32_t> p_data) {
                void* mapped = nullptr;
                vk_check(vkMapMemory(*m_device,
                                     m_device_memory,
                                     0,
                                     p_data.size_bytes(),
                                     0,
                                     &mapped),
                         "vkMapMemory");
                memcpy(mapped, p_data.data(), p_data.size_bytes());
                vkUnmapMemory(*m_device, m_device_memory);
            }

            void copy_to_image(const VkCommandBuffer& p_command,
                               const VkImage& p_image,
                               image_extent p_extent) {
                VkBufferImageCopy buffer_image_copy = {
                    .bufferOffset = 0,
                    .bufferRowLength = 0,
                    .bufferImageHeight = 0,
                    .imageSubresource = { .aspectMask =
                                            VK_IMAGE_ASPECT_COLOR_BIT,
                                          .mipLevel = 0,
                                          .baseArrayLayer = 0,
                                          .layerCount = 1 },
                    .imageOffset = { .x = 0, .y = 0, .z = 0 },
                    .imageExtent = { .width = p_extent.width,
                                     .height = p_extent.height,
                                     .depth = 1 }
                };

                vkCmdCopyBufferToImage(p_command,
                                       m_handle,
                                       p_image,
                                       VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                       1,
                                       &buffer_image_copy);
            }

            void destruct() {
                if (m_handle != nullptr) {
                    vkDestroyBuffer(*m_device, m_handle, nullptr);
                }

                if (m_device_memory != nullptr) {
                    vkFreeMemory(*m_device, m_device_memory, nullptr);
                }
            }

            operator VkBuffer() { return m_handle; }

            operator VkBuffer() const { return m_handle; }

        private:
            std::shared_ptr<device> m_device = nullptr;
            VkDeviceMemory m_device_memory = nullptr;
            VkBuffer m_handle = nullptr;
        };
    };
};