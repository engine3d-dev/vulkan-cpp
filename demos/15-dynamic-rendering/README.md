# Demo 15 -- Dynamic Rendering

This demo shows how to get dynamic rendering setup.


## Setting the Physical Device Features

Before using dynamic rendering, you need to set the dynamic rendering feature to be enabled. vulkan-cpp now has support for enabling this feature.

This introduces a struct called `vk::device_features` which performs the chaining operation at compile-time to automatically assign the `.pNext` parameter when you add them into the initializer list as shown below.

```C++
vk::device_features device_features{
    vk::dynamic_rendering_feature{ {
        .dynamicRendering = true,
    } },
};
```

Then `vk::device_features` provides you a way to get access to the internal pointer to the entire chain of the pNext pointer itself through an API call `.data()`.

When constructing a logical device, here is how you can pass in that pointer.

```C++


vk::device_params logical_params = {
    // setting logical device to the enabled features
    .features = device_features.data(),
    ....
};

vk::device logical_device(physical, logical_params);
```

## Previously

Originally before dynamic rendering, you would have to create a renderpass handle and framebuffers handles:

```C++
std::array<vk::attachment, 2> renderpass_attachments = {
    // color attachment
    vk::attachment{
        .format = surface_properties.format.format,
        .layout = vk::image_layout::color_optimal,
        .samples = vk::sample_bit::count_1,
        .load = vk::attachment_load::clear,
        .store = vk::attachment_store::store,
        .stencil_load = vk::attachment_load::dont_care,
        .stencil_store = vk::attachment_store::dont_care,
        .initial_layout = vk::image_layout::undefined,
        .final_layout = vk::image_layout::present_src_khr,
    },
    // depth attachment
    vk::attachment{
        .format = depth_format,
        .layout = vk::image_layout::depth_stencil_optimal,
        .samples = vk::sample_bit::count_1,
        .load = vk::attachment_load::clear,
        .store = vk::attachment_store::dont_care,
        .stencil_load = vk::attachment_load::dont_care,
        .stencil_store = vk::attachment_store::dont_care,
        .initial_layout = vk::image_layout::undefined,
        .final_layout = vk::image_layout::depth_stencil_read_only_optimal,
    },
};

vk::renderpass main_renderpass(logical_device, renderpass_attachments);

std::vector<vk::framebuffer> swapchain_framebuffers(image_count);
for (uint32_t i = 0; i < swapchain_framebuffers.size(); i++) {
    std::array<VkImageView, renderpass_attachments.size()>
        image_view_attachments = { swapchain_images[i].image_view(),
                                    swapchain_depth_images[i].image_view() };

    vk::framebuffer_params framebuffer_info = {
        .renderpass = main_renderpass,
        .views = image_view_attachments,
        .extent = swapchain_extent
    };
    swapchain_framebuffers[i] =
        vk::framebuffer(logical_device, framebuffer_info);
}
```



Then following by `vkCmdBeGinRenderPass` and `vkCmdEndRenderPass` calls to start rendering within that renderpass instance.
Which would look like this:

```C++
vk::renderpass_begin_params begin_renderpass = {
    .extent = swapchain_extent,
    .current_framebuffer = swapchain_framebuffers[current_frame],
    .color = color,
    .subpass = vk::subpass_contents::inline_bit
};
main_renderpass.begin(current, begin_renderpass);

main_renderpass.end();
```

## With Dynamic Rendering

Now with Dynamic Rendering. You no longer need to keep track and create framebuffers and renderpass handles at all.

You can completely remove the creation of the renderpass and framebuffer handles. Replace `vk::attachment` with `vk::rendering_attachment`.


Here are what the rendering attachments look like for dynamic rendering:

```C++
vk::rendering_attachment color_render_attachment = {
    .image_view = swapchain_images[current_frame].image_view(),
    .layout = vk::image_layout::color_optimal,
    .resolve_mode = vk::resolved_mode_flags::none,
    .resolve_image_view = nullptr,
    .resolve_image_layout = vk::image_layout::undefined,
    .load = vk::attachment_load::clear,
    .store = vk::attachment_store::store,
    .clear_values = clear_color
};

vk::rendering_attachment depth_stencil_attachment = {
    .image_view = swapchain_depth_images[current_frame].image_view(),
    .layout = vk::image_layout::depth_stencil_optimal,
    .resolve_mode = vk::resolved_mode_flags::none,
    .resolve_image_view = nullptr,
    .resolve_image_layout = vk::image_layout::undefined,
    .load = vk::attachment_load::clear,
    .store = vk::attachment_store::store,
    .depth_values = depth_value
};
```


## Calling vkCmdBeginRendering/vkCmdEndRendering
Then instead of calling `vkCmdBeginRenderPass` you call `vkCmdBeginRendering` instead. Which will look quite similar to calling the BeginRenderPass API.



