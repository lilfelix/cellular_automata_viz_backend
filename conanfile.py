from conan import ConanFile
from conan.tools.cmake import CMake, CMakeToolchain, CMakeDeps

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
        self.folders.build = "conan"
        self.folders.generators = "conan"
        self.folders.source = "."

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        self.copy("grpc_cpp_plugin", dst="bin", keep_path=False)

    def package_info(self):
        self.cpp_info.bindirs = ["bin"]
        self.env_info.PATH.append(self.package_folder)