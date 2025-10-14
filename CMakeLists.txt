cmake_minimum_required(VERSION 3.27)
project(vulkan-cpp CXX)

build_library(
    DIRECTORIES
    src
    # demos/5-swapchain
    # demos/6-graphics-pipeline
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
    fmt
    Vulkan
    VulkanHeaders
    glm
    stb
    tinyobjloader

    LINK_PACKAGES
    glfw
    fmt::fmt
    Vulkan::Vulkan
    vulkan-headers::vulkan-headers
    glm::glm
    stb::stb
    tinyobjloader::tinyobjloader
)

generate_compile_commands()

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_LIST_DIR})
