# Demo 6 -- Setting up Graphics Pipeline

This demo will show how to set up the `VkPipeline` handle. Which is the graphics pipeline.

This is one of the fundamental primitives to doing rendering operations with Vulkan.

The vulkan tutorial website goes over more in-depth of the responsibility of the graphics pipeline. In this demo will briefly go over the parts to setting up a `vk::pipeline` with vulkan-cpp.

# Process of the Graphics Pipeline

In this section will briefly begin to introduce the graphics pipeline using the vulkan-cpp abstraction, `vk::pipeline` class.

The sequence diagram shows stages for the graphics pipeline:

> [!NOTE]
> Credit to the vulkan web-page that can be seen used in this [link](https://vulkan-tutorial.com/Drawing_a_triangle/Graphics_pipeline_basics/Introduction)

<!-- ![alt text](vulkan_simplified_pipeline.svg) -->
![vulkan_simplified_pipeline](https://github.com/user-attachments/assets/978c738c-9d58-4c69-866a-12471475254e)

### Input Assembler
* collects raw verticse from buffers you specify as inputs
* May also use index buffers to repeat certain elements without duplicating vertex data itself.

### Vertex Shader
* Executes on a per-vertex basis
* Generally applies transformation to turn vertex position from model to screen spaces.
* Passes per-vertex through the graphics pipeline sequence.

This image shows the difference between a clip coordinate. Clip coordinate a 4D vector from the vertex shader that gets subsequently turned into normalized device coordinate by dividing the whole vector by its last component.

These normalized device coordinates are `homogeneous coordinates` that map the framebuffer to a [-1,1] by [-1,1] coordinate system as shown below:

![normalized_device_coordinates](https://github.com/user-attachments/assets/a23de82b-0cb0-4fe1-b329-88cc9180bd49)

### Tessellation Shader
* Used for subdividing geometry based on rules to increase mesh quality.
* Often used to make surfaces like brick walls and staircases look less then flag when nearby.

### Geometry Shader
* Executed on every primitive (triangle, line, point) and could discard it or ouput more then what was given to it.

### Rasterization
* This stage discretizes the primitives into fragments
* These are pixel elements that are filled in the framebuffer.
* As shown, fragments that are outside screen-space are discarded and the attribute outputted by the vertex shader are interpolated across fragments
* Fragments that are behind other primitive fragments are discarded here, due to depth testing.

### Fragment Shader
* Executed for every fragment that survives and determined which framebuffers the fragments are written to with which color and depth values.
* Which that is done because it interpolates data from the vertex shader.
* Which includes texture coordinates and normals for lighting

### Color Blending
* Applies the operations to mix different fragments that map to the same pixel within the framebuffer.
* Fragments can overwrite each other, adding up/mixed based upon transparency.

# Graphics Pipeline Concepts

## How to use `vk::pipeline`?

Now, in this section will show you how to setup the graphics pipeline using vulkan-cpp parameters to construct the `vk::pipeline` object.

### What are optional parameters to use?
Noting the geometry and tesselation shader parameters are **optional** to be set if you do not plan to do operations that require you have computations involving the topology of your geometry.

Whereas the **fragment** shaders can be disabled in situations such as shadow-mapping generations.

# Constructing the actual Graphics Pipeline

> [!NOTE]
> `vk::pipeline` implementation is still in-progress as I have not exposed the other stages quite yet. That will be worked in exposing soon.

Previously in demo 5 focused in setting up the swapchain, command buffers, images, and framebuffers to ensure we can at least be able to change the background color to a different color.

To indicate the changes we have made are working with no additional errors from the validation layers.

Goal for this demo, where we setup the graphics pipeline is to finally get us a triangle to render to the screen.

<img width="275" height="215" alt="triangle_coordinates_colors" src="https://github.com/user-attachments/assets/2a80ab09-669d-4298-927e-e8f6e0a545a1" />

## Loading Shaders using `vk::shader_resource`

The graphics pipeline handle requires there to be a shader to be loaded with. This will load into the shader module handles using the `vk::shader_resource` abstraction.

The shader resource class helps with loading in the shaders and associating the stage and loading in N amount of handles with the shaders specified.

You should be able to copy shaders, `test.vert` and `test.frag` glsl shaders into your project location. For the demos, which are contained in a folder called `shader_samples/sample1/`.

Here are what each of the shaders contain:

## Vertex Shader (Update)

In the next and incoming demos, we would typically have these vertices handled via vertex attributes. For getting this triangle to render. The next demo will cover vertex buffers that will show how to get vertex attributes working.

For now, the vertices will be hardcoded.

```glsl
#version 450

vec2 positions[3] = vec2[](
    vec2(0.0, -0.5),
    vec2(0.5, 0.5),
    vec2(-0.5, 0.5)
);

void main() {
    gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
}

```

## Fragment Shader

The fragment shader is what processes the vertices of your 3D geometry, in this case the triangle for this demo.

Here is what the current fragment shader code looks like:
```glsl

#version 450

// Add
layout(location = 0) out vec3 fragColor;

// ...

// Add
vec3 colors[3] = vec3[](
    vec3(1.0, 0.0, 0.0),
    vec3(0.0, 1.0, 0.0),
    vec3(0.0, 0.0, 1.0)
);

void main() {
    // ...

    // Add
    fragColor = colors[gl_VertexIndex];
}
```

In that same fragment shader, add the following code:

```glsl

#version 450

layout(location = 0) in vec3 fragColor;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = vec4(fragColor, 1.0);
}
```

The `main` function is called for every fragment just like the vertex shader. Colors within glsl are 4-component vectors with R, G, B, and Alpha channels. Within from [0, 1].

Because fragment shaders do not have builtin functions such as `gl_Position` in the vertex shader. You need to supply your own output variable for each framebuffer.

Where `layout(location = 0)` modifier specifies index of the framebuffer. The color gradient is written to `outColor` that is linked to the first (and only) framebuffer at index 0.

## Compiling Shaders


If you have used OpenGL, it would provide API's to automatically compile your shaders to `.spv`, BUT in Vulkan this responsibility will be on you. the developer. Therefore, before you can load in your shaders. You are going to need to compile your shaders to .spv

For context, `.spv` is the binary format of the compiled-down shader code.

```bash
glslc.exe shader_samples/sample1/test.vert -o shader_samples/sample1/test.vert.spv
glslc.exe shader_samples/sample1/test.frag -o shader_samples/sample1/test.frag.spv
```

<!-- Now, that you have setup the glsl shaders to specify what data it should expect to render the triangle on the GPU. We will now continue to building the graphics pipeline. -->

## Setting up `vk::shader_source`

<!-- In vulkan-cpp you only need to specify the .spv shader sources and their stages.

Here is how you specify the shader sources: -->

Now, the shaders are compiled to `.spv`. In this section, I am going to show how to load the spirv compiled binaries.

Each `vk::shader_source` contains the location to the individual binaries along with the stage of the shader they are used for.

This is important to distinction to know how these shaders are going to be utilized.

```C++
std::array<vk::shader_source, 2> shader_sources = {
    vk::shader_source{
        .filename = "shader_samples/sample1/test.vert.spv",
        .stage = vk::shader_stage::vertex,
    },
    vk::shader_source{
        .filename = "shader_samples/sample1/test.frag.spv",
        .stage = vk::shader_stage::fragment,
    }
};
```

## Initializing the `vk::shader_resource`

Now, the shader sources are already configured. Now use pass that into `vk::shader_resource_info`.

For this demo, we will now initialize the `vk::shader_resource`. Firstly passing the `shader_info` parameters to the constructor. Followed by passing the logical device as the first parameter.

```C++
vk::shader_resource_info shader_info = {
    .sources = shader_sources,
};
vk::shader_resource geometry_resource(logical_device, shader_info);
```

### Setting the vertex attributes

The `vk::shader_resource` does allow you to set the vertex attributes as the following below. Though for the triangle, it will be brought up in the next few demos.

If you have no vertex attributes that needs to be set. Then you can completely ignore invoking this API.

This is what the API looks like. In later demos, we will cover how you may setup the vertex attributes for `vk::shader_resource`.

```C++
geoemetry_resource.vertex_attributes(...);
```

## Configuring the Graphics Pipeline

In vulkan-cpp, I tried to consider ways of approaching the design for the graphics pipeline for simplifying steps needed to configure the graphics pipeline.

Parmaters to configure `vk::pipeline` with:
* `VkRenderPass`.
* `span<const VkShaderModule>`
* `span<const VertexAttributeDescription>`
* `span<const VertexAttributeBindingDescription>`

Which are provided to you by the `vk::shader_resource` class.

### Parameters for `vk::pipeline`

When configuring the graphics pipeline, there are an extensive amount of parameters. Which are set to default values that can be changed.

These are the list of parameters that need to be set for the graphics pipeline set to default values.

```C++
struct pipeline_params {
    VkRenderPass renderpass = nullptr;
    std::span<const shader_handle> shader_modules{};
    std::span<const VkVertexInputAttributeDescription>
        vertex_attributes;
    std::span<const VkVertexInputBindingDescription>
        vertex_bind_attributes;
    std::span<VkDescriptorSetLayout> descriptor_layouts;

    input_assembly_state input_assembly;
    viewport_state viewport;
    rasterization_state rasterization;
    multisample_state multisample;
    color_blend_state color_blend;
    bool depth_stencil_enabled = false;
    depth_stencil_state depth_stencil;
    std::span<dynamic_state> dynamic_states = {};
};
```

For this demo, we are only going to be setting the color blend states and the dynamic states.

### Color Blend States

These parameters tell the rasterization (when enabled) how to access any color attachments during rendering operations.

`vk::color_blend_attachment_state` defaults `.blend_enabled` to true. This can be used to enable alpha blending via transparency.

### Dynamic States

Enabling parameters `vk::dynamic_state::viewport` and `vk::dynamic_state::scissor`, this allows the graphics pipeline handles to handle dynamically changing when resizing the window and updating the graphics pipeline handles.


```C++
// Sets up the default color blend attachment state
std::array<vk::color_blend_attachment_state, 1> color_blend_attachments = {
    vk::color_blend_attachment_state{},
};

// Setting up graphics pipeline to having a dynamic state
// So we are not needing to invalidate and have the driver be responsible for that.
std::array<vk::dynamic_state, 2> dynamic_states = {
    vk::dynamic_state::viewport, vk::dynamic_state::scissor
};

vk::pipeline_params pipeline_params = {
    .renderpass = main_renderpass,
    .shader_modules = geometry_resource.handles(),
    .vertex_attributes = geometry_resource.vertex_attributes(),
    .vertex_bind_attributes = geometry_resource.vertex_bind_attributes(),
    .color_blend = {
        .attachments = color_blend_attachments,
    },
    .dynamic_states = dynamic_states,
};
vk::pipeline main_graphics_pipeline(logical_device, pipeline_params);
```

# Graphics Pipeline Instantiated

Now the graphics pipeline has been initialized and to check you can print out using the `.alive` API to see if the handle was initialized successfully. As it should return true if it initialized successfully.

Now, let us go ahead and start using the `vk::pipeline` API's, which includes calling the bind and draw call API to render our first triangle

```C++

while (!glfwWindowShouldClose(window)) {
    // ....
    uint32_t current_frame = presentation_queue.acquire_next_image();
    vk::command_buffer current = swapchain_command_buffers[current_frame];

    // ...
    // current.begin(...)
    // main_renderpass.begin(current, ...)

    // NEW: Using graphics pipeline bind API to bind resource to the command buffer
    main_graphics_pipeline.bind(current);

    // NEW: Add draw call here
    // vkCmdDraw(VkCommandBuffer, vertexCount, instanceCount, firstVertex, firstInstance)
    vkCmdDraw(current, 3, 1, 0, 0);

    // main_renderpass.end(current)
    // current.end();
    // ...
}

```

# Post Cleanup

After creating the graphics pipeline, be sure to invoke the `.destroy` API to ensure the handles are destroyed properly.

> [!NOTE] This will be replaced to fully using RAII to automate cleanup for your objects.

```C++

while (!glfwWindowShouldClose(window)) {
    // ....
}

// make sure to destroy child objects of Vulkan before the actual logical device.
main_graphics_pipeline.destruct();
geometry_resource.destruct();

logical_device.destruct();
```

# Final Result

If you followed this demo tutorial correctly with no given issue.

You should be able to see this triangle get rendered:

<img width="790" height="617" alt="Screenshot 2025-12-06 201645" src="https://github.com/user-attachments/assets/ae89b34c-76d3-436d-8f09-91cb575ed44d" />
