from conan import ConanFile
from conan.tools.cmake import CMake, cmake_layout

class CellularAutomata3DConan(ConanFile):
    name = "cellular_automata3d"
    version = "1.0"
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeToolchain", "CMakeDeps"
    requires = (
        "protobuf/5.27.0",
        "grpc/1.65.0",
        "tl-expected/1.1.0",
    )
    exports_sources = "src/*", "CMakeLists.txt", "proto/*"

    def layout(self):
        cmake_layout(self)
        self.folders.build = "."  # avoid nested build/

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()