```C++
vk::rendering_begin_parameters begin_params = {
    .render_area = { { 0, 0 },
                        {
                        swapchain_extent.width,
                        swapchain_extent.height,
                        }, },
    .layer_count = 1,
    .color_attachments = std::span<const vk::rendering_attachment>(
        &color_render_attachment, 1),
    .depth_attachment = depth_stencil_attachment,
    .stencil_attachment = depth_stencil_attachment,
};

vk::viewport_params viewport = {
    .x = 0.0f,
    .y = 0.0f,
    .width = static_cast<float>(swapchain_extent.width),
    .height = static_cast<float>(swapchain_extent.height),
    .min_depth = 0.0f,
    .max_depth = 1.0f,
};
current.set_viewport(
    0, 1, std::span<const vk::viewport_params>(&viewport, 1));

vk::scissor_params scissor = {
    .offset = { 0, 0 },
    .extent = swapchain_extent,
};

current.set_scissor(
    0, 1, std::span<const vk::scissor_params>(&scissor, 1));

current.begin_rendering(begin_params);

// always follow up with `.end_rendering()`
current.end_rendering();
```


## Image Layout Transitions

One of the things renderpasses do for you is automatically handling layout transitions. With dynamic rendering this is not automatically handled for you.

So, you have to perform image memory barriers before you perform the `.begin_rendering` call.

Before the rendering attachments, do the following:

```C++
swapchain_images[current_frame].memory_barrier(
    current,
    surface_properties.format.format,
    VK_IMAGE_LAYOUT_UNDEFINED,
    VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

// Because dynamic rendering does not automatically handle layout transitions
// These memory barriers set the color and depth images for the output
swapchain_depth_images[current_frame].memory_barrier(
    current,
    depth_format,
    VK_IMAGE_LAYOUT_UNDEFINED,
    VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
    VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);
```

Then after `.end_rendering` follow-up with this memory barrier call.

```C++
swapchain_images[current_frame].memory_barrier(
    current,
    surface_properties.format.format,
    VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
```

## Final Result

This is what it should look like altogether.

```C++
swapchain_images[current_frame].memory_barrier(
    current,
    surface_properties.format.format,
    VK_IMAGE_LAYOUT_UNDEFINED,
    VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

// Because dynamic rendering does not automatically handle layout transitions
// These memory barriers set the color and depth images for the output
swapchain_depth_images[current_frame].memory_barrier(
    current,
    depth_format,
    VK_IMAGE_LAYOUT_UNDEFINED,
    VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
    VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);

vk::rendering_attachment color_render_attachment = {
    .image_view = swapchain_images[current_frame].image_view(),
    .layout = vk::image_layout::color_optimal,
    .resolve_mode = vk::resolved_mode_flags::none,
    .resolve_image_view = nullptr,
    .resolve_image_layout = vk::image_layout::undefined,
    .load = vk::attachment_load::clear,
    .store = vk::attachment_store::store,
    .clear_values = clear_color
};

vk::rendering_attachment depth_stencil_attachment = {
    .image_view = swapchain_depth_images[current_frame].image_view(),
    .layout = vk::image_layout::depth_stencil_optimal,
    .resolve_mode = vk::resolved_mode_flags::none,
    .resolve_image_view = nullptr,
    .resolve_image_layout = vk::image_layout::undefined,
    .load = vk::attachment_load::clear,
    .store = vk::attachment_store::store,
    .depth_values = depth_value
};

vk::rendering_begin_parameters begin_params = {
    .render_area = { { 0, 0 },
                        {
                        swapchain_extent.width,
                        swapchain_extent.height,
                        }, },
    .layer_count = 1,
    .color_attachments = std::span<const vk::rendering_attachment>(
        &color_render_attachment, 1),
    .depth_attachment = depth_stencil_attachment,
    .stencil_attachment = depth_stencil_attachment,
};

vk::viewport_params viewport = {
    .x = 0.0f,
    .y = 0.0f,
    .width = static_cast<float>(swapchain_extent.width),
    .height = static_cast<float>(swapchain_extent.height),
    .min_depth = 0.0f,
    .max_depth = 1.0f,
};
current.set_viewport(
    0, 1, std::span<const vk::viewport_params>(&viewport, 1));

vk::scissor_params scissor = {
    .offset = { 0, 0 },
    .extent = swapchain_extent,
};

current.set_scissor(
    0, 1, std::span<const vk::scissor_params>(&scissor, 1));

current.begin_rendering(begin_params);

main_graphics_pipeline.bind(current);
vkCmdDraw(current, 3, 1, 0, 0);

current.end_rendering();

swapchain_images[current_frame].memory_barrier(
    current,
    surface_properties.format.format,
    VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
```
<!-- 
## Expectation to See on Screen -->