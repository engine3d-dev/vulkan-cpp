
set(
    all_src
    vulkan-cpp/instance.cpp
    vulkan-cpp/utilities.cpp
    vulkan-cpp/physical_device.cpp
    vulkan-cpp/device.cpp
    vulkan-cpp/device_queue.cpp
)

add_library(${PROJECT_NAME} ${all_src})


