cmake_minimum_required(VERSION 4.0)

# Generate compile commands for anyone using our libraries.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_COLOR_DIAGNOSTICS ON)

project(vulkan-cpp LANGUAGES CXX)

static_library(
    ENABLE_TESTS OFF

    INCLUDE_DIRS ${CMAKE_CURRENT_LIST_DIR}

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
