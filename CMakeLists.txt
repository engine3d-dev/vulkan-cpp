cmake_minimum_required(VERSION 3.27)
project(vulkan-cpp CXX)

build_library(
    DIRECTORIES src demos/11-depth-buffering
    ENABLE_TESTS OFF

    PACKAGES
    glfw3
    fmt
    Vulkan
    VulkanHeaders
    glm
    stb

    LINK_PACKAGES
    glfw
    fmt::fmt
    Vulkan::Vulkan
    vulkan-headers::vulkan-headers
    glm::glm
    stb::stb
)

generate_compile_commands()

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_LIST_DIR})
