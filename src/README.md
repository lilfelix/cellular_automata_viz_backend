The c++ backend defined by this repo is intended to be used together with the frontend in this repo:
https://github.com/lilfelix/cellular_automata_viz_frontend

These are the parts needed to visualize a 3D cellular automata:
- generate a random 128 bit set representing the rule
- check whether central cell lives or dies in next step based on values of 2-bit <x,y,z> neighborhood vector plus central cell bit
- print whether each index results in cell living or dying in next step
- ticker to move time forward
- data structure to hold state of all cells in world
- function to generate world state (3D cell grid). Takes in size of grid (x_max,y_max,z_max) and randomizes the value (0 or 1) in each cell of the grid
- function to updates cells (world state) at each step based on rule map. Takes in args: current_world_state, rule_map and outputs next_world_state

### Quick start

- initialize grpc git submodule (built from source)
    - `git clone --recurse-submodules https://github.com/lilfelix/cellular_automata_viz_backend.git`
    - Or if the repo is already cloned: `git submodule update --init --recursive`
- install the dependencies via Conan: 
```
conan install . --output-folder=out --build=missing --profile:host=default
```
- run CMake to generate the build system `cmake -DCMAKE_BUILD_TYPE=Release -B build -S .`
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
The compiling of .proto to C++ source files is handled by CMake. See CMakeLists.txt.  
It can also be done manually:

For release build:
```bash
protoc -I=$PWD/proto \
       --cpp_out=$PWD/out/build/Release/generated \
       --grpc_out=$PWD/out/build/Release/generated \
       --plugin=protoc-gen-grpc=$(find . -type f -name grpc_cpp_plugin | head -n1) \
       $PWD/proto/sim_server.proto
```

For debug build:
```bash
protoc -I=$PWD/proto \
       --cpp_out=$PWD/out/build/Debug/generated \
       --grpc_out=$PWD/out/build/Debug/generated \
       --plugin=protoc-gen-grpc=$(find . -type f -name grpc_cpp_plugin | head -n1) \
       $PWD/proto/sim_server.proto
```

[Version support](https://protobuf.dev/support/version-support/)

### gRPC as external dependency
Easiest way I've found to install grpc with the grpc_cpp_plugin is to do it from source