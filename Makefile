# Makefile for DateTimeForCpp project
# Targets:test (test/test.cpp)

CXX      := g++.exe
CXXFLAGS := -std=c++20 -O2 -g -Wall -Wextra
INCLUDES := -Iinclude
LDFLAGS  := 
LDLIBS   := 

# Build dir
BUILD_DIR := build

# Target files
TARGETS := $(BUILD_DIR)/test.exe

.PHONY: all clean test

all: $(TARGETS)

# Make build dir
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# test - Test for DateTime
$(BUILD_DIR)/test.exe: test/test.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< -o $@ $(LDFLAGS)

test: $(BUILD_DIR)/test.exe

clean:
	-if exist $(BUILD_DIR) rmdir /s /q $(BUILD_DIR)
