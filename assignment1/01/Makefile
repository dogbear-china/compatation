include ../common/common.mk

TARGET := $(BUILD_DIR)/lab01

.PHONY: all run test clean

all: $(TARGET)

$(TARGET): main.cpp ../common/csr_matrix.hpp
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CPPFLAGS) $(COMMON_STANDARD) $(COMMON_WARNINGS) $(RELEASE_FLAGS) main.cpp -o $@

run: all
	./$(TARGET)

test: run

clean:
	rm -rf $(BUILD_DIR)
