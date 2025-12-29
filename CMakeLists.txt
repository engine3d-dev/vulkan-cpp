cmake_minimum_required(VERSION 3.27)
project(vulkan-cpp CXX)

build_library(
    DIRECTORIES vulkan-cpp/
    demos/1-instance
    demos/2-physical-device
    demos/3-logical-device
    demos/4-surface
    demos/5-swapchain
    demos/6-graphics-pipeline
    demos/7-vertex-buffer
    demos/8-index-uniform-buffers
    demos/9-uniforms
    demos/10-textures
    demos/11-depth-buffering
    demos/12-loading-models
    # demos/13-skybox
    # demos/sandbox
    ENABLE_TESTS OFF

    PACKAGES
    glfw3
    Vulkan
    VulkanHeaders
    glm
    stb
    tinyobjloader

    LINK_PACKAGES
    glfw
    Vulkan::Vulkan
    vulkan-headers::vulkan-headers
    glm::glm
    stb::stb
    tinyobjloader::tinyobjloader
)

target_sources(${PROJECT_NAME} PUBLIC
    FILE_SET CXX_MODULES
    TYPE CXX_MODULES
    FILES
    vulkan-cpp/vk.cppm
    vulkan-cpp/types.cppm
    vulkan-cpp/utilities.cppm
    vulkan-cpp/instance.cppm
    vulkan-cpp/physical_device.cppm
    vulkan-cpp/device.cppm
    vulkan-cpp/device_queue.cppm
    vulkan-cpp/surface.cppm
    vulkan-cpp/swapchain.cppm
    vulkan-cpp/device_present_queue.cppm
    vulkan-cpp/command_buffer.cppm
    vulkan-cpp/renderpass.cppm
    vulkan-cpp/framebuffer.cppm
    vulkan-cpp/sample_image.cppm
    vulkan-cpp/shader_resource.cppm
    vulkan-cpp/pipeline.cppm
    vulkan-cpp/buffer_streams.cppm
    vulkan-cpp/vertex_buffer.cppm
    vulkan-cpp/buffer_streams16.cppm
    vulkan-cpp/buffer_streams32.cppm
    vulkan-cpp/index_buffer.cppm
    vulkan-cpp/uniform_buffer.cppm
    vulkan-cpp/descriptor_resource.cppm
    vulkan-cpp/texture.cppm
)

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_LIST_DIR})
target_include_directories(${PROJECT_NAME} PUBLIC ./)
