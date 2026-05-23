module;

#include <vulkan/vulkan.h>
#include <span>
#include <memory>
#include <optional>

export module vk:index_buffer;

import :types;
import :utilities;
import :command_buffer;
import :buffer_streams32;
import :device;

export namespace vk {
    inline namespace v1 {

        /**
         * @brief Test implementation for index buffers
         *
         * This implementatino is meant to be used for an example.
         *
         * Though this can be used into your own code if you would like.
         */
        class index_buffer {
        public:
            index_buffer() = delete;
            index_buffer(std::shared_ptr<device> p_device,
                         std::span<const uint32_t> p_indices,
                         const buffer_parameters& p_params)
              : m_device(p_device) {

                m_index_buffer =
                  std::make_shared<buffer_stream32>(m_device, p_indices.size_bytes(), p_params);

                m_index_buffer->write(p_indices);
            }

            ~index_buffer() {
                destruct();
            }

            [[nodiscard]] bool alive() const { return *m_index_buffer; }

            operator VkBuffer() const { return *m_index_buffer; }

            operator VkBuffer() { return *m_index_buffer; }

            void destruct() { m_index_buffer->destruct(); }

        private:
            std::shared_ptr<device> m_device = nullptr;
            std::shared_ptr<buffer_stream32> m_index_buffer;
        };
    };
};