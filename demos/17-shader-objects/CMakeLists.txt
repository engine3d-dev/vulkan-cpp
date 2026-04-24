cmake_minimum_required(VERSION 4.0)
project(shader-objects CXX)

build_application(
    SOURCES
    application.cpp

    PACKAGES
    vulkan-cpp
    Vulkan
    glfw3
    glm
    stb
    tinyobjloader

    LINK_PACKAGES
    vulkan-cpp
    tinyobjloader
    Vulkan::Vulkan
)