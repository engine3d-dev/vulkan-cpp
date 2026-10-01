cmake_minimum_required(VERSION 4.0)
project(demos CXX)

build_demos_exe(
    DEMO_SOURCES
    1-instance/1-instance.cpp
    2-physical-device/2-physical-device.cpp
    3-logical-device/3-logical-device.cpp
    4-surface/4-surface.cpp
    5-swapchain/5-swapchain.cpp
    6-graphics-pipeline/6-graphics-pipeline.cpp
    7-vertex-buffer/7-vertex-buffer.cpp
    8-index-buffer/8-index-buffer.cpp
    9-uniforms/9-uniforms.cpp
    10-textures/10-textures.cpp
    11-depth-buffer/11-depth-buffer.cpp
    12-loading-models/12-loading-models.cpp
    13-skybox/13-skybox.cpp
    14-imgui/14-imgui.cpp
    15-dynamic-rendering/15-dynamic-rendering.cpp
    16-descriptor-indexing/16-descriptor-indexing.cpp
    17-buffer-device-address/17-buffer-device-address.cpp

    PACKAGES
    vulkan-cpp
    glfw3
    Vulkan
    glm
    stb
    tinyobjloader

    LINK_PACKAGES
    vulkan-cpp
    glfw
    Vulkan::Vulkan
    glm::glm
    stb::stb
    tinyobjloader
)


target_sources(13-skybox PUBLIC
    FILE_SET CXX_MODULES
    TYPE CXX_MODULES
    FILES
    13-skybox/hdri_environment.cppm
    13-skybox/skybox.cppm
)