cmake_minimum_required(VERSION 3.27)
project(uniforms CXX)

build_application(
    SOURCES
    application.cpp

    PACKAGES
    vulkan-cpp
    Vulkan
    glfw3
    glm
    stb

    LINK_PACKAGES
    vulkan-cpp
    Vulkan::Vulkan
)