# Compiler and flags
CXX := clang++
CXXFLAGS := -std=c++17 -Wall -Wextra -Wpedantic -Werror -O2 -g
LDFLAGS := 
CPPFLAGS := -MMD -MP

# Directories
SRC_DIR := src
BUILD_DIR := build
BIN_DIR := bin
INCLUDE_DIR := include
TARGET := $(BIN_DIR)/main
DEBUG_TARGET := $(BIN_DIR)/main_debug

# Source and object files
SRCS := $(shell find $(SRC_DIR) -name '*.cpp')
OBJS := $(SRCS:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)
DEBUG_OBJS := $(SRCS:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.debug.o)
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