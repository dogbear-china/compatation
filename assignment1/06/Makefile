include ../common/common.mk

SOURCES := main.cpp $(COMMON_GENERATOR)
TARGET := $(BUILD_DIR)/lab06

.PHONY: all run test clean

all: $(TARGET)

$(TARGET): $(SOURCES) ../common/csr_matrix.hpp
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CPPFLAGS) $(COMMON_STANDARD) $(COMMON_WARNINGS) $(RELEASE_FLAGS) $(SOURCES) -o $@

run: $(TARGET)
	./$(TARGET)

test: $(TARGET)
	./$(TARGET) --check

clean:
	rm -rf $(BUILD_DIR)
