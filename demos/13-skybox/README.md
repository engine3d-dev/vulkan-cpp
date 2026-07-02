## Demo 13 Skybox

In demo 13, shows how to configure with the vulkan-cpp to get a skybox to work with the most minimal requirements.

I am going to be using stbi_image library to help with loading the three faces.

Note: There are two files in demo 13, one called `skybox.cppm` and another called `hdri_environment.cppm`. The `skybox.cppm` file is what this README will walk you through.

If you are interested in learning how to load an HDRI environment specifically, checkout `hdri_environment.cppm` as the setup is quite similar. Only difference is you would be loading a single image file and changing from `stbi_load` to `stbi_loadf` instead.

## Loading 6 faces for cubemap

To load the skybox, I have provided a skybox directory inside of the assets directory.

In `asset_samples/skybox`, you should see 6 `.jpg` images. These are going to be the images we use.

Before configuring any vulkan handles, make sure to store the path correctly to those images. As shown below:

```C++
std::array<std::string, 6> faces = {
    "asset_samples/skybox/right.jpg", "asset_samples/skybox/left.jpg",
    "asset_samples/skybox/top.jpg",   "asset_samples/skybox/bottom.jpg",
    "asset_samples/skybox/front.jpg", "asset_samples/skybox/back.jpg"
};
```

To load all 6 of those faces, use stbi_image to load those images. The way, how I have it implemented is by doing the following:

- First we validate that the images specified are 6 images only.
- We load in the first image to specify the parameters for the dimensions, and the total size in bytes of that image (as they should all be the equivalent).
- Starting at i=1, we load the other 5 images into an array, being `std::array<std::span<uint8_t>, 6>`

```C++
if (faces.size() != 6) {
    std::println("Cubemap requires 6 faces, received {} count of faces",
                    faces.size());
    return;
}

int w = 0;
int h = 0;
int channels = 0;
std::array<std::span<uint8_t>, 6> faces{};

auto* face0 =
    stbi_load(faces[0].c_str(), &w, &h, &channels, STBI_rgb_alpha);
int face_width = w;
int face_height = h;

VkFormat image_format = VK_FORMAT_R8G8B8A8_SRGB;
const uint32_t bytes_per_pixel =
    static_cast<uint32_t>(vk::bytes_per_texture_format(image_format));
auto size_bytes = face_width * face_height * bytes_per_pixel;

faces[0] = to_bytes(face0, size_bytes);

for (size_t i = 1; i < faces.size(); i++) {
    auto* face_pixels =
        stbi_load(faces[i].c_str(), &w, &h, &channels, STBI_rgb_alpha);
    faces[i] = to_bytes(face_pixels, size_bytes);

    if (faces[i].empty()) {
        std::println("Could not load face: {}", faces[i]);
        return;
    }

    if (w != face_width || h != face_height) {
        std::println("Cubemap faces must match dimensions. Face 0 is "
                        "{}x{}, face {} is {}x{} ({})",
                        face_width,
                        face_height,
                        i,
                        w,
                        h,
                        p_faces[i]);
        return;
    }
}
```

## Configure `vk::sample_image`

By now you should be aware that `vk::sample_image`, is essentially a wrapper around the creation for the `VkImage`, `VkImageView`, and `VkSampler`.

For this skybox demo, these are the parameters to configure those handles with:

```C++
vk::image_params skybox_params = {
    .extent = { .width = width, .height = height, .depth = 1 },
    .format = image_format,
    .memory_mask = p_physical.memory_properties(
        vk::memory_property::device_local_bit),
    .usage =
        vk::image_usage::transfer_dst_bit | vk::image_usage::sampled_bit,
    .image_flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT,
    .view_type = VK_IMAGE_VIEW_TYPE_CUBE,
    .layer_count = 6,
    .array_layers = 6,
};
m_skybox_image = vk::sample_image(m_device, skybox_params);
```

