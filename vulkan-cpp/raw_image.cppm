module;

#include <span>

export module vk:raw_image;

import :types;

export namespace vk {
    inline namespace v1 {
        class raw_image {
        public:
            raw_image() = delete;

            raw_image(const VkDevice& p_device, image_extent p_extent, std::span<const uint8_t> p_bytes) : m_device(p_device) {
            }

            ~raw_image() {

            }

            void construct(image_extent p_extent, std::span<const uint8_t> p_bytes) {}

            void destruct() {
            }

        private:
            VkDevice m_device=nullptr;
        };
    };
};