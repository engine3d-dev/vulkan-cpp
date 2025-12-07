# Demo 5 -- Swapchains

This part of the demo is the most crucial and quite code-heavy to setup. There are quite a bit of steps to creating the swapchain.

Vulkan does not have a concept of "default framebuffer", therefore it requires an infrastructure known as a `swapchain`. Which must be explicitly created in Vulkan.

Swapchain is essentially a queue of images that are waiting to get displayed to the screen. This is a requirement if you want you application to render anything to the screen.

General purpose of the swapchain is to synchronize the presentation of these images with the refresh rate of the screen.

In the vulkan tutorial, they mention you should check for suitability with the swapchain.

In vulkan-cpp, the only part you as the application developer need to worry about is what extensions you would like to enable for setting up the swapchain.

This demo will show how to setup the following to get a background color to the screen:
* Creating the swapchain handle
* Setting up vulkan images
* Setting up command buffers
* Setting up framebuffers
* Setting up renderpass

This demo will go over not only how to create the swapchain, but also how to get presentation working with the swapchain altogether.

As vulkan-cpp simplifies the way

# Creating the swapchain

Before we create the swapchain handles. We need to enumerate surface capabilities from the `vk::surface`.

You can query what capabilities that is allowed for you to operate when you create a surface handle.

Here is what information surface capabilities can give you:
* Max of images queued that are presentable
* Min of images queued that are presentable
* Minimum smallest valid extent (size) of a surface can be

Which are parameters the swapchain needs to expect what amount of presentable images that can be queued at a time to the swapchain.

## Set Swapchain Parameters

Before creating the swapchain handle, set the following parameters

```C++
uint32_t graphics_index = 0;
vk::swapchain_params swapchain_params = {
    .width = static_cast<uint32_t>(width),
    .height = static_cast<uint32_t>(height),
    .present_index = graphics_index,
};

```

## Constructing Swapchain Handle

Then to create the swapchain handle, it requires 4 parameters:
* logical device for creating the vulkan object
* window_surface is how you draw to the screen (essentially a canvas)
* surface_properties for specifically the kinds of surface properties that can be used to specify what this swapchain can do with in terms of handling certain amount of images.

```C++

vk::swapchain main_swapchain(logical_device, window_surface, swapchain_params, surface_properties);

```

## vk::swapchain API's

* `.presentable_images` - returns a std::span<const VkImage> of presentable images queried by the swapchain.

# Creating Vulkan images using `vk::sample_image`

In vulkan-cpp to create a vulkan image, essentially VkImage and VkImageView. vulkan-cpp's equivalent is referred to as `vk::sample_image`.

In this case because we used the `vk::swapchain` to query for presentable images. `vk::sample_image` also allows for providing a VkImage if there is one that can be provided.

## Presentable Images

After creating the swapchain handle, you can call `..presentable_images()`.

By doing the following:

```C++

std::span<const VkImage> images = main_swapchain.presentable_images();

// image_count will be referenced throughout the demos
uint32_t image_count = images.size();

```

As soon the images have been queried, you can now create the swapchain images.

```C++

// pasted from previous section to here
std::span<const VkImage> images = main_swapchain.presentable_images();
uint32_t image_count = images.size();

std::vector<vk::sample_image> swapchain_images(image_count);

```

Then we will use the surface capabilities to set the current sizes of the images to be equivalent to our window.

Our window to draw stuff is done on the surface. Which is why the swapchain images, its required to set the extent of the images.

To get the surface capabilities, use the surface_properties variable previously defined earlier on in the section.

```C++
VkExtent2D swapchain_extent = surface_properties.capabilities.currentExtent;
```

Iterate over the array of images and configure their parameters

```C++
for(uint32_t i = 0; i < swapchain_images.size(); i++) {
    vk::image_params swapchain_image_params = {
        .extent = { swapchain_extent.width, swapchain_extent.width },
        .format = surface_properties.format.format,
        .aspect = vk::image_aspect_flags::color_bit,
        .usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .mip_levels = 1,
        .layer_count = 1,
        .phsyical_memory_properties = physical_device.memory_properties(),
    };

    swapchain_images[i] = vk::sample_image(logical_device, images[i], swapchain_images_params);
}
```

