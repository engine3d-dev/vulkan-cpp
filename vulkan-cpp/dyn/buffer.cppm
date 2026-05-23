module;

#include <vulkan/vulkan.h>
#include <vector>
#include <span>
#include <bit>
#include <memory>

export module vk:buffer_device_address;

import :types;
import :utilities;
import :device;

export namespace vk::dyn {

    class buffer {
    public:
        buffer() = delete;

        buffer(std::shared_ptr<device> p_device,
               uint64_t p_device_size,
               const buffer_parameters& p_params)
          : m_device(p_device) {
            construct(p_device_size, p_params);
        }

        ~buffer() {
            destruct();
        }

        // Can be invoked to perform invalidation on this buffer
        void construct(uint64_t p_device_size,
                       const buffer_parameters& p_params) {

            m_size_bytes = p_device_size;

            // Constructs this dyn::buffer
            VkBufferCreateInfo buffer_ci = {
                .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .size = p_device_size, // size in bytes
                .usage = static_cast<VkBufferUsageFlags>(p_params.usage),
                .sharingMode = p_params.share_mode,
            };

            vk_check(vkCreateBuffer(*m_device, &buffer_ci, nullptr, &m_handle),
                     "vkCreateBuffer");

            // Required to ensure the memory allocated is correspondent to the
            // shader buffer device address bit
            VkMemoryAllocateFlagsInfo allocate_flags_info = {
                .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO,
                .pNext = nullptr,
                .flags =
                  static_cast<VkMemoryAllocateFlags>(p_params.allocate_flags),
            };
            // retrieving buffer memory requirements
            VkMemoryRequirements memory_requirements = {};
            vkGetBufferMemoryRequirements(
              *m_device, m_handle, &memory_requirements);
            uint32_t mapped_memory_requirements =
              memory_requirements.memoryTypeBits & p_params.memory_mask;
            uint32_t memory_index =
              std::countr_zero(mapped_memory_requirements);

            // Required to be set for buffer device addresses
            VkMemoryAllocateInfo memory_alloc_info = {
                .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
                .pNext = &allocate_flags_info,
                .allocationSize = memory_requirements.size,
                .memoryTypeIndex = memory_index,
            };

            // Allocating for this buffer handle
            vk_check(vkAllocateMemory(
                       *m_device, &memory_alloc_info, nullptr, &m_device_memory),
                     "vkAllocateMemory");
            vk_check(vkBindBufferMemory(*m_device, m_handle, m_device_memory, 0),
                     "vkBindBufferMemory");
        }

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
                    .imageOffset = {
                        static_cast<int32_t>(image_copy.image_offset.width),
                        static_cast<int32_t>(image_copy.image_offset.height),
                        static_cast<int32_t>(image_copy.image_offset.depth),
                    },
                    .imageExtent = {
                        image_copy.image_extent.width,
                        image_copy.image_extent.height,
                        image_copy.image_extent.depth,
                    },
                };
            }

            vkCmdCopyBufferToImage(p_command,
                                   m_handle,
                                   p_image,
                                   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                   static_cast<uint32_t>(image_copies.size()),
                                   image_copies.data());
        }

        //! @brief Destroys this object
        void destruct() {
            if (m_handle != nullptr) {
                vkDestroyBuffer(*m_device, m_handle, nullptr);
            }

            if (m_device_memory != nullptr) {
                vkFreeMemory(*m_device, m_device_memory, nullptr);
            }
        }

        template<typename T>
        void transfer(std::span<const T> p_data, uint32_t p_offset = 0) {
            void* mapped = nullptr;
            vk_check(vkMapMemory(*m_device,
                                 m_device_memory,
                                 p_offset,
                                 p_data.size_bytes(),
                                 0,
                                 &mapped),
                     "vkMapMemory");
            memcpy(mapped, p_data.data(), p_data.size_bytes());
            vkUnmapMemory(*m_device, m_device_memory);
        }

        //! @brief Allows to retrieve the address to this particular buffer
        //! handle
        [[nodiscard("Cannot discard get_device_address()")]] const uint64_t
        get_device_address() const {
            VkBufferDeviceAddressInfo buffer_address_info = {
                .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
                .buffer = m_handle,
            };

            return static_cast<uint64_t>(
              vkGetBufferDeviceAddress(*m_device, &buffer_address_info));
        }

        [[nodiscard("Cannot discard device_memory()")]] VkDeviceMemory
        device_memory() const {
            return m_device_memory;
        }

        [[nodiscard("cannot discard size_bytes()")]] uint32_t size_bytes()
          const {
            return m_size_bytes;
        }

        operator VkBuffer() { return m_handle; }

        operator VkBuffer() const { return m_handle; }

    private:
        uint32_t m_size_bytes = 0;
        std::shared_ptr<device> m_device = nullptr;
        VkBuffer m_handle = nullptr;
        VkDeviceMemory m_device_memory;
    };
};