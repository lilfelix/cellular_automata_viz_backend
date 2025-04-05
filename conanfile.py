from conan import ConanFile
from conan.tools.cmake import CMake

class CellularAutomata3DConan(ConanFile):
    name = "cellular_automata3d"
    version = "1.0"
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeToolchain", "CMakeDeps"

    requires = (
        "protobuf/5.27.0",
        "tl-expected/1.1.0",
        "catch2/3.5.2",
    )
    exports_sources = "src/*", "CMakeLists.txt", "proto/*"

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()
