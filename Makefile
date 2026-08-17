# Compiler & Flags
CXX      := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -I./src/parser

# Folder Output Binary
BUILD_DIR := build

# Source Files
TEST_SRC  := test/parser.cpp

# Executables Target
TEST_BIN  := $(BUILD_DIR)/test_runner

# Macro for path
PWD_DIR   := $(shell pwd)
TEST_FLAGS := -DSAMPLE_DIR=\"$(PWD_DIR)\"

# Target Default
all: test

# Rules to build and running test
test: $(TEST_BIN)
	@echo "== [RUNNING TESTS] =="
	@./$(TEST_BIN)

# Linker for binary test
$(TEST_BIN): $(SRC_FILES) $(TEST_SRC)
	@mkdir -p $(BUILD_DIR)
	@echo "== [BUILDING TEST BINARY] =="
	$(CXX) $(CXXFLAGS) $(TEST_FLAGS) $^ -o $@

# Clean build directory
clean:
	@echo "== [CLEANING BUILD DIR] =="
	rm -rf $(BUILD_DIR)

.PHONY: all test clean