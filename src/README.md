These are the parts needed to build a 3D cellular automata! 
- [x] generate a random 128 bit set representing the rule
- [x] check whether central cell lives or dies in next step based on values of 2-bit <x,y,z> neighborhood vector plus central cell bit
- [x] print whether each index results in cell living or dying in next step
- [ ] ticker to move time forward
- [ ] data structure to hold state of all cells in world
- [ ] function to generate world state (3D cell grid). Takes in size of grid (x_max,y_max,z_max) and randomizes the value (0 or 1) in each cell of the grid
- [ ] function to updates cells (world state) at each step based on rule map. Takes in args: current_world_state, rule_map and outputs next_world_state

### Quick start

- install the dependencies via Conan: `conan install . --build=missing`
- run CMake to generate the build system 
    - `cmake -DCMAKE_BUILD_TYPE=Release -B conan -S .`
    - `cmake -DCMAKE_BUILD_TYPE=Debug -B conan -S .`
- build the project `cmake --build conan --parallel`

### Protobuf
[Version support](https://protobuf.dev/support/version-support/)

```bash
protoc -I=. --grpc_out=. --plugin=protoc-gen-grpc=`which grpc_cpp_plugin` sim_server.proto
protoc -I=. --cpp_out=. sim_server.proto
```

### Conan

```bash
conan install . --build=missing
```

conanfile.txt is a configuration file used by Conan, a C++ package manager. It specifies the external dependencies your project needs, such as third-party libraries (e.g., gRPC, Boost, OpenSSL).

•	Lists required packages and their versions.
•	Specifies the build type (e.g., Release, Debug).
•	Defines generators that create files for build systems (e.g., CMake).
•	Optionally includes settings like compiler versions, architectures, etc.

Also check out the [conan tutorial](https://docs.conan.io/2/tutorial/consuming_packages/build_simple_cmake_project.html)


### CMake

CMakeLists.txt is the primary configuration file for CMake, a build system generator. It specifies how to build your project, including how to compile source files, link libraries, generate executables, and handle build configurations (e.g., Debug or Release).

•	Defines project properties (name, version).
•	Specifies source files to compile.
•	Sets compiler options, include directories, and linker flags.
•	Handles custom commands (e.g., generating code from protobuf files).
•	Manages build targets, such as libraries and executables.
•	Can find and link against external libraries using find_package().