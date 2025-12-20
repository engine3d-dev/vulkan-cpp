cmake_minimum_required(VERSION 3.27)
project(vulkan-cpp CXX)

build_library(
    DIRECTORIES
    src
    demos/1-instance
    demos/2-physical-device
    demos/3-logical-device
    demos/4-surface
    demos/5-swapchain
    demos/6-graphics-pipeline
    # demos/7-vertex-buffer
    # demos/8-index-uniform-buffers
    # demos/9-uniforms
    # demos/10-textures
    # demos/11-depth-buffering
    # demos/12-loading-models
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
    vk-modules/vk.cppm
    vk-modules/types.cppm
    vk-modules/utilities.cppm
    vk-modules/instance.cppm
    vk-modules/physical_device.cppm
    vk-modules/device.cppm
    vk-modules/device_queue.cppm
    vk-modules/surface.cppm
    vk-modules/swapchain.cppm
    vk-modules/device_present_queue.cppm
    vk-modules/command_buffer.cppm
    vk-modules/renderpass.cppm
    vk-modules/framebuffer.cppm
    vk-modules/sample_image.cppm
    vk-modules/shader_resource.cppm
    vk-modules/pipeline.cppm
)

generate_compile_commands()

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_LIST_DIR})
target_include_directories(${PROJECT_NAME} PUBLIC ./)
