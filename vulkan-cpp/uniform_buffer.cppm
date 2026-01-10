module;

#include <vulkan/vulkan.h>
#include <span>
#include <array>

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
                        const uniform_params& p_uniform_info) : m_device(p_device), m_size_bytes(p_uniform_info.size_bytes) {
                
                uint32_t property_flags = memory_property::host_visible_bit |
                                  memory_property::host_coherent_bit;
                buffer_parameters uniform_info = {
                    .device_size = m_size_bytes,
                    .physical_memory_properties =
                    p_uniform_info.phsyical_memory_properties,
                    .property_flags = (memory_property)property_flags,
                    .usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                    .debug_name = p_uniform_info.debug_name.c_str(),
                    .vkSetDebugUtilsObjectNameEXT = p_uniform_info.vkSetDebugUtilsObjectNameEXT
                };
                m_uniform_handle = buffer_stream(m_device, uniform_info);
            }

            [[nodiscard]] bool alive() const { return m_uniform_handle; }

            void update(const void* p_data) {
                m_uniform_handle.write(p_data, m_size_bytes);
            }

            operator VkBuffer() const { return m_uniform_handle; }

            operator VkBuffer() { return m_uniform_handle; }

            [[nodiscard]] uint32_t size_bytes() const { return m_size_bytes; }

            void destroy() {
                m_uniform_handle.destroy();
            }

        private:
            uint32_t m_size_bytes = 0;
            VkDevice m_device = nullptr;
            buffer_stream m_uniform_handle{};
        };
    };
};