
set(
    all_src
    vulkan-cpp/instance.cpp
    vulkan-cpp/utilities.cpp
    vulkan-cpp/physical_device.cpp
    vulkan-cpp/device.cpp
    vulkan-cpp/device_queue.cpp
    vulkan-cpp/surface.cpp
    vulkan-cpp/swapchain.cpp
    vulkan-cpp/device_present_queue.cpp
    vulkan-cpp/command_buffer.cpp
    vulkan-cpp/renderpass.cpp
    vulkan-cpp/framebuffer.cpp
    vulkan-cpp/shader_resource.cpp
    vulkan-cpp/pipeline.cpp
    vulkan-cpp/vertex_buffer.cpp
)

add_library(${PROJECT_NAME} ${all_src})


