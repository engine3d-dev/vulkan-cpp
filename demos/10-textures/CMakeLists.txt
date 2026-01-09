cmake_minimum_required(VERSION 3.27)
project(textures CXX)

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
    Vulkan::Vulkan
)