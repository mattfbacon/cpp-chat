.PHONY: cmake debug release clean distclean install

BUILD_DIR := build

CMAKE := cmake

CMAKE_FLAGS :=

export CMAKE_GENERATOR = Ninja Multi-Config

$(BUILD_DIR): CMakeLists.txt
	$(CMAKE) $(CMAKE_FLAGS) -S. -H. -B$(BUILD_DIR)
cmake: $(BUILD_DIR)

debug release: %: $(BUILD_DIR)
# hacky support for multiconfig generators
	$(CMAKE) --build $(BUILD_DIR) --target $@ -- -v\
		|| $(CMAKE) --build $(BUILD_DIR) --config $(shell echo $@ | python -c "print(input().capitalize())") -- -v
clean distclean install: %: $(BUILD_DIR)
	$(CMAKE) --build $(BUILD_DIR) --target $@

.DEFAULT_GOAL = release
