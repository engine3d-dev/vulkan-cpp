cmake_minimum_required(VERSION 4.0)
project(descriptor-indexing CXX)

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
    stb::stb
    vulkan-cpp
    tinyobjloader
    Vulkan::Vulkan
)