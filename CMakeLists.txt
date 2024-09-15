cmake_minimum_required(VERSION 3.15)
project(CellularAutomata3D VERSION 1.0 LANGUAGES CXX)

# https://github.com/protocolbuffers/protobuf/issues/12185#issuecomment-1594685860
set(ABSL_ENABLE_INSTALL ON)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# Set the architecture for Apple Silicon
set(CMAKE_OSX_ARCHITECTURES "arm64")  # or "x86_64", or "arm64;x86_64" for universal
set(CMAKE_BUILD_PARALLEL_LEVEL 6) # use "nproc" to check number of cores available

# Add the gRPC submodule to CMake build
add_subdirectory(external/grpc)

# Set the output directories for built targets
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Source files
file(GLOB_RECURSE SRCS ${CMAKE_SOURCE_DIR}/src/*.cpp)
file(GLOB_RECURSE PROTO_FILES ${CMAKE_SOURCE_DIR}/proto/*.proto)

# Ensure source files are found
if(NOT SRCS)
    message(FATAL_ERROR "No source files found in src directory.")
endif()

# Proto files directory
set(GENERATED_DIR ${CMAKE_BINARY_DIR}/generated)
file(MAKE_DIRECTORY ${GENERATED_DIR})

# Find the grpc_cpp_plugin executable
set(_GRPC_CPP_PLUGIN_EXECUTABLE "${CMAKE_SOURCE_DIR}/external/grpc/cmake/build/grpc_cpp_plugin")

# Ensure the executable is found
if(NOT EXISTS ${_GRPC_CPP_PLUGIN_EXECUTABLE})
    message(FATAL_ERROR "grpc_cpp_plugin not found: ${_GRPC_CPP_PLUGIN_EXECUTABLE}")
else()
    message(STATUS "Using grpc_cpp_plugin at: ${_GRPC_CPP_PLUGIN_EXECUTABLE}")
endif()

# Generate gRPC and Protobuf sources
foreach(PROTO_FILE ${PROTO_FILES})
    get_filename_component(PROTO_NAME ${PROTO_FILE} NAME_WE)
    
    set(GRPC_SRC ${GENERATED_DIR}/${PROTO_NAME}.pb.cc)
    set(GRPC_HDR ${GENERATED_DIR}/${PROTO_NAME}.pb.h)
    set(GRPC_GRPC_SRC ${GENERATED_DIR}/${PROTO_NAME}.grpc.pb.cc)
    set(GRPC_GRPC_HDR ${GENERATED_DIR}/${PROTO_NAME}.grpc.pb.h)

    add_custom_command(
        OUTPUT ${GRPC_SRC} ${GRPC_HDR} ${GRPC_GRPC_SRC} ${GRPC_GRPC_HDR}
        COMMAND protoc
        ARGS --grpc_out=${GENERATED_DIR} --cpp_out=${GENERATED_DIR}
        --plugin=protoc-gen-grpc=${_GRPC_CPP_PLUGIN_EXECUTABLE} -I${CMAKE_SOURCE_DIR}/proto ${PROTO_FILE}
        DEPENDS ${PROTO_FILE}
        COMMENT "Running gRPC C++ code generation on ${PROTO_FILE}"
    )

    list(APPEND SRCS ${GRPC_SRC} ${GRPC_GRPC_SRC})
endforeach()

# Add include directories for generated files and Conan packages
include_directories(${GENERATED_DIR})
include_directories(${CMAKE_BINARY_DIR}/conan/include)
include_directories(${CMAKE_SOURCE_DIR}/include)

# Add executable target
add_executable(CellularAutomata3D ${SRCS})

# Link libraries provided by Conan
target_link_libraries(CellularAutomata3D PRIVATE ${CONAN_LIBS})

# Link gRPC lib built from source
target_link_libraries(CellularAutomata3D PRIVATE grpc++)

# Link against gRPC::grpc++_reflection
target_link_libraries(CellularAutomata3D PRIVATE grpc++_reflection)

# Install target
install(TARGETS CellularAutomata3D DESTINATION bin)