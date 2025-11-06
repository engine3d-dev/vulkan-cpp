#pragma once
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

namespace vk {

    class surface {
    public:
        surface(const VkInstance& p_device, GLFWwindow* p_window_handle);

        [[nodiscard]] bool alive() const { return m_surface_handler; }

        void destroy();

        operator VkSurfaceKHR() const { return m_surface_handler; }
        operator VkSurfaceKHR() { return m_surface_handler; }

    private:
        VkInstance m_instance = nullptr;
        VkSurfaceKHR m_surface_handler = nullptr;
    };
};