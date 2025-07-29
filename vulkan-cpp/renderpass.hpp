#pragma once
#include <vulkan-cpp/types.hpp>
#include <vulkan/vulkan.h>
#include <array>
namespace vk {
    struct renderpass_begin_info {
        VkCommandBuffer current_command=nullptr;
        VkExtent2D extent;
        VkFramebuffer current_framebuffer=nullptr;
        std::array<float, 4> color;
        subpass_contents subpass;
    };

    class renderpass {
    public:
        renderpass() = default;
        renderpass(const VkDevice& p_device, const renderpass_attachments& p_attachemnts);

        void create(const renderpass_attachments& p_attachemnts);

        [[nodiscard]] bool alive() const { return m_renderpass; }

        void begin(const renderpass_begin_info& p_begin_info);
        void end(const VkCommandBuffer& p_current);

        void destroy();


        operator VkRenderPass() const { return m_renderpass; }

        operator VkRenderPass() { return m_renderpass; }

    private:
        VkDevice m_device=nullptr;
        VkRenderPass m_renderpass=nullptr;
    };

};