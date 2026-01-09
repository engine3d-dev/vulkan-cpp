cmake_minimum_required(VERSION 3.28)
project(test_package CXX)


find_package(vulkan-cpp REQUIRED)
find_package(glfw3 REQUIRED)
# Vulkan
# glm
# stb
# tinyobjloader
find_package(Vulkan REQUIRED)
find_package(glm REQUIRED)
find_package(stb REQUIRED)
find_package(tinyobjloader REQUIRED)

add_executable(${PROJECT_NAME})

target_sources(${PROJECT_NAME} PUBLIC
    FILE_SET CXX_MODULES
    TYPE CXX_MODULES
    PRIVATE application.cpp
)

# Verifying the dependencies audio-cpp::audio-cpp are setup correctly
# get_target_property(AUDIO_INCLUDE_DIRS miniaudio::miniaudio INTERFACE_INCLUDE_DIRECTORIES)
# message(STATUS "audio-cpp include dirs: ${AUDIO_INCLUDE_DIRS}")
target_link_libraries(
    ${PROJECT_NAME} PRIVATE
    vulkan-cpp
    glfw
    Vulkan::Vulkan
    glm::glm
    stb::stb
    tinyobjloader::tinyobjloader
)

# build_application(
#     SOURCES
#     application.cpp

#     PACKAGES
#     vulkan-cpp
#     glfw3
#     Vulkan
#     glm
#     stb
#     tinyobjloader

#     LINK_PACKAGES
#     vulkan-cpp
#     glfw
#     Vulkan::Vulkan
#     glm::glm
#     stb::stb
#     tinyobjloader::tinyobjloader
# )
