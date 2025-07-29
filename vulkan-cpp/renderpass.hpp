#pragma once
#include <vulkan-cpp/types.hpp>
#include <vulkan/vulkan.h>

namespace vk {

    class renderpass {
    public:
        renderpass() = default;
        renderpass(const VkDevice& p_device, const renderpass_attachments& p_attachemnts);

        void create(const renderpass_attachments& p_attachemnts);

        [[nodiscard]] bool alive() const { return m_renderpass; }

        void destroy();


        operator VkRenderPass() const { return m_renderpass; }

        operator VkRenderPass() { return m_renderpass; }

    private:
        VkDevice m_device=nullptr;
        VkRenderPass m_renderpass=nullptr;
    };

};