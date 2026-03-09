module;

#include <vulkan/vulkan.h>
#include <span>
#include <array>
#include <cassert>

export module vk:uniform_buffer;

export import :types;
export import :utilities;
export import :command_buffer;
export import :buffer_streams;

export namespace vk {
    inline namespace v1 {
        /**
         * @brief represents a vulkan uniform buffer
         *
         * Maps uniforms and gpu-specific resources
         */
        class uniform_buffer {
        public:
            uniform_buffer() = default;
            uniform_buffer(const VkDevice& p_device,
                           uint64_t p_size_bytes,
                           const uniform_params& p_uniform_info)
              : m_device(p_device)
              , m_size_bytes(p_size_bytes) {
                buffer_parameters uniform_info = {
                    .physical_memory_properties =
                      p_uniform_info.phsyical_memory_properties,
                    .property_flags = static_cast<memory_property>(
                      memory_property::host_visible_bit |
                      memory_property::host_coherent_bit),
                    .usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                    .debug_name = p_uniform_info.debug_name.c_str(),
                    .vkSetDebugUtilsObjectNameEXT =
                      p_uniform_info.vkSetDebugUtilsObjectNameEXT
                };
                m_uniform_handle =
                  buffer_stream(m_device, p_size_bytes, uniform_info);
            }

            [[nodiscard]] bool alive() const { return m_uniform_handle; }

            template<typename T>
            void transfer(std::span<const T> p_uniform_data) {
                m_uniform_handle.transfer<T>(p_uniform_data);
            }

            void transfer(std::span<const uint8_t> p_uniforms) {
                m_uniform_handle.transfer(p_uniforms);
            }

            [[nodiscard]] uint64_t size_bytes() const { return m_size_bytes; }

            operator VkBuffer() const { return m_uniform_handle; }

            operator VkBuffer() { return m_uniform_handle; }

            void destroy() { m_uniform_handle.destroy(); }

        private:
            uint64_t m_size_bytes;
            VkDevice m_device = nullptr;
            buffer_stream m_uniform_handle{};
        };
    };
};