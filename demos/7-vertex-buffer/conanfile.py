from conan import ConanFile
from conan.tools.cmake import CMake, cmake_layout

class Demo(ConanFile):
    name = "game-demo"
    version = "1.0"
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeDeps", "CMakeToolchain"
    export_source = "CMakeLists.txt", "application.cpp"

    # Putting all of your build-related dependencies here
    def build_requirements(self):
        self.tool_requires("cmake/[^4.0.0]")
        self.tool_requires("ninja/[^1.3.0]")
        self.tool_requires("engine3d-cmake-utils/4.0")

    # Putting all of your packages here
    # To build engine3d/1.0 locally do the following:
    # conan create . --name=engine3d --version=0.1.0 --user=local --channel=12345
    def requirements(self):
        self.requires("glfw/3.4")
        self.requires("glm/1.0.1")
        self.requires("stb/cci.20230920")
        self.requires("tinyobjloader/2.0.0-rc10")
        self.requires("vulkan-cpp/5.0")

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()
    
    def layout(self):
        cmake_layout(self)