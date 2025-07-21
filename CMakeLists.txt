cmake_minimum_required(VERSION 3.27)
project(vulkan-cpp CXX)


# build_application(
#     SOURCES
#     application.cpp
#     src/vulkan-cpp/instance.cpp
#     src/vulkan-cpp/utilities.cpp

#     PACKAGES
#     glfw3
#     fmt
#     Vulkan
#     VulkanHeaders

#     LINK_PACKAGES
#     glfw
#     fmt::fmt
#     Vulkan::Vulkan
#     vulkan-headers::vulkan-headers
# )
build_library(
    DIRECTORIES src
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

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_LIST_DIR})
