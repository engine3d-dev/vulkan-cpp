# Demo 1 -- Initialization of Vulkan

In this demo, I go over how to initiate vulkan's API through vulkan-cpp's representation of `VkInstance`, referred to as `vk::instance`.

This demo will also be going over how to setup the debugging callback that is used to print to stdout of the validation layer errors that occur when you have errors in your vulkan application code.

## Extensions

There are two types of extensions that vulkan has you specified for. Instance extensions that are used to extend the `vk::instance` specifications. Which is involved for any interaction with the `vk::instance` such as `vk::physical_device` that are effected by those extensions.

Device extensions that are used to specify for `vk::device`. These extensions extend capabilities that a `vk::device` can have. Including what behavior they are allowed to be used for within other vulkan handlers.

## Validation Layers

### What validation layers are?

Since vulkan was designed around minimal driver overhead goal is to limit error checking with the API by default. Including simple mistakes such as `enumerating` physical devices and passing incorrect values.

Vulkan introduces whats called a `validation layer` which can be optionally specified. That gets hooked to the vulkan function API calls for additional operations.

Common Operations such as:

* Checking values of parameters against specifications to detect misuse.
* Tracking creation/destruction objects to find resource leaks.
* Checking thread safety by tracking threads that calls originate from.
* Logging every call and its parameter to the stdout.
* Tracing vulkan calls for profiling and replaying.

### To request for validation layers that are supported

`vk::instance` provides a function called `.validation()`, for requesting for validation layers that are supported by vulkan's current version you specify. Returns a std::span, that is a view-only contiguous buffer.

```C++
std::span<const vk::layer_properties> properties = api_instance.validation();
for(vk::layer_properties property : properties) {
    std::println("Validation Layer Name:\t\t{}", property.name);
    std::println("Validation Layer Description: {}", property.description);
}
```

### To enable validation layers

Create an array for specifying list of validation layers that get specified when `vk::instance` gets created.

```C++
std::array<const char*, 1> validation_layers = {
    "VK_LAYER_KHRONOS_validation",
};
```



## Creating Debug Callback

Before we initialize the vulkan instance. We will first initialize the debugging callback for validation layer errors. These validation layers messages are to help diagnost errors in working with vulkan and provide messages with a diagnose of what went wrong within the API's.

```C++
static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(
  VkDebugUtilsMessageSeverityFlagBitsEXT p_message_severity,
  VkDebugUtilsMessageTypeFlagsEXT p_message_type,
  const VkDebugUtilsMessengerCallbackDataEXT* p_callback_data,
  void* p_user_data) {
    // std::print("validation layer:\t\t{}\n\n", p_callback_data->pMessage); // either this or your own log print function
    return false;
}
```

### Specify debug_message_utility

This struct `debug_message_utility` is used for specify what sort of messages and what severe errors do you want this debug callback to be used to log to stdout the validation layer errors.

Including specifying what level of errors do you want this callback to be invoked.

```C++
vk::debug_message_utility debug_callback_info = {
    .severity = vk::message::verbose | vk::message::warning | vk::message::error,
    .message_type = vk::debug::general | vk::debug::validation | vk::debug::performance,
    .callback = debug_callback
};
```

## Creating vk::instance

Setting up extensions for vk::instance:

```C++
std::vector<const char*>
initialize_instance_extensions() {
    std::vector<const char*> extension_names;

    extension_names.emplace_back(VK_KHR_SURFACE_EXTENSION_NAME);

    // An additional surface extension needs to be loaded. This extension is
    // platform-specific so needs to be selected based on the platform the
    // example is going to be deployed to. Preprocessor directives are used
    // here to select the correct platform.
#ifdef VK_USE_PLATFORM_WIN32_KHR
    extension_names.emplace_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#endif
#ifdef VK_USE_PLATFORM_XLIB_KHR
    extensionNames.emplace_back(VK_KHR_XLIB_SURFACE_EXTENSION_NAME);
#endif
#ifdef VK_USE_PLATFORM_XCB_KHR
    extensionNames.emplace_back(VK_KHR_XCB_SURFACE_EXTENSION_NAME);
#endif
#ifdef VK_USE_PLATFORM_ANDROID_KHR
    extensionNames.emplace_back(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME);
#endif
#ifdef VK_USE_PLATFORM_WAYLAND_KHR
    extensionNames.emplace_back(VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME);
#endif
#ifdef VK_USE_PLATFORM_MACOS_MVK
    extensionNames.emplace_back(VK_MVK_MACOS_SURFACE_EXTENSION_NAME);
#endif
#ifdef USE_PLATFORM_NULLWS
    extensionNames.emplace_back(VK_KHR_DISPLAY_EXTENSION_NAME);
#endif
    return extension_names;
}
```

## To create a vk::instance you need to pass in `vk::application_params` and `vk::debug_message_utility`

```C++
std::vector<const char*> global_extensions = initialize_instance_extensions();
vk::debug_message_utility debug_callback_info = {
    .severity = vk::message::verbose | vk::message::warning | vk::message::error,
    .message_type = vk::debug::general | vk::debug::validation | vk::debug::performance,
    .callback = debug_callback
};

vk::application_params config = {
    .name = "vulkan instance",
    .version = vk::api_version::vk_1_3, // specify to using vulkan 1.3
    .validations = validation_layers, // .validation takes in a std::span<const char*>
    .extensions = global_extensions // .extensions also takes in std::span<const char*>
};

vk::instance api_instance(config, debug_callback_info);
```

# Done! You finally initialize vulkan's API!!

To make sure the `vk::instance` is initialize you can check if the handler is alive by checking or printing the boolean value `vk::instance` provides you. The `.alive()` function.

Or you can print the validation layers available from `vk::instance`

```C++
std::span<const vk::layer_properties> properties = api_instance.validation();
for(vk::layer_properties property : properties) {
    std::println("Validation Layer Name:\t\t{}", property.name);
    std::println("Validation Layer Description: {}", property.description);
}
```

# Final Conclusion

Whenever you create vulkan handlers follow by calling the `.destroy` function to make sure they are properly cleaned up. If you do not, this will activate the validation layer error messages for having children objects in vulkan not cleaned up.

In the demo code, I show you how to setup vk::instance including when to call the `.destroy()` function.

How you'd do it with `vk::instance`:

```C++
api_instance.destroy();
```