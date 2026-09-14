include ../common/common.mk

TARGET := $(BUILD_DIR)/lab02
SOURCES := main.cpp $(COMMON_GENERATOR) $(COMMON_BENCHMARK)

.PHONY: all run test clean

all: $(TARGET)

$(TARGET): $(SOURCES) ../common/csr_matrix.hpp ../common/benchmark_utils.hpp
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CPPFLAGS) $(COMMON_STANDARD) $(COMMON_WARNINGS) $(RELEASE_FLAGS) $(SOURCES) -o $@

run: all
	./$(TARGET)

test: run

clean:
	rm -rf $(BUILD_DIR)
