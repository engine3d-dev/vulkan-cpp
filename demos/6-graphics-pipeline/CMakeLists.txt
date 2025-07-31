cmake_minimum_required(VERSION 3.27)
project(graphics-pipeline CXX)

build_application(
    SOURCES
    application.cpp

    PACKAGES
    Vulkan
    VulkanHeaders
    # vulkan-cpp

    LINK_PACKAGES
    Vulkan::Vulkan
    vulkan-headers::vulkan-headers
    # vulkan-cpp::vulkan-cpp
    vulkan-cpp
)