- `extent` is to specify the width and height for the given image. Since this is a skybox, the width and height should relatively be the same.
- `format` is set to `VK_FORMAT_R8G8B8A8_SRGB`
- `p_physical.memory_properties` is specified to be `device_local_bit` for allocating this image in high-speed, VRAM-only memory local to the GPU.
- `usage` specifies with `image_usage::transfer_dst_bit` because the demo copies raw pixels from staging into this image, we also bitwise OR `image_usage::sampled_bit` so the fragment shader can read it using `sampler2D`.
- `image_flags` is configured with `VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT` because we are creating this image to store 6 regions for a skybox and this tells the image for what kind of data to expect.
- `view_type` configures `VK_IMAGE_VIEW_TYPE_CUBE` to tell the image not to treat the data as a flat texture array.
- `layer_count` specifies that the image will have 6 distinct image layers
- `array_layers` specifies how many texture slices are allocated to VRAM.

## Configuring Staging Buffer

We need to create a staging buffer to allow for CPU-visible data to be transferred to via `host_visible` without manually cashe flushing (host_coherent). Populating this buffer with the skybox pixels we loaded, earlier.

Using this buffer as an entry point using the `transfer_src_bit` to do a fast data transfer onto the device-local skybox texture.

```C++
vk::buffer_parameters staging_params = {
    .memory_mask = p_physical.memory_properties(
        vk::memory_property::host_visible_bit |
        vk::memory_property::host_coherent_bit),
    .usage = vk::buffer_usage::transfer_src_bit,
};

vk::buffer staging(m_device, total_size_bytes, staging_params);
```

- `host_visible_bit | host_coherent_bit` is to allocate the buffer to system RAM or a special region of VRAM the CPU can directly write and automatically become visible to the GPU.
- `vk::buffer_usage::transfer_src_bit` is to tell the use of this buffer handle is specifically to store data, which allows the driver to optimize underlying memory allocations specifically for outbound direct memory transfers to the GPU memory.

## Transferring the faces pixels data

Then to perform the actual transfer operation for the pixels for the 6 faces. You just call the following API:

```C++
// faces is defined as std::array<std::span<uint8_t>, 6>
staging.transfer(faces);
```

## Cleanup stbi_image pixels

After transferring the pixels and already written, you can just simply perform cleanup:

```C++
for (size_t i = 0; i < faces.size(); i++) {
    stbi_image_free(faces[i].data());
}
```

## Mapping each Skybox Face Regions with Vulkan

The `std::array<vk::buffer_image_copy, 6>` allows for executing batch of offset stride copy operations. For each region (per-face), we tightly pack a single cubemap face to its source memory.

That source memory to an un-offset subresource footprint that targets different base array layers and other subresource parameters.

Allowing for those 6-face data streams to be represented as a linear buffer stream that optimally laid out for the device.


## Submitting to the Queue

Performing this queue submission is to conclude by actually having the data  to be fully copied over to the GPU.

```C++
VkQueue graphics_queue = nullptr;
vkGetDeviceQueue(m_device, 0, 0, &graphics_queue);
const VkCommandBuffer cmd = upload_cmd;
VkSubmitInfo submit = {
    .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
    .pNext = nullptr,
    .waitSemaphoreCount = 0,
    .pWaitSemaphores = nullptr,
    .pWaitDstStageMask = nullptr,
    .commandBufferCount = 1,
    .pCommandBuffers = &cmd,
    .signalSemaphoreCount = 0,
    .pSignalSemaphores = nullptr,
};
vk::vk_check(vkQueueSubmit(graphics_queue, 1, &submit, nullptr),
                "vkQueueSubmit(cubemap upload)");
vk::vk_check(vkQueueWaitIdle(graphics_queue),
                "vkQueueWaitIdle(cubemap upload)");

upload_cmd.destruct();
staging.destruct();
```

## Next Steps

The `skybox.cppm` implementation actually just recreates a separate `vk::pipeline`, `vk::shader_resource`, `vk::descriptor_resource`, and `vk::uniform_buffer` for the skybox shaders.

If you have been following along in the vulkan-cpp demos where we use `vk::pipeline` (graphics pipeline) API, `skybox.cppm` follows the same way we have always been configuring and initializing those handles.

This README is meant to give you an idea on the core portions of the code for loading and performing staging correctly with a skybox. Since this demo, has quite a significant amount of code involved.