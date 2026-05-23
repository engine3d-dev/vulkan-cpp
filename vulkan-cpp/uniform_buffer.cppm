module;

#include <vulkan/vulkan.h>
#include <span>
#include <array>
#include <cassert>

export module vk:uniform_buffer;

export import :types;
export import :utilities;
export import :command_buffer;
export import :buffer;

export namespace vk {
    inline namespace v6 {
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
                           const buffer_parameters& p_uniform_params)
              : m_device(p_device)
              , m_size_bytes(p_size_bytes) {
                m_uniform_handle =
                  buffer(m_device, p_size_bytes, p_uniform_params);
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
            buffer m_uniform_handle{};
        };
    };
};