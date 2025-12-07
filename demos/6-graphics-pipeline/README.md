# Demo 6 -- Setting up Graphics Pipeline

This demo will show how to set up the `VkPipeline` handle. Which is the graphics pipeline.

This is one of the fundamental primitives to doing rendering operations with Vulkan.

The vulkan tutorial website goes over more in-depth of the responsibility of the graphics pipeline. In this demo will briefly go over the parts to setting up a `vk::pipeline` with vulkan-cpp.

# Process of the Graphics Pipeline

This section will briefly touch on the graphics pipeline and the process behind it.

This is the following graphics sequence diagram:

![NOTE]
> Credit to the vulkan web-page that can be seen used in this [link](https://vulkan-tutorial.com/Drawing_a_triangle/Graphics_pipeline_basics/Introduction)

![alt text](vulkan_simplified_pipeline.svg)

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

## Programmable Stages

Diagrams marked green are process in the graphics pipeline you can upload your code to modify its parameters.

Which involve:
* Offload code to the GPU
* Sending uniform data such as (textures, lighting, ray tracing, etc)
* Process data on multiple GPU cores simultaneously to process objects in parallel.

## Optional Stages

These are stages that can be entirely optional. Optional because they are not required for a variety of different reasons.

Stages involved are:
* `geometry` and `tessellation` can be disabled if you plan to deal with simpler geometry.
* `fragment shaders` can be disabled, if you need depth values only. Useful for `shadow-mapping` generation.

# Building the Graphics Pipeline

![NOTE]
> `vk::pipeline` implementation is still in-progress as I have not exposed the other stages quite yet. That will be worked in exposing soon.

Previously in demo 5. We focused on how to setup the swapchain, command buffers, images, and framebuffers to make sure we can send some data to the screen. Which was changing the background color to a specific color.

In this demo, we are going to try and get a triangle drawn to the screen. Which wont focus too much in getting transforms to work, we will focus in specifying the raw vertices to the GPU to draw the triangle.

Since the triangle, we will not be applying any transformations. We will specify the positions of the three vertices directly as normalized device coordinates to create the following shape:

<img width="275" height="215" alt="triangle_coordinates_colors" src="https://github.com/user-attachments/assets/2a80ab09-669d-4298-927e-e8f6e0a545a1" />

# Loading Shaders

In Vulkan before building the graphics pipeline. You need to specify both the source to the shader compiled code and the stage this shader is being compiled as.

Since Vulkan does not have a runtime shader compiler that comes with it, you have to compile your glsl shader to spirv binary format.

In this case, you'd compile `shader_samples/sample1/test.vert` to `shader_samples/sample1/test.vert.spv`. Same needs to happen compiling `sample1/test.frag` to `shader_samples/sample1/test.frag.spv`

## Vertex Shader

In the next demo, we will actually use a vertex buffer to store these vertices. For now we will type in the raw vertices in the vertex shader. If you
look at the `shader_samples/sample1/test.vert` shader. As shown below:

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

In the fragment shader is what forms using the position vertices you pass from the vertex shader. Then fills that area onto the screen with fragments.

The fragment shader is involved in executing on those fragments to produce color and depth for the framebuffer (or framebuffers).

A simple fragment shader outputs color gradient. Adding this into your vertex shader.

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

In the fragment shader, add the following code:

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

I have mentioned that previously vulkan does not know what to do with your shader source code and expects only the final binary blob that is after compiling the source code.

To compile the shaders to SPIRV-bytecode, you should already have glsl install and be able to reference similarily to your platform-specific file paths.

```bash
glslc.exe shader_samples/sample1/test.vert -o shader_samples/sample1/test.vert.spv
glslc.exe shader_samples/sample1/test.frag -o shader_samples/sample1/test.frag.spv
```

## Finalized Setting up Shaders

Now, that you have setup the glsl shaders to specify what data it should expect to render the triangle on the GPU. We will now continue to building the graphics pipeline.

## Loading Shader Modules

In vulkan-cpp you only need to specify the .spv shader sources and their stages.

Here is how you specify the shader sources:

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

## Creating `vk::shader_resource`

Before you create the shader_resource, you need to specify the `logical device` and the `vk::shader_resource_info`.

The shader resource information are parameters required to be specified for the shader resource such as vertex attributes.

Here is how to specify the parameters:

```C++
vk::shader_resource_info shader_info = {
    .sources = shader_sources,
};
vk::shader_resource geometry_resource(logical_device, shader_info);
```

### Setting the vertex attributes

The `vk::shader_resource` does allow you to set the vertex attributes as the following below. Though for the triangle, it will be brought up in the next few demos.

```C++
geoemetry_resource.vertex_attributes(...);
```

## Creating the Graphics Pipeline

In vulkan-cpp, it is quite simple to create a graphics pipeline. vulkan-cpp was designed to be quite simple in the workflow for specifying work through Vulkan. This includes specifying the graphics pipeline.

In the `vk::shader_resource` implementation is supposed to be wrapper for creating the raw `VkShaderModule` handles needed to be specified by the vulkan-cpp graphics pipeline. Specifically `vk::pipeline`.

Now, that you have created the shader sources and loaded them into the vulkan shader handles. Here is how you setup the graphics pipeline.

The `vk::shader_resource` gives you access to the handles created.

These are handles returned in that order:

* span<const VkShaderModule>
* span<const VertexAttributeDescription>
* span<const VertexAttributeBindingDescription>

```C++
vk::pipeline_settings pipeline_configuration = {
    .renderpass = main_renderpass,
    .shader_modules = geometry_resource.handles(),
    .vertex_attributes = geometry_resource.vertex_attributes(),
    .vertex_bind_attributes = geometry_resource.vertex_bind_attributes()
};
vk::pipeline main_graphics_pipeline(logical_device, pipeline_configuration);
```

# Finally Created the Graphics Pipeline

As soon the graphics pipeline as been created using `vk::pipeline`. This demo will introduce a few of the `vk::pipeline` API's.

`vk::pipeline` gives you a `.bind(const VkCommandBuffer&)` API. This just means making a particular resource available to be used by the GPU. This is a common operation done in Computer Graphics.

Here is the additional code to add in the mainloop:

```C++

while (!glfwWindowShouldClose(window)) {
    // ....
    uint32_t current_frame = presentation_queue.acquire_next_image();
    vk::command_buffer current = swapchain_command_buffers[current_frame];

    // Binding to make resources available and accessible by the GPU
    // Add
    main_graphics_pipeline.bind(current);

    // Draw call to render to the screen
    // Add
    vkCmdDraw(current, 3, 1, 0, 0);

    // ....
}

```

# Final Result

As soon those two lines of code have been added. This is what the final result should look like:

<img width="790" height="617" alt="Screenshot 2025-12-06 201645" src="https://github.com/user-attachments/assets/ae89b34c-76d3-436d-8f09-91cb575ed44d" />
