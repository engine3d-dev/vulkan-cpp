# Demo 16: Descriptor Indexing


In this demo, I show how to get descriptor indexing to work properly using vulkan-cpp's wrappers.


## Enabling Descriptor Indexing Features

To enable the descriptor indexing features, you must add onto the `vk::device_features` initialization here, as follows.

```C++

vk::device_features device_features{
    vk::dynamic_rendering_feature{ {
        .dynamicRendering = true,
    } },

    // ADD This: Enabling descriptor indexing
    vk::descriptor_indexing_feature{ {
        .descriptorBindingPartiallyBound = true,
        .runtimeDescriptorArray = true,
        .descriptorBindingVariableDescriptorCount = true,
        .descriptorBindingSampledImageUpdateAfterBind = true,
        .shaderSampledImageArrayNonUniformIndexing = true,
    } },
};
```

## Configuring Descriptor Flags

After enabling the descriptor indexing features. You must set flags to your descriptor binding layouts. In vulkan-cpp, these are referred to as `vk::descriptor_entry`.

Specifically these flags:
* `vk::descriptor_bind_flags::partially_bound_bit`
* `vk::descriptor_bind_flags::variable_descriptor_count_bit`
* `vk::descriptor_bind_flags::update_after_bind`

As you can see below, I add these flags onto this specific binding point entry of this specific resource.

These flags are a way to indicate this specific layout binding may have a max amount of descriptor (resources) that may be utilized at this specific binding point in the shader.

```C++
std::vector<vk::descriptor_entry> entries_set1 = {
    vk::descriptor_entry{
        // layout (set = 1, binding = 0) uniform sampler2D
        .type = vk::descriptor_type::combined_image_sampler,
        .binding_point = {
            .binding = 0,
            .stage = vk::shader_stage::fragment,
        },
        .descriptor_count = 1,
        .flags = vk::descriptor_bind_flags::partially_bound_bit |
                    vk::descriptor_bind_flags::variable_descriptor_count_bit |
                    vk::descriptor_bind_flags::update_after_bind,
    }
};
```

## Configuring Descriptor Layout

After the descriptor entryr has its flags set. You are going to need to set the descriptor layout `.flag` parameters to `vk::descriptor_layout_flags::update_after_bind_pool`.

This is a third parameter part of the `vk::descriptor_resource` constructor, since this is only ever needed to be set if you plan to use descriptor indexing to keep support for both traditional and modern approaches.

You also have to set the max descriptors so Vulkan knows how much slots that may be in-used. In this demno, I only specify `1`. Though if you had, lets say 10. You would set max descriptors to `10`.

Here is how you'd set the flag in the constructor:

```C++
uint32_t max_descriptor = 1;
vk::descriptor_layout set1_layout = {
    .slot = 1,
    .max_sets = image_count,
    .entries = entries_set1,
    .descriptor_counts = std::span<const uint32_t>(&max_descriptor, 1),
};

vk::descriptor_resource set1_resource(logical_device, set1_layout, vk::descriptor_layout_flags::update_after_bind_pool);
```

## Update Descriptors

In this demo, you can leave the descriptors to be updated the same way it has been previously since we only ever use a single texture.

Where we update them like this:

```C++
vk::texture_params config_texture = {
    .memory_mask =
        physical_device.memory_properties(static_cast<vk::memory_property>(
        vk::memory_property::host_visible_bit |
        vk::memory_property::host_cached_bit)),
};
vk::texture texture1(logical_device,
                        std::filesystem::path("asset_samples/viking_room.png"),
                        config_texture);

std::array<vk::write_image, 1> samplers = {
    vk::write_image{
        .sampler = texture1.image().sampler(),
        .view = texture1.image().image_view(),
        .layout = vk::image_layout::shader_read_only_optimal,
    },
};

// Specify image descriptor images/samplers to the descriptor
std::array<vk::write_image_descriptor, 1> set1_samples = {
    vk::write_image_descriptor{
        .dst_binding = 0,
        .sample_images = samplers,
    }
};

set1_resource.update({}, set1_samples);
```

## Push Constants

Then for accessibility to be able to use the textures specifically, we create a push_constant data below to use for our index data to retrieve specific textures.

```C++
struct push_constant_data {
    uint32_t texture_index=0;
};
```

## Configuring `vk::pipeline` for Push Constant

Make sure to configur the graphics pipeline to ensure it will handle the push constants correctly.

Below is a demonstration of the only parameters needed for specifying the parameters for sending over push constants.

```C++

// ADD THIS: Used for specifying which accessibility in shader stages to access push constants data
uint32_t format = static_cast<uint32_t>(surface_properties.format.format);
uint32_t vertex_mask = static_cast<uint32_t>(vk::shader_stage::vertex);
uint32_t fragment_mask = static_cast<uint32_t>(vk::shader_stage::fragment);
uint32_t stage_mask = vertex_mask | fragment_mask;
vk::shader_stage stage = static_cast<vk::shader_stage>(stage_mask);

// ADD THIS
vk::push_constant_range range = {
    .stage = stage,
    .offset = 0,
    .range = sizeof(push_constant_data),
};
vk::pipeline_params pipeline_configuration = {
    .use_render_pipeline = true,
    .color_attachment_formats = std::span<const uint32_t>(&format, 1),
    .depth_format = static_cast<uint32_t>(depth_format),
    .stencil_format = static_cast<uint32_t>(depth_format),
    .renderpass = nullptr,
    .shader_modules = geometry_resource.handles(),
    .vertex_attributes = geometry_resource.vertex_attributes(),
    .vertex_bind_attributes = geometry_resource.vertex_bind_attributes(),
    .descriptor_layouts = layouts,
    .color_blend = {
        .attachments = color_blend_attachments,
    },
    .depth_stencil_enabled = true,
    .dynamic_states = dynamic_states,

    // ADD THIS
    .push_constants = std::span<const vk::push_constant_range>(&range, 1),
};
```


## Transferring Push Constant Data

Now, that we have done the hard part of doing the boilerplate. You can then send over your texture index to access the texture and you should receive the same result when you do 

In here, you will see in the mainloop where, I call `vk::pipeline::push_constant` API to transfer over the push constant data over to the shader.

```C++
push_constant_data push = {
    .texture_index = 0,
};
main_graphics_pipeline.push_constant<push_constant_data>(
    current, push, stage, 0, sizeof(push_constant_data));
```


## Thats it!

As soon, you transfer over the push constants. You should still see the same viking room demo from demo 12. Where I use that same demo to demonstrate using descriptor indexing to fetch the viking room texture for the 3D mesh.

