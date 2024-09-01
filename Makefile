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

# Source and object files
SRCS := $(shell find $(SRC_DIR) -name '*.cpp')
OBJS := $(SRCS:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

# Default rule
all: $(TARGET)

# Linking
$(TARGET): $(OBJS) | $(BIN_DIR)
	$(CXX) $(OBJS) -o $@ $(LDFLAGS)

# Compile source files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -I$(INCLUDE_DIR) -c $< -o $@

# Create build directories if not exist
$(BUILD_DIR) $(BIN_DIR):
	mkdir -p $@

# Clean up build artifacts
clean:
	$(RM) -r $(BUILD_DIR) $(BIN_DIR)

# Include dependency files
-include $(DEPS)

# Phony targets
.PHONY: all clean