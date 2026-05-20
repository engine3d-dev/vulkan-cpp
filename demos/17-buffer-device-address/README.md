# Demo 17: Buffer Device Address


In this demo 17, will show how to get buffer device addresses working using vulkan-cpp APIs.

## Enabling Buffer Device Addresses

To enable buffer device addresses, add onto the `vk::device_features` initialization here:

```C++
vk::device_features device_features{
    vk::dynamic_rendering_feature{ {
        .dynamicRendering = true,
    } },
    vk::descriptor_indexing_feature{ {
        .shaderSampledImageArrayNonUniformIndexing = true,
        .descriptorBindingSampledImageUpdateAfterBind = true,
        .descriptorBindingPartiallyBound = true,
        .descriptorBindingVariableDescriptorCount = true,
        .runtimeDescriptorArray = true,
    } },

    // NEW: Add buffer device address feature here
    vk::buffer_device_address{ {
        .bufferDeviceAddress = true,
    } },
};
```

## Replacing `vk::uniform_buffer` with `vk::dyn::buffer`

The `vk::dyn::buffer` is meant to be a buffer abstraction similar to `vk::buffer` with an additional API to getting the buffers device address from the buffer.

Which also does performs an automated configuration to for buffer device address.

The initialization for the test_ubo variable is quite similar.

```C++

vk::buffer_parameters uniform_params = {
    .memory_mask =
        physical_device.memory_properties(static_cast<vk::memory_property>(
        vk::memory_property::host_visible_bit |
        vk::memory_property::host_cached_bit)),
    .usage =
        static_cast<uint32_t>(vk::buffer_usage::uniform_buffer_bit |
                            vk::buffer_usage::shader_device_address_bit),
    .allocate_flags = vk::memory_allocate_flags::device_address_bit_khr,
};

// NEW: Change this from vk::uniform_buffer to vk::dyn::buffer
vk::dyn::buffer test_ubo =
    vk::dyn::buffer(logical_device, sizeof(global_uniform), uniform_params);
```

## Getting the Device Address

In the mainloop, you can retrieve the buffer device address through the `.get_device_address` API.


When getting the `ubo_addr`, you pass this into the push constant to send in the address to retrieve that specific buffer to access.

```C++

while (!glfwWindowShouldClose(window)) {
    // ...
    // Performing some operation
    global_uniform ubo = {
        .model = glm::rotate(glm::mat4(1.0f),
                                time * glm::radians(90.0f),
                                glm::vec3(0.0f, 0.0f, 1.0f)),
        .view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f),
                            glm::vec3(0.0f, 0.0f, 0.0f),
                            glm::vec3(0.0f, 0.0f, 1.0f)),
        .proj = glm::perspective(glm::radians(45.0f),
                                    (float)swapchain_extent.width /
                                    (float)swapchain_extent.height,
                                    0.1f,
                                    10.0f)
    };
    ubo.proj[1][1] *= -1;

    test_ubo.transfer<global_uniform>(std::span<const gloabl_uniform>(&ubo, 1));

    // NEW: ADD THIS LINE
    const uint64_t ubo_addr = test_ubo.get_device_address();

    push_constant_data push = {
        .texture_index = 0,

        // NEW: Set device address here
        .global_ubo_addr = ubo_addr,
    };
    main_graphics_pipeline.push_constant<push_constant_data>(
        current, push, stage, 0, sizeof(push_constant_data));

    // ...
}

```

## Changes to the shader

### `vertex` shader

In the vertex shader, we originally had used `layout(set = 0, binding = 0)` to contain a descriptor set that allows us to write our buffer.

Now, that we are using a buffer device address, we can change that layout to being `layout(buffer_reference, std140)`.

Here is what `layout(buffer_reference, std140)`:

- `buffer_reference`: is to tell the resource lookup that the layout of this uniform is not going to be lookedup as a descriptor binding rather as a reference to this buffer.
- `std140`: Just set what the alignment rules are in how these uniform data can be aligned.

```C++
#version 450

#extension GL_EXT_buffer_reference : require

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoords;
layout(location = 3) in vec3 inNormals;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoords;
layout(location = 2) out vec3 fragNormals;
layout(location = 3) out flat int fragTexIndex;

// NEW: Replace layout(set=0, binding=1)
layout(buffer_reference, std140) buffer readonly UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
};

layout(push_constant) uniform Constants {
    int texture_index;

    // NEW: Pass uint64_t from the push_constant_data and this
    // specifies what piece of data to access from buffer device address previously stored
    UniformBufferObject global_ubo_address;
} push_const;



void main() {

    // NEW: Add this here to retrieve the global uniform buffer data
    UniformBufferObject ubo = push_const.global_ubo_address;

    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition, 1.0);
    fragColor = inColor;
    fragTexCoords = inTexCoords;
    fragNormals = inNormals;
    fragTexIndex = push_const.texture_index;
}
```

## That is it!

Configuring buffer device address and using this Vulkan feature to access your uniform buffers are now much easier through the use of buffer device address.

This will reduce the need to have to manage an entire descriptor sets for specifically buffers, and to just provide the address for the shader to directly access the buffer already stored in memory the GPU already has access.

