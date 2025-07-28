cmake_minimum_required(VERSION 3.27)
project(vulkan-cpp CXX)

build_library(
    DIRECTORIES src demos/4-surface
    ENABLE_TESTS OFF

    PACKAGES
    glfw3
    fmt
    Vulkan
    VulkanHeaders

    LINK_PACKAGES
    glfw
    fmt::fmt
    Vulkan::Vulkan
    vulkan-headers::vulkan-headers
)

generate_compile_commands()

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_LIST_DIR})
