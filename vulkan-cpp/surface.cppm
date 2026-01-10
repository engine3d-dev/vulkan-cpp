module;

#define GLFW_INCLUDE_VULKAN
#if _WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include <vulkan/vulkan.h>
#else
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#endif

export module vk:surface;


export import :types;
export import :utilities;
export import :instance;

export namespace vk {
    inline namespace v1 {

        class surface {
        public:
            surface(const VkInstance& p_instance, GLFWwindow* p_window_handle) : m_instance(p_instance) {
                vk_check(glfwCreateWindowSurface( m_instance, p_window_handle, nullptr, &m_surface_handler), "glfwCreateWindowSurface");
            }

            [[nodiscard]] bool alive() const { return m_surface_handler; }

            void destroy() {
                if (m_surface_handler != nullptr) {
                    vkDestroySurfaceKHR(m_instance, m_surface_handler, nullptr);
                }
                vkDestroyInstance(m_instance, nullptr);
            }

            operator VkSurfaceKHR() const { return m_surface_handler; }
            operator VkSurfaceKHR() { return m_surface_handler; }

        private:
            VkInstance m_instance = nullptr;
            VkSurfaceKHR m_surface_handler = nullptr;
        };

    };
};