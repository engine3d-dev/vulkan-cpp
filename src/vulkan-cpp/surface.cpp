#include <vulkan-cpp/imports.hpp>
#include <vulkan-cpp/surface.hpp>
#include <vulkan-cpp/utilities.hpp>

namespace vk {
    surface::surface(const VkInstance& p_instance, GLFWwindow* p_window_handle) : m_instance(p_instance) {

        vk_check(glfwCreateWindowSurface(m_instance, p_window_handle, nullptr, &m_surface_handler),"glfwCreateWindowSurface");

    }

    void surface::destroy() {
        if(m_surface_handler != nullptr) {
            vkDestroySurfaceKHR(m_instance, m_surface_handler, nullptr);
        }
        vkDestroyInstance(m_instance, nullptr);
    }
};