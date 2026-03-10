cmake_minimum_required(VERSION 3.27)
project(skybox CXX)

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
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_23)

target_sources(${PROJECT_NAME} PUBLIC
    FILE_SET CXX_MODULES
    TYPE CXX_MODULES
    FILES
    skybox.cppm
)