-include Config.mk

CFLAGS = -ffreestanding -Wall -Wextra -MMD -MP -c -I.
LDFLAGS = -nostdlib -z max-page-size=0x1000
TEST_LDFLAGS = -lgtest -lgtest_main -pthread

C_FILES := $(shell find $(SRC_DIR) -path $(TEST_DIR) -prune -o -name '*.cc' -print)
S_FILES := $(shell find $(SRC_DIR) -path $(TEST_DIR) -prune -o -name '*.S' -print)
TEST_FILES := $(shell find $(TEST_DIR) -name '*.cc')

OBJS := $(patsubst %.cc,$(BUILD_DIR)/%.o,$(C_FILES)) $(patsubst %.S,$(BUILD_DIR)/%.o,$(S_FILES))
DEPS := $(OBJS:.o=.d)
TEST_OBJS := $(patsubst %.cc,$(BUILD_DIR)/%.o,$(TEST_FILES))

$(BUILD_DIR)/%.o: %.cc
	@mkdir -p $(dir $@)
	@echo "CC $< -> $@"
	@$(CXX) $(CFLAGS) $< -o $@

$(BUILD_DIR)/%.o: %.S
	@mkdir -p $(dir $@)
	@echo "AS $< -> $@"
	@$(AS) -f elf64 $< -o $@

$(TARGET): $(OBJS) link.ld
	@echo $(C_FILES) $(S_FILES)
	@echo "LD $(OBJS) -> $(TARGET)"
	@$(LD) $(LDFLAGS) -T link.ld $(OBJS) -o $(TARGET)

define find_source_file
  $(shell echo $(1) | sed -e 's|$(TEST_DIR)|$(SRC_DIR)|' -e 's|\.test||')
endef

test: $(TEST_OBJS)
	@mkdir -p $(TEST_BIN)
	@for test_obj in $(TEST_OBJS); do \
	  test_executable=$$(basename $$test_obj .test.o)_test; \
	  test_executable_path=$(TEST_BIN)/$$test_executable; \
	  echo "Linking $$test_executable_path"; \
	  $(CXX) $$test_obj -o $$test_executable_path $(TEST_LDFLAGS); \
	  ./$$test_executable_path; \
	done

clean:
	rm -rf $(BUILD_DIR) $(TARGET) $(wildcard $(TEST_BIN)/*_test)

-include $(DEPS)

.PHONY: clean test
