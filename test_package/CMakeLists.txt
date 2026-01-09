cmake_minimum_required(VERSION 3.28)
project(test_package CXX)

build_application(
    SOURCES
    application.cpp

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
    tinyobjloader::tinyobjloader
)
