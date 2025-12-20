module;

#include <vulkan/vulkan.h>
#include <span>

export module vk:buffer_streams;

export import :types;
export import :utilities;

export namespace vk {
    inline namespace v1 {
        /**
         * @brief Represents a VkBuffer handler for creating VkBuffer handle
         * 
         * Purpose for using VkBuffer handle to streaming bytes of data into the GPU memory
         * 
         */
        class buffer_stream {
        public:
            buffer_stream() = default;
            buffer_stream(const VkDevice& p_device,
                        const buffer_parameters& p_settings) : m_device(p_device) {
                 m_allocation_size = p_settings.device_size;

                VkBufferCreateInfo buffer_ci = {
                    .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
                    .pNext = nullptr,
                    .flags = 0,
                    .size = m_allocation_size, // size in bytes
                    .usage = p_settings.usage,
                    .sharingMode = p_settings.share_mode,
                };

                vk_check(vkCreateBuffer(p_device, &buffer_ci, nullptr, &m_handle),
                        "vkCreateBuffer");

                // 2. retrieving buffer memory requirements
                VkMemoryRequirements memory_requirements = {};
                vkGetBufferMemoryRequirements(p_device, m_handle, &memory_requirements);

                // 3. selects the required memory requirements for this specific buffer
                // allocations
                uint32_t memory_index =
                select_memory_requirements(p_settings.physical_memory_properties,
                                            memory_requirements,
                                            p_settings.property_flags);

                // 4. allocatring the necessary memory based on memory requirements for
                // the buffer handles
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
                    .objectHandle = (uint64_t)m_handle, // specify vulkan to what object handle this is
                    .pObjectName = p_settings.debug_name // specify what type of buffer this is
                };

                if(p_settings.vkSetDebugUtilsObjectNameEXT != nullptr) {
                    // vkSetDebugUtilsObjectNameEXT(m_device, &debug_info);
                    p_settings.vkSetDebugUtilsObjectNameEXT(m_device, &debug_info);
                }
        #endif
                vk_check(vkAllocateMemory(
                        p_device, &memory_alloc_info, nullptr, &m_device_memory),
                        "vkAllocateMemory");

                // 5. bind memory resource of this buffer handle
                vk_check(vkBindBufferMemory(p_device, m_handle, m_device_memory, 0),
                        "vkBindBufferMemory");
            }

            /**
             * @param span<T> writes some buffer data in the GPU's memory using
             * vkMapMemory/vkUnmapMemory
             */
            template<typename T>
            void write(std::span<const T> p_in_data) {
                VkDeviceSize buffer_size = p_in_data.size_bytes();
                void* mapped = nullptr;
                vk_check(vkMapMemory(
                        m_device, m_device_memory, 0, buffer_size, 0, &mapped),
                        "vkMapMemory");
                memcpy(mapped, p_in_data.data(), buffer_size);
                vkUnmapMemory(m_device, m_device_memory);
            }

            void write(const void* p_in_data, uint32_t p_size_bytes) {
                void* mapped = nullptr;
                vk_check(vkMapMemory(
                        m_device, m_device_memory, 0, p_size_bytes, 0, &mapped),
                        "vkMapMemory");
                memcpy(mapped, p_in_data, p_size_bytes);
                vkUnmapMemory(m_device, m_device_memory);
            }

            /**
             *
             * @brief This function automatically assumes the destination image
             * layout is going to be set to VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
             *
             * @param p_command is the current command buffer to perform and store
             * this operation into
             * @param p_image is the destination to copy data from the buffer to
             * @param p_extent is the size of the image that is being copied
             *
             * ```C++
             * 
             * buffer_streams texture_image(logical_device, ...);
             * 
             * texture_image.copy(temp_command_buffer, texture_image, texture_format, old_layout, new_layout);
             * ```
             * 
             */
            void copy_to_image(const VkCommandBuffer& p_command,
                            const VkImage& p_image,
                            image_extent p_extent) {
                 VkBufferImageCopy buffer_image_copy = {
                    .bufferOffset = 0,
                    .bufferRowLength = 0,
                    .bufferImageHeight = 0,
                    .imageSubresource = { .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                                        .mipLevel = 0,
                                        .baseArrayLayer = 0,
                                        .layerCount = 1 },
                    .imageOffset = { .x = 0, .y = 0, .z = 0 },
                    .imageExtent = { .width = p_extent.width, .height = p_extent.height, .depth = 1 }
                };

                vkCmdCopyBufferToImage(p_command,
                                    m_handle,
                                    p_image,
                                    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                    1,
                                    &buffer_image_copy);
            }

            /**
             * @param p_data is the bytes to write into the GPU's memory through the
             * Vulkan vkMapMemory/vkUnmapMemory API's.
             * 
             * ```C++
             * 
             * buffer_streams staging_buffer(logical_device, ...);
             * 
             * std::array<uint8_t, 4> white_color = { 0xFF, 0xFF, 0xFF, 0xFF };
             * staging_buffer.write(white_color);
             * ```
             * 
             */
            void write(std::span<const uint8_t> p_data) {
                void* mapped = nullptr;
                vk_check(
                    vkMapMemory(
                    m_device, m_device_memory, 0, p_data.size_bytes(), 0, &mapped),
                    "vkMapMemory");
                memcpy(mapped, p_data.data(), p_data.size_bytes());
                vkUnmapMemory(m_device, m_device_memory);
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
            uint32_t m_allocation_size = 0;
        };
    };
};