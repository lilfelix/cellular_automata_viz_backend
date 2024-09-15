from conan import ConanFile
from conan.tools.cmake import CMake

class CellularAutomata3DConan(ConanFile):
    name = "cellular_automata3d"
    version = "1.0"
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeDeps", "CMakeToolchain"
    requires = (
        "protobuf/5.27.0",
        "grpc/1.65.0",
    )
    exports_sources = "src/*", "CMakeLists.txt", "proto/*"

    def layout(self):
        self.folders.build = "build"
        self.folders.generators = "build"
        self.folders.source = "."

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()
