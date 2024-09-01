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

# Find the grpc_cpp_plugin executable
find_program(GRPC_CPP_PLUGIN grpc_cpp_plugin)

if(NOT GRPC_CPP_PLUGIN)
    message(FATAL_ERROR "grpc_cpp_plugin not found. Ensure gRPC is installed correctly.")
endif()

# Proto files directory
set(PROTO_DIR ${CMAKE_SOURCE_DIR}/proto)

# Source files
file(GLOB_RECURSE SRCS ${CMAKE_SOURCE_DIR}/src/*.cpp)
file(GLOB_RECURSE PROTO_FILES ${PROTO_DIR}/*.proto)

# Generate gRPC and Protobuf sources
foreach(PROTO_FILE ${PROTO_FILES})
    get_filename_component(PROTO_NAME ${PROTO_FILE} NAME_WE)
    set(GRPC_SRC ${CMAKE_BINARY_DIR}/generated/${PROTO_NAME}.pb.cc)
    set(GRPC_HDR ${CMAKE_BINARY_DIR}/generated/${PROTO_NAME}.pb.h)
    set(GRPC_GRPC_SRC ${CMAKE_BINARY_DIR}/generated/${PROTO_NAME}.grpc.pb.cc)
    set(GRPC_GRPC_HDR ${CMAKE_BINARY_DIR}/generated/${PROTO_NAME}.grpc.pb.h)

    add_custom_command(
        OUTPUT ${GRPC_SRC} ${GRPC_HDR} ${GRPC_GRPC_SRC} ${GRPC_GRPC_HDR}
        COMMAND protobuf::protoc
        ARGS --grpc_out=${CMAKE_BINARY_DIR}/generated --cpp_out=${CMAKE_BINARY_DIR}/generated
             --plugin=protoc-gen-grpc=${GRPC_CPP_PLUGIN} -I${PROTO_DIR} ${PROTO_FILE}
        DEPENDS ${PROTO_FILE}
        COMMENT "Running gRPC C++ code generation on ${PROTO_FILE}"
    )

    list(APPEND PROTO_SRCS ${GRPC_SRC} ${GRPC_GRPC_SRC})
    list(APPEND PROTO_HDRS ${GRPC_HDR} ${GRPC_GRPC_HDR})
endforeach()

# Add include directories
include_directories(${CMAKE_BINARY_DIR}/generated ${CMAKE_SOURCE_DIR}/include)

# Add executable target
add_executable(MyExecutable ${SRCS} ${PROTO_SRCS} ${PROTO_HDRS})

# Link libraries
target_link_libraries(MyExecutable PRIVATE ${CONAN_LIBS})

# Install target
install(TARGETS MyExecutable DESTINATION bin)