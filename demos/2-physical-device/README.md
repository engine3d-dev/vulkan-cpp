# Demo 2 -- Initialize Physical Device

In vulkan before you do any rendering, you must select a physical device. 

Vulkan was designed for maximum control and performance, which requires moving decision to the hands of the application developer.

This decision involves letting the application developer select their physical device. By allowing to select the physical device.

This also means you can select capabilities of the physical devices as to learning what operations you can do with Vulkan.

## Selecting a Physical Device

`vk::instance` in a Vulkan application is your entry point. It is your first interaction to setting up the Vulkan API.

It is required before you select a vulkan physical device, that you must HAVE a `vk::insance` or if you dont use the `vk::instance` that you have the handle initialized in your application.

## Types of Physical Devices

There are different types of physical devices:

* `vk::physical::integrated`    - the device is one embedded or tightly coupled with the host.
* `vk::physical::discrete`      - the device is typically a separated processor connected to the host via hyperlink
* `vk::physical::virtualized`   - the device typically is a virtual node in a virtualization environment
* `vk::physical::cpu`           - device is typically running on the same processor as the host
* `vk::physical::other`         - device is typically running on same procesors as the host

### Code Example to select

In vulkan-cpp, it is simple to select a physical device. This is an example on how to create a `vk::physical_device`.

```C++

// assume you've done this portion of setting the vulkan instance
vk::instance api_instance(/* set params */);

// select a physical device type
vk::physical_params params = {
    .device_type = vk::physical::discrete
};

vk::physical_device selected_device(api_instance, params);

```

After Selecting a Physical Device

To do a valid check if the handle has been created, use the `.alive()` API.

```C++

if(selected_device.alive()) {
    // print/log statements to stdout
}

```

# Thats it!

You have successfully created a vulkan physical device.

By having a physical devices, this allows you to pull in capabilities that your particular hardware GPU for information for specific operations.

This allows you to enumerate information that can tell you if your specific GPU hardware has support for certain vulkan operations

> [!NOTE]
> I have plans to make changes to this API for allowing `vk::instance` provide you API's to select `vk::physical_device`'s.
> There will be updates added, as soon I make those changes.