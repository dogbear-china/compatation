include ../common/common.mk

TARGET := $(BUILD_DIR)/lab04
SOURCES := main.cpp $(COMMON_GENERATOR) $(COMMON_BENCHMARK)
BENCH_CPU ?=

.PHONY: all run affinity-info compare-affinity test clean

all: $(TARGET)

$(TARGET): $(SOURCES) ../common/csr_matrix.hpp ../common/benchmark_utils.hpp
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CPPFLAGS) $(COMMON_STANDARD) $(COMMON_WARNINGS) $(RELEASE_FLAGS) $(SOURCES) -o $@

run: all
	@if [ -n "$(BENCH_CPU)" ]; then \
		echo "运行模式：绑定 CPU $(BENCH_CPU)"; \
		taskset -c "$(BENCH_CPU)" sh -c 'taskset -pc $$$$; exec ./$(TARGET)'; \
	else \
		echo "运行模式：未额外绑核"; \
		sh -c 'taskset -pc $$$$; exec ./$(TARGET)'; \
	fi

affinity-info:
	@sh -c 'taskset -pc $$$$'

compare-affinity: all
	@if [ -z "$(BENCH_CPU)" ]; then \
		echo "请指定允许使用的核心，例如：make compare-affinity BENCH_CPU=2"; \
		exit 1; \
	fi
	@echo "===== 未额外绑核 ====="
	@sh -c 'taskset -pc $$$$; exec ./$(TARGET)'
	@echo "===== 绑定 CPU $(BENCH_CPU) ====="
	@taskset -c "$(BENCH_CPU)" sh -c 'taskset -pc $$$$; exec ./$(TARGET)'

test: run

clean:
	rm -rf $(BUILD_DIR)
