include ../common/common.mk

TARGET := $(BUILD_DIR)/lab07
DEBUG_TARGET := $(BUILD_DIR)/lab07_debug
SOURCES := main.cpp spmv_student.cpp $(COMMON_GENERATOR) $(COMMON_BENCHMARK) $(COMMON_IO)
RUN ?= latest
BENCH_CPU ?=
SPMV_DATA_ROOT ?= ../data
PIN_COMMAND := $(if $(BENCH_CPU),taskset -c $(BENCH_CPU),)

.PHONY: all run test debug-test benchmark clean

all: $(TARGET)

$(TARGET): $(SOURCES) ../common/csr_matrix.hpp ../common/benchmark_utils.hpp
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CPPFLAGS) $(COMMON_STANDARD) $(COMMON_WARNINGS) $(RELEASE_FLAGS) $(SOURCES) -o $@

$(DEBUG_TARGET): $(SOURCES) ../common/csr_matrix.hpp ../common/benchmark_utils.hpp
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CPPFLAGS) $(COMMON_STANDARD) $(COMMON_WARNINGS) $(DEBUG_FLAGS) $(SOURCES) -o $@

run: test

test: $(TARGET)
	./$(TARGET) test

debug-test: $(DEBUG_TARGET)
	ASAN_OPTIONS=detect_leaks=0 ./$(DEBUG_TARGET) test

benchmark: $(TARGET)
	@mkdir -p results
	SPMV_DATA_ROOT="$(SPMV_DATA_ROOT)" $(PIN_COMMAND) ./$(TARGET) benchmark results/$(RUN).csv

clean:
	rm -rf $(BUILD_DIR) results
