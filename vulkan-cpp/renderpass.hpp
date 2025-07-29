#pragma once
#include <vulkan-cpp/types.hpp>
#include <vulkan/vulkan.h>

namespace vk {

    /**
     * @name renderpass
     *
     * @param p_device is the logical device to associate the creation of
     * renderpasses
     * @param p_renderpass_attachment is a vk::attachment to specify the
     * individual attachment operation that handle in creating
     * VkAttachmentDescription, VkAttachmentReference, and VkSubpassDescription
     * 
     * @param p_enable_subpass because subpasses are optional, this is a boolean to enable if we want to apply subpasses
     */

    class renderpass {
    public:
        renderpass() = default;
        renderpass(const VkDevice& p_device,
                   const renderpass_attachments& p_attachemnts);
        renderpass(const VkDevice& p_device,
                   const std::span<attachment> p_renderpass_attachments,
                   bool p_enable_subpasses = true);

        void create(const renderpass_attachments& p_attachemnts);

        [[nodiscard]] bool alive() const { return m_renderpass; }

        void begin(const renderpass_begin_info& p_begin_info);
        void end(const VkCommandBuffer& p_current);

        void destroy();

        operator VkRenderPass() const { return m_renderpass; }

        operator VkRenderPass() { return m_renderpass; }

    private:
        VkDevice m_device = nullptr;
        VkRenderPass m_renderpass = nullptr;
    };

};