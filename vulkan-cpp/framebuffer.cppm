module;

#include <vulkan/vulkan.h>

export module vk:framebuffer;

export import :types;
export import :utilities;

export namespace vk {
    inline namespace v1 {
        /**
         * @name vk::framebuffer
         *
         * @param p_device is logical device that creates and manages the
         * destruction of vulkan framebuffer handles.
         * @param p_setting are specifications to configure the creation of vulkan
         * VkFramebuffer handles.
         */
        class framebuffer {
        public:
            framebuffer() = default;
            framebuffer(const VkDevice& p_device,
                        const framebuffer_params& p_setting) :  m_device(p_device) {
                
                VkFramebufferCreateInfo framebuffer_ci = {
                    .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
                    .pNext = nullptr,
                    .flags = 0,
                    .renderPass = p_setting.renderpass,
                    .attachmentCount = static_cast<uint32_t>(p_setting.views.size()),
                    .pAttachments = p_setting.views.data(),
                    .width = p_setting.extent.width,
                    .height = p_setting.extent.height,
                    .layers = 1
                };

                vk::vk_check(vkCreateFramebuffer(
                            m_device, &framebuffer_ci, nullptr, &m_framebuffer),
                            "vkCreateFramebuffer");
            }

            [[nodiscard]] bool alive() const { return m_framebuffer; }

            void destroy() {
                if (m_framebuffer != nullptr) {
                    vkDestroyFramebuffer(m_device, m_framebuffer, nullptr);
                }
            }

            operator VkFramebuffer() const { return m_framebuffer; }

            operator VkFramebuffer() { return m_framebuffer; }

        private:
            VkDevice m_device = nullptr;
            VkFramebuffer m_framebuffer = nullptr;
        };
    };
};