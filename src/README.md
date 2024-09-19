These are the parts needed to build a 3D cellular automata! 
- [x] generate a random 128 bit set representing the rule
- [x] check whether central cell lives or dies in next step based on values of 2-bit <x,y,z> neighborhood vector plus central cell bit
- [x] print whether each index results in cell living or dying in next step
- [ ] ticker to move time forward
- [ ] data structure to hold state of all cells in world
- [ ] function to generate world state (3D cell grid). Takes in size of grid (x_max,y_max,z_max) and randomizes the value (0 or 1) in each cell of the grid
- [ ] function to updates cells (world state) at each step based on rule map. Takes in args: current_world_state, rule_map and outputs next_world_state

### Quick start

- initialize grpc git submodule (built from source)
    - `git clone --recurse-submodules https://github.com/lilfelix/cell_automata_project.git`
    - Or if the repo is already cloned: `git submodule update --init --recursive`
- install the dependencies via Conan: `conan install . --build=missing --output-folder=<build or debug>`
- run CMake to generate the build system 
    - `cmake -DCMAKE_BUILD_TYPE=Release -B build -S .`
    - `cmake -DCMAKE_BUILD_TYPE=Debug -B build -S .`
- build the project `cmake --build build --parallel 6`

### grpCurl

`grpcurl -plaintext localhost:50051 list`
`grpcurl -d '{"dimensions":{"y_max":"10","z_max":"10","x_max":"10"}}' -plaintext localhost:50051 sim_server.StateService/InitWorldState`

```bash
RULE=$(echo -n 0123456789abcdef0123456789abcdef | xxd -r -p | base64); \       
grpcurl -d '{"world_state_id":"0", "rule":"'$RULE'"}' -plaintext localhost:50051 \
sim_server.StateService/StepWorldStateForward
```

### Protobuf
[Version support](https://protobuf.dev/support/version-support/)
[Good blogpost on gRPC with CMake](https://www.f-ax.de/dev/2020/11/08/grpc-plugin-cmake-support.html)

For release build:
```bash
protoc -I=$PWD/proto \
       --cpp_out=$PWD/build/generated \
       --grpc_out=$PWD/build/generated \
       --plugin=protoc-gen-grpc=$(find . -type f -name grpc_cpp_plugin | head -n1) \
       $PWD/proto/sim_server.proto
```

For debug build:
```bash
protoc -I=$PWD/proto \
       --cpp_out=$PWD/build/debug/generated \
       --grpc_out=$PWD/build/debug/generated \
       --plugin=protoc-gen-grpc=$(find . -type f -name grpc_cpp_plugin | head -n1) \
       $PWD/proto/sim_server.proto
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