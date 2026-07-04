module;

#include <vulkan/vulkan.h>
#include <span>

export module vk::timeline_semaphore;

import :types;
import :utilies;

export namespace vk {
    inline namespace v6 {

        /**
         * @brief Wrapper around VkSemaphore that creates a timeline semaphore
         */
        class timeline_semaphore {
        public:
            timeline_semaphore(const VkDevice& p_device,
                               uint64_t p_initial_value)
              : m_device(p_device)
              , m_value(p_initial_value) {
                VkSemaphoreTypeCreateInfo timeline_semaphore_info = {
                    .sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO,
                    .pNext = nullptr,
                    .semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE,
                    .initialValue = p_initial_value,
                };

                VkSemaphoreCreateInfo semaphore_ci = {
                    .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
                    .pNext = &timeline_semaphore_info,
                };

                vk_check(vkCreateSemaphore(
                           m_device, &semaphore_ci, nullptr, &m_semaphore),
                         "vkCreateSemaphore");
            }

            /**
             * @return the initial value for this specific timeline semaphore
             */
            [[nodiscard]] uint64_t value() const { return m_value; }

            void destruct() {
                if (m_semaphore != nullptr) {
                    vkDestroySemaphore(m_device, m_semaphore, nullptr);
                }
            }

            /**
             * @return VkSemaphoreSubmitInfo struct for submitting this timeline semaphore.
            */
            VkSemaphoreSubmitInfo data(void* p_next=nullptr) const {
                return {
                    .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
                    .pNext = p_next,
                    .semaphore = m_sempahore,
                    .value = m_value,
                    .stage_flag = m_stage_flag,
                };
            }

            VkSemaphore operator() { return m_semaphore; }

            VkSemaphore operator() const { return m_semaphore; }

        private:
            VkDevice m_device = nullptr;
            VkSemaphore m_semaphore;
            VkPipelineStageFlags m_stage_flag;
            uint64_t m_value = 0;
        };
    };
};