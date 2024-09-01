# Compiler and flags
CXX := clang++
CXXFLAGS := -std=c++17 -Wall -Wextra -Wpedantic -Werror -O2 -g
LDFLAGS := -L/usr/local/lib -lgrpc++ -lprotobuf  # Add gRPC and protobuf libraries
CPPFLAGS := -MMD -MP -I/usr/local/include        # Include path for protobuf headers

# Directories
SRC_DIR := src
PROTO_DIR := proto             # Directory where .proto files are stored
BUILD_DIR := build
BIN_DIR := bin
INCLUDE_DIR := include
TARGET := $(BIN_DIR)/main
DEBUG_TARGET := $(BIN_DIR)/main_debug

# Source and object files
SRCS := $(shell find $(SRC_DIR) -name '*.cpp')
PROTO_SRCS := $(shell find $(PROTO_DIR) -name '*.proto')
PROTO_OBJS := $(PROTO_SRCS:$(PROTO_DIR)/%.proto=$(BUILD_DIR)/%.pb.o)
OBJS := $(SRCS:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o) $(PROTO_OBJS)
DEBUG_OBJS := $(SRCS:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.debug.o) $(PROTO_OBJS)
DEPS := $(OBJS:.o=.d)

# Default rule
all: $(TARGET)

# Linking
$(TARGET): $(OBJS) | $(BIN_DIR)
	$(CXX) $(OBJS) -o $@ $(LDFLAGS)

# Debug linking
$(DEBUG_TARGET): $(DEBUG_OBJS) | $(BIN_DIR)
	$(CXX) $(DEBUG_OBJS) -o $@ $(LDFLAGS)

# Compile source files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -I$(INCLUDE_DIR) -c $< -o $@

# Compile source files with debug symbols
$(BUILD_DIR)/%.debug.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -Wno-unused-variable -O0 -g $(CPPFLAGS) -I$(INCLUDE_DIR) -c $< -o $@

# Generate .pb.cc and .pb.h from .proto
$(BUILD_DIR)/%.pb.o: $(PROTO_DIR)/%.proto | $(BUILD_DIR)
	protoc -I=$(PROTO_DIR) --cpp_out=$(BUILD_DIR) $<
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -I$(BUILD_DIR) -c $(BUILD_DIR)/$*.pb.cc -o $@

# Create build directories if not exist
$(BUILD_DIR) $(BIN_DIR):
	mkdir -p $@

# Clean up build artifacts
clean:
	$(RM) -r $(BUILD_DIR) $(BIN_DIR)

# Include dependency files
-include $(DEPS)

# Phony targets
.PHONY: all clean debug