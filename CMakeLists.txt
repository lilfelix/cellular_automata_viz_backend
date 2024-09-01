cmake_minimum_required(VERSION 3.15)
project(CellularAutomata3D VERSION 1.0 LANGUAGES CXX)


# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set the output directories for built targets
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Include Conan-generated toolchain file for dependencies
include(${CMAKE_BINARY_DIR}/conan_toolchain.cmake)

# Find the protoc and grpc_cpp_plugin executables
find_program(PROTOC_EXECUTABLE protoc)
find_program(GRPC_CPP_PLUGIN_EXECUTABLE grpc_cpp_plugin)

if(NOT PROTOC_EXECUTABLE)
    message(FATAL_ERROR "protoc not found. Ensure Protobuf is installed correctly.")
endif()

if(NOT GRPC_CPP_PLUGIN_EXECUTABLE)
    message(FATAL_ERROR "grpc_cpp_plugin not found. Ensure gRPC is installed correctly.")
endif()

# Proto files directory
set(PROTO_DIR ${CMAKE_SOURCE_DIR}/proto)
set(GENERATED_DIR ${CMAKE_BINARY_DIR}/generated)

# Source files
file(GLOB_RECURSE SRCS ${CMAKE_SOURCE_DIR}/src/*.cpp)
file(GLOB_RECURSE PROTO_FILES ${PROTO_DIR}/*.proto)

# Make sure the generated directory exists
file(MAKE_DIRECTORY ${GENERATED_DIR})

# Generate gRPC and Protobuf sources
foreach(PROTO_FILE ${PROTO_FILES})
    get_filename_component(PROTO_NAME ${PROTO_FILE} NAME_WE)
    
    set(GRPC_SRC ${GENERATED_DIR}/${PROTO_NAME}.pb.cc)
    set(GRPC_HDR ${GENERATED_DIR}/${PROTO_NAME}.pb.h)
    set(GRPC_GRPC_SRC ${GENERATED_DIR}/${PROTO_NAME}.grpc.pb.cc)
    set(GRPC_GRPC_HDR ${GENERATED_DIR}/${PROTO_NAME}.grpc.pb.h)

    add_custom_command(
        OUTPUT ${GRPC_SRC} ${GRPC_HDR} ${GRPC_GRPC_SRC} ${GRPC_GRPC_HDR}
        COMMAND ${PROTOC_EXECUTABLE}
        ARGS --grpc_out=${GENERATED_DIR} --cpp_out=${GENERATED_DIR}
             --plugin=protoc-gen-grpc=${GRPC_CPP_PLUGIN_EXECUTABLE} -I${PROTO_DIR} ${PROTO_FILE}
        DEPENDS ${PROTO_FILE}
        COMMENT "Running gRPC C++ code generation on ${PROTO_FILE}"
    )

    list(APPEND PROTO_SRCS ${GRPC_SRC} ${GRPC_GRPC_SRC})
    list(APPEND PROTO_HDRS ${GRPC_HDR} ${GRPC_GRPC_HDR})
endforeach()

# Add include directories
include_directories(${GENERATED_DIR} ${CMAKE_SOURCE_DIR}/include)

# Add executable target
add_executable(CellularAutomata3D ${SRCS} ${PROTO_SRCS} ${PROTO_HDRS})

# Link libraries
target_link_libraries(CellularAutomata3D PRIVATE ${CONAN_LIBS})

# Install target
install(TARGETS CellularAutomata3D DESTINATION bin)