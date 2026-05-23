module;

#include <vulkan/vulkan.h>
#include <span>
#include <array>
#include <cassert>
#include <memory>
#include <optional>

export module vk:uniform_buffer;

import :types;
import :utilities;
import :command_buffer;
import :buffer_streams;
import :device;

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
            uniform_buffer(std::shared_ptr<device> p_device,
                           uint64_t p_size_bytes,
                           const buffer_parameters& p_uniform_params)
              : m_device(p_device)
              , m_size_bytes(p_size_bytes) {
                m_uniform_handle =
                  std::make_optional<buffer_stream>(m_device, p_size_bytes, p_uniform_params);
            }

            ~uniform_buffer() {
                destruct();
            }

            [[nodiscard]] bool alive() const { return *m_uniform_handle; }

            template<typename T>
            void transfer(std::span<const T> p_uniform_data) {
                m_uniform_handle->transfer<T>(p_uniform_data);
            }

            void transfer(std::span<const uint8_t> p_uniforms) {
                m_uniform_handle->transfer(p_uniforms);
            }

            [[nodiscard]] uint64_t size_bytes() const { return m_size_bytes; }

            operator VkBuffer() const { return *m_uniform_handle; }

            operator VkBuffer() { return *m_uniform_handle; }

            void destruct() { m_uniform_handle->destruct(); }

        private:
            uint64_t m_size_bytes;
            std::shared_ptr<device> m_device = nullptr;
            std::optional<buffer_stream> m_uniform_handle;
        };
    };
};