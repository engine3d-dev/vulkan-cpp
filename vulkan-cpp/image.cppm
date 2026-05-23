module;

#include <string_view>
#include <span>

export module vk:image;

import :types;

export namespace vk {
    inline namespace v6 {

        /**
         * @brief interface for the purpose of acting as an interface to
         * different implementations for support variety of approaches in
         * loading images.
         */
        class image {
        public:
            virtual ~image() = default;

            /**
             * @brief Perform loading operations specific to the implementation
             * of this interface
             */
            bool load(std::string_view p_path, texture_params p_params) {
                return image_load(p_path, p_params);
            }

            /**
             * @brief Read you the bytes read from the specific image loading
             * implementations
             */
            [[nodiscard]] std::span<const uint8_t> read() const {
                return image_read();
            }

            /**
             * @return the extent that contains properties about the image
             * (dimensions, depth, etc)
             */
            [[nodiscard]] image_extent extent() const { return image_extent(); }

        protected:
            virtual bool image_load(std::string_view, texture_params) = 0;

            virtual std::span<const uint8_t> image_read() const = 0;

            virtual image_extent image_extent() const = 0;
        };
    };
};