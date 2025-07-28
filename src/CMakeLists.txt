
set(
    all_src
    vulkan-cpp/instance.cpp
    vulkan-cpp/utilities.cpp
    vulkan-cpp/physical_device.cpp
)

add_library(${PROJECT_NAME} ${all_src})


