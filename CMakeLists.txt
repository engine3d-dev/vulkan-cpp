# cmake_minimum_required(VERSION 4.0)

# # Generate compile commands for anyone using our libraries.
# set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
# set(CMAKE_COLOR_DIAGNOSTICS ON)

# project(vulkan-cpp LANGUAGES CXX)

# # build_library(
# #     DIRECTORIES vulkan-cpp/
# #     # demos/1-instance
# #     # demos/2-physical-device
# #     # demos/3-logical-device
# #     # demos/4-surface
# #     # demos/5-swapchain
# #     # demos/6-graphics-pipeline
# #     # demos/7-vertex-buffer
# #     # demos/8-index-uniform-buffers
# #     # demos/9-uniforms
# #     # demos/10-textures
# #     # demos/11-depth-buffering
# #     # demos/12-loading-models
# #     # demos/13-skybox
# #     # demos/sandbox
# #     ENABLE_TESTS OFF

# #     PACKAGES
# #     glfw3
# #     Vulkan
# #     glm
# #     stb
# #     tinyobjloader

# #     LINK_PACKAGES
# #     glfw
# #     Vulkan::Vulkan
# #     glm::glm
# #     stb::stb
# #     tinyobjloader::tinyobjloader
# # )

cmake_minimum_required(VERSION 4.0)

# Generate compile commands for anyone using our libraries.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_COLOR_DIAGNOSTICS ON)

project(vulkan-cpp LANGUAGES CXX)

# add_library(${PROJECT_NAME} STATIC)
build_library(
    DIRECTORIES vulkan-cpp/
    ENABLE_TESTS OFF

    PACKAGES
    glfw3
    Vulkan
    glm
    stb

    LINK_PACKAGES
    glfw
    Vulkan::Vulkan
    glm::glm
    stb::stb
)

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_23)

target_sources(${PROJECT_NAME} PUBLIC
    FILE_SET CXX_MODULES
    TYPE CXX_MODULES
    FILES
    vulkan-cpp/vk.cppm
    vulkan-cpp/types.cppm
    vulkan-cpp/utilities.cppm
    vulkan-cpp/instance.cppm
    vulkan-cpp/physical_device.cppm
    vulkan-cpp/device.cppm
    vulkan-cpp/device_queue.cppm
    vulkan-cpp/surface.cppm
    vulkan-cpp/swapchain.cppm
    vulkan-cpp/device_present_queue.cppm
    vulkan-cpp/command_buffer.cppm
    vulkan-cpp/renderpass.cppm
    vulkan-cpp/framebuffer.cppm
    vulkan-cpp/sample_image.cppm
    vulkan-cpp/shader_resource.cppm
    vulkan-cpp/pipeline.cppm
    vulkan-cpp/buffer_streams.cppm
    vulkan-cpp/vertex_buffer.cppm
    vulkan-cpp/buffer_streams16.cppm
    vulkan-cpp/buffer_streams32.cppm
    vulkan-cpp/index_buffer.cppm
    vulkan-cpp/uniform_buffer.cppm
    vulkan-cpp/descriptor_resource.cppm
    vulkan-cpp/texture.cppm
)

install(
    TARGETS ${PROJECT_NAME}
    EXPORT vulkan-cpp_targets
    FILE_SET CXX_MODULES DESTINATION "."
    LIBRARY DESTINATION "lib"
    ARCHIVE DESTINATION "lib"
    CXX_MODULES_BMI DESTINATION "bmi"
)

install(
    EXPORT vulkan-cpp_targets
    FILE "vulkan-cpp-config.cmake"
    DESTINATION "lib/cmake"
    CXX_MODULES_DIRECTORY "cxx-modules"
)

# Always run this custom target by making it depend on ALL
add_custom_target(copy_compile_commands ALL
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${CMAKE_BINARY_DIR}/compile_commands.json
    ${CMAKE_SOURCE_DIR}/compile_commands.json
    DEPENDS ${CMAKE_BINARY_DIR}/compile_commands.json)
