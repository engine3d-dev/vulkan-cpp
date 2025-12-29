import os
from conan import ConanFile
from conan.tools.cmake import CMakeToolchain, CMake, cmake_layout, CMakeDeps
from conan.tools.files import copy
import os

class VulkanCpp(ConanFile):
    name = "vulkan-cpp"
    version = "4.0"
    settings = "os", "compiler", "build_type", "arch"

    def build_requirements(self):
        self.tool_requires("cmake/4.1.2")
        self.tool_requires("ninja/1.13.1")
        self.tool_requires("engine3d-cmake-utils/4.0")
        self.tool_requires("cmake-modules-toolchain/1.0.3")

    def requirements(self):
        self.requires("glfw/3.4")
        self.requires("vulkan-headers/1.3.290.0")
        # self.requires("imguidocking/2.0")
        self.requires("glm/1.0.1")
        self.requires("stb/cci.20230920")
        self.requires("tinyobjloader/2.0.0-rc10")
        self.requires("boost-ext-ut/2.3.1")


    def export_sources(self):
        copy(self,"CMakeLists.txt", self.recipe_folder, self.export_sources_folder)
        copy(self,"src/CMakeLists.txt", self.recipe_folder, self.export_sources_folder)
        copy(self,"*.hpp", self.recipe_folder, self.export_sources_folder)
        copy(self,"*.cpp", self.recipe_folder, self.export_sources_folder)

    def generate(self):
        deps = CMakeDeps(self)
        deps.generate()
        tc = CMakeToolchain(self)
        tc.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        copy(self, "LICENSE", src=self.source_folder, dst=os.path.join(self.package_folder, "licenses"))
        copy(self, pattern="*.h", src=os.path.join(self.source_folder, "vulkan-cpp"), dst=os.path.join(self.package_folder, "vulkan-cpp"))
        copy(self, pattern="*.hpp", src=os.path.join(self.source_folder, "vulkan-cpp"), dst=os.path.join(self.package_folder, "vulkan-cpp"))
        copy(self, pattern="*.a", src=self.build_folder, dst=os.path.join(self.package_folder, "lib"), keep_path=False)
        copy(self, pattern="*.so", src=self.build_folder, dst=os.path.join(self.package_folder, "lib"), keep_path=False)
        copy(self, pattern="*.lib", src=self.build_folder, dst=os.path.join(self.package_folder, "lib"), keep_path=False)
        copy(self, pattern="*.dll", src=self.build_folder, dst=os.path.join(self.package_folder, "bin"), keep_path=False)
        copy(self, pattern="*.dylib", src=self.build_folder, dst=os.path.join(self.package_folder, "lib"), keep_path=False)
        cmake = CMake(self)
        cmake.install()
    
    def layout(self):
        cmake_layout(self)
    
    def package_info(self):
        self.cpp_info.set_property("cmake_target_name", "vulkan-cpp::vulkan-cpp")
        self.cpp_info.libs = ["vulkan-cpp"]
        self.cpp_info.includedirs = ['./', './vulkan-cpp']
