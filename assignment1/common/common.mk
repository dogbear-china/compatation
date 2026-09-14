CXX ?= g++

CPPFLAGS += -I../common
COMMON_WARNINGS := -Wall -Wextra -Wpedantic
COMMON_STANDARD := -std=c++17
RELEASE_OPT_LEVEL := -O2
RELEASE_FLAGS := $(RELEASE_OPT_LEVEL) -march=native -DNDEBUG
DEBUG_FLAGS := -O0 -g3 -fsanitize=address,undefined -fno-omit-frame-pointer

COMMON_GENERATOR := ../common/matrix_generator.cpp
COMMON_BENCHMARK := ../common/benchmark_utils.cpp
COMMON_IO := ../common/matrix_io.cpp

BUILD_DIR := build
