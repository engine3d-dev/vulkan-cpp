#pragma once
#include <vulkan/vulkan.h>
#include <vulkan-cpp/types.hpp>

namespace vk {

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
                    const framebuffer_params& p_setting);

        [[nodiscard]] bool alive() const { return m_framebuffer; }

        void destroy();

        operator VkFramebuffer() const { return m_framebuffer; }

        operator VkFramebuffer() { return m_framebuffer; }

    private:
        VkDevice m_device = nullptr;
        VkFramebuffer m_framebuffer = nullptr;
    };
};