* `.extent` - is used to specify dimensions of the image
* `.format` - it to specify specific `VkFormat` for the swapchain image
* `.aspect` - is specifying which image aspects are in the view (image view)
* `.mip_levels` - is used to control the LOD detail of the `vk::sample_image.
* `.layer_count` - is the number of array layers accessible to the view (useful for when we start do do cubemaps/skybox's)
* `.physical_memory_properties` - configures the specific memory types which memory heaps your specific vulkan objects belong to.

# Creating Command Buffers using `vk::command_buffer`

To setup the command buffer there are only a few things to note.

To configure the creation of `vk::command_buffer`, there are a few things to note when it comes to setting up command buffers.

Command buffers are commands that you can queue up and do something called command submission.

Where you submit commands to the queue for rendering or displaying.

## Command Levels

Command levels are to specify what kind of command buffer this is.

Which you have:
* `primary`
* `secondary`
* `protected`

To give a simpler overview, primary is the parameter we will set to tell Vulkan the command buffers we are creating are going to be our primary command buffers for submission


## Setting Command Buffer Parameters

First allocate an array of command buffers -- using image_count we previously defined.

```C++
std::vector<vk::command_buffer> swapchain_command_buffers(image_count);
```

Then configure the command buffers

```C++

for(size_t i = 0; i < swapchain_command_buffers.size(); i++) {
    vk::command_params command_params = {
        .levels = vk::command_levels::primary,
        .queue_index = swapchain_params.present_index,
        .flags = vk::command_pool_flags::reset
    };
}
```

* `.levels` - is to specify how this command buffer will be represented and used as.
* `.queue_index` - is specifying what submission this command buffer will be submitted to the specific queue.
* `.flags` - is a flag for specifying how the command buffer's pool to indicate how this command buffer will be used.

Once you do this, then the command buffers should be working.

To check if the command buffer is initialize, you can check if the handle is valid by calling the `.alive()` API.


# Creating Our First Renderpass

Renderpasses are ways to specify operations. This is used when you want to things such as deferred rendering.

Where you wait to do specific rendering operations. Rendperass is a very common approach for handling specific rendering operations

## Render Attachments

Renderpass attachments are how you specify the rendering operations within the renderpass itself. A basic example is a G-buffer (color-pass). Where you only want to render the colors for your 3D meshes.

If you were to do this in raw vulkan's C API, you would need to describe parameters for both the `VkAttachmentDescription` and the `VkAttachmentReference`.

vulkan-cpp handles mapping out these attachments altogether under the struct `vk::attachment` which contains parameters for the type of attachment you want to load for that attachment description

## How to Create RenderPass Attachments

Creating renderpass attachments is quite simple in vulkan-cpp.

In this demo, we only create a single renderpass attachment. In demo 11, it shows how to create a second attachment reference for the renderpass.

As that demo focuses on depth-buffering. So, for now we only create a single attachment.

```C++
// Creating a single attachment for the renderpass
// Getting the format specifically from the surface_properties in earlier sections of this and previous demo.
std::array<vk::attachment, 1> renderpass_attachments = {
    vk::attachment{
        .format = surface_properties.format.format,
        .layout = vk::image_layout::color_optimal,
        .samples = vk::sample_bit::count_1,
        .load = vk::attachment_load::clear,
        .store = vk::attachment_store::dont_care,
        .stencil_load = vk::attachment_load::clear,
        .stencil_store = vk::attachment_store::dont_care,
        .initial_layout = vk::image_layout::undefined,
        .final_layout = vk::image_layout::present_src_khr,
    },
};

```

* `.format` - is the specific format the image view will be using.
* `.layout` - is used to map to the specific VkAttachmentReference under the hood.
* `.samples` - specifying the number of samples of the image
* `.load` - specify how the color and depth components are specified at begin of subpass.
* `.store` - specify how color and depth are treated at the last of the subpass.
* `.stencil_load` - specify stencil data are treated at the begin of subpass
* `.stencil_store` - specify stencil data are treated at the end of subpass.
* `.initial_layout` - is the layout of the attachment resource will be when renderpass instance begins
* `.final_layout` - is layout of attachment resource when renderpass instance ends.

If you would like to know more, check the vulkan specifications. For VkAttachmentDescription and VkAttachmentReference.

Now the attachments have been specified is simple.

To construct the renderpass is fairly simple:

```C++

std::array<vk::attachment, 1> renderpass_attachments = {
    vk::attachment{
        .format = surface_properties.format.format,
        .layout = vk::image_layout::color_optimal,
        .samples = vk::sample_bit::count_1,
        .load = vk::attachment_load::clear,
        .store = vk::attachment_store::dont_care,
        .stencil_load = vk::attachment_load::clear,
        .stencil_store = vk::attachment_store::dont_care,
        .initial_layout = vk::image_layout::undefined,
        .final_layout = vk::image_layout::present_src_khr,
    },
};

// construct renderpass
vk::renderpass main_renderpass(logical_device,  renderpass_attachments);

```

That is it to creating the renderpass!

# Creating Framebuffers using `vk::framebuffer`

There are a few steps to creating the framebuffers.
* Using the image we created and get their image_views
* Specifying the renderpass
* Specify swapchain's extent (dimensions like width and height)

To instantiate the framebuffers parameters, its similar to the command buffers.

Do:

```C++
std::vector<vk::framebuffer> swapchain_framebuffers(image_count);
```

Before specifying the framebuffers, you will need to specify your image view for the specified color attachment that we specified in the renderpass attachments.

One thing to note here, renderpass attachments and the image views need to be synchronized in how they are configured during creation.

Example is, if you have a renderpass depth attachment used for the depth view. Then you pass in an image view that is used for the color attachment. This will cause validation layer error messages.

To create the image view attachments, I made the size of the array to the amount of attachments to make sure the amount of image views you created match the renderpass attachment themselves.

As each renderpass attachment is used to specify the image view that will use those attachments.

```C++

for(uint32_t i = 0; i < swapchain_framebuffers.size(); i++) {
    // setting image views based on amount of attachments
    std::array<VkImageView, renderpass_attachments.size()> image_attachments = {
        swapchain_images[i].image_view(),
    };
}
```

Now, the image views are ready to be used by the framebuffer.

It is time to set the framebuffer parameters for `vk::framebuffer`.

```C++

for(uint32_t i = 0; i < swapchain_framebuffers.size(); i++) {
    std::array<VkImageView, renderpass_attachments.size()> image_attachments = {
        swapchain_images[i].image_view(),
    };

    // Setting framebuffer params
    vk::framebuffer_params framebuffer_info = {
        .renderpass = main_renderpass,
        .views = image_view_attachments,
        .extent = swapchain_extent
    };
}
```

Finally, to construct the framebuffer is you called the class `vk::framebuffer`'s constructor to create the framebuffers.

```C++

for(uint32_t i = 0; i < swapchain_framebuffers.size(); i++) {
    std::array<VkImageView, renderpass_attachments.size()> image_attachments = {
        swapchain_images[i].image_view(),
    };

    // Setting framebuffer params
    vk::framebuffer_params framebuffer_info = {
        .renderpass = main_renderpass,
        .views = image_view_attachments,
        .extent = swapchain_extent
    };

    swapchain_framebuffers[i] = vk::framebuffer(logical_device, framebuffer_info);
}
```

#  Before we Continue

Before we continue, make sure you have either your queue handle created or `vk::device_present_queue` constructed.

Now that we are getting ready to present to the screen. We will use `vk::device_present_queue`.

## Parameters

To construct the device presentation queue, specify the queue family and index to use the VkQueue handle as.

Here is how to set the queue parameters:

```C++
vk::queue_params enumerate_present_queue{
    .family = 0,
    .index = 0,
};
```

## Constructing the `vk::device_present_queue`

For submitting commands to be display to the screen, make sure the queue to submit the work has been created.

To create the device present queue do:

```C++
vk::queue_params present_queue_params{
    .family = 0,
    .index = 0,
};

vk::device_present_queue presentation_queue(logical_device, main_swapchain, present_queue_params);
```


# Putting it together

Now, that all of the necessary vulkan-cpp objects are created.

In the mainloop, this demo will show you how to set them all up within the mainloop.

## The mainloop 

This is what the mainloop looks like:

```C++

while(!glfwWindowShouldClose(window)) {
    glfwPollEvents();
}
```

## Acquiring Next Image

Vulkan provides you an API to retrieve the next presentable image to the screen. The API for that is accessed through the `vk::device_present_queue` called `.acquire_next_image()`.

Add that to your mainloop:

```C++

while(!glfwWindowShouldClose(window)) {
    glfwPollEvents();

    // retrieving next frame to be our current
    uint32_t current_frame = presentable_queue.acquire_next_image();

}
```

## Retrieving the current command buffer

Right as you get the acquired index within your current frame. You can use this uint32_t index to retrieve your vulkan objects.

One thing to note, image_count should be value 3 as that is number of images to be prepared for presentation.

To get the current command buffer, do:

```C++

while(!glfwWindowShouldClose(window)) {
    glfwPollEvents();

    uint32_t current_frame = presentable_queue.acquire_next_image();

    // retrieving command buffer
    vk::command_buffer current = swapchain_command_buffers[current_frame];
}
```

## Recording Command Buffers

Recording command buffers in Vulkan, is necessary to tell Vulkan what is being done in a particular command.

Command buffers are sequence of instructions that allow you to write out what is needed for the GPU to perform.

These can be binding GPU resources, draw-calls to offload draw commands to the GPU's.

Command buffers a semantic of `begin` and `end` mechanism to set the command buffer to be in a recording state. As you will see below.

To record the command buffer, you call `.begin` and with the specified parameter on how the command buffer will be used during recording state.

Here is what that looks like:

```C++

while(!glfwWindowShouldClose(window)) {
    glfwPollEvents();

    uint32_t current_frame = presentable_queue.acquire_next_image();

    // retrieving command buffer
    vk::command_buffer current = swapchain_command_buffers[current_frame];

    // call begin/end to record command buffer
    current.begin(vk::command_usage::simulatneous_use_bit);
    // in here is where you'd do rendering or other operatons
    current.end();
}
```

## Renderpass Begin/End

Whenever you start recording your command buffers, calling the renderpass `begin` and `end` functions typically reside during the command buffers recording state.

When a `vk::renderpass` calls `begin` and `end`. It means to tell when to start drawing to the GPU and when to stop drawing tasks to the GPU.

During renderpass's `begin/end` function calls, this is where you primarily do your binding GPU resources and draw calls in.

Note -- With the new mentions of dynamic rendering, this does not need to be used. I will be creating a demo for using it with dynamic rendering involved.

As dynamic rendering removes the needs to creating renderpass and managing its state. For now, we do it the older way of managing the renderpass state.

Before you call renderpass begin/end, you need to specify the renderpass_begin_params as it is required.

`renderpass_begin_info` requires you to specify the framebuffer, the swapchain size (extent), color for the background of the screen, and the subpass contents.

Here is how you would set the renderpass begin/end:

```C++

std::array<float, 4> background_color = { 0.f, 0.5f, 0.5f, 1.f };

while(!glfwWindowShouldClose(window)) {
    glfwPollEvents();

    uint32_t current_frame = presentable_queue.acquire_next_image();

    // retrieving command buffer
    vk::command_buffer current = swapchain_command_buffers[current_frame];

    // call begin/end to record command buffer
    current.begin(vk::command_usage::simulatneous_use_bit);
    
    vk::renderpass_begin_params begin_params = {
        .current_command = current,
        .extent = swapchain_extent,
        .current_framebuffer = swapchain_framebuffers[current_frame],
        .color = background_color,
        .subpass = vk::subpass_contents::inline_bit
    };

    // set renderpass begin/end function calls
    main_renderpass.begin(begin_params);

    // binding GPU resources

    // draw resources

    main_renderpass.end();
    

    current.end();
}
```

## Presenting to the screen

The way vulkan-cpp has thought of submitting commands is through the API's `vk::device_present_queue` exposes to you.

Which takes in a `span<const VkCommandBuffer>` to submit.

There are two parts to presenting to the screen. You have submitting command buffers and the presentation to the frame.

Here is what the last two parts of the code looks like:

```C++

std::array<float, 4> background_color = { 0.f, 0.5f, 0.5f, 1.f };

while(!glfwWindowShouldClose(window)) {
    glfwPollEvents();

    uint32_t current_frame = presentable_queue.acquire_next_image();

    // retrieving command buffer
    vk::command_buffer current = swapchain_command_buffers[current_frame];

    current.begin(vk::command_usage::simulatneous_use_bit);
    
    vk::renderpass_begin_params begin_params = {
        .current_command = current,
        .extent = swapchain_extent,
        .current_framebuffer = swapchain_framebuffers[current_frame],
        .color = background_color,
        .subpass = vk::subpass_contents::inline_bit
    };

    main_renderpass.begin(begin_params);
    // ....
    main_renderpass.end();
    current.end();

    // submission and presentation

    std::array<const VkCommandBuffer, 1> commands = {current};
    presentation_queue.submit_async(commands);
    presentation_queue.present_frame(current_frame);
}
```


# Final Conclusion

One of the neat things about vulkan-cpp, is you can treat any of the class that I wrote as the raw handles themselves. Without having to specify a getter.

I demonstrate this when I create a `array<const VkCommandBuffer, 1>` because vk::command_buffer can be treated as a VkCommandBuffer rather then containing a separate getter for retrieving the handle.

This is the same for the logical device `vk::device`, as it is passed around in vulkan-cpp's objects expect a `VkDevice` type rather then the class type.

This allows for flexibility in what specific resources I can pass in, as long it is vulkan compliant and the resources are vulkan specific handles. Then vulkan-cpp will work.

Which allows for prototyping and easier to interopt into other projects that may the other vulkan-cpp objects.

# That is it!

Whew! We finally did it!

On that note, that is it! You should be able to see a blue screen on your window now!

# Final Result

The final result should be a window with a changed background color.

<img width="796" height="625" alt="Screenshot 2025-12-06 183239" src="https://github.com/user-attachments/assets/cf6b3ae5-bf7e-4a77-898f-0ec0c7b6f8c0" />