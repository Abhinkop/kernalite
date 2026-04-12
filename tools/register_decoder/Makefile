CC = gcc
CFLAGS = -Wall -Wextra
TARGET = register_decoder
SRC = main.c
SRC_H = 	sctlr_el1_64.h \
			sctlr_el1_32.h

BUILD_DIR ?= build

APP_BUILD_DIR = $(BUILD_DIR)/tools/register_decoder

# --- Verbosity Control ---
# If V is not 1, we prefix commands with @ to hide them.
ifeq ($(V),1)
  VERBOSE_PREFIX :=
else
  VERBOSE_PREFIX := @
endif

all: $(TARGET)

$(TARGET): $(SRC)
	$(VERBOSE_PREFIX)mkdir -p $(APP_BUILD_DIR)
	$(VERBOSE_PREFIX)$(CC) $(CFLAGS) -o $(APP_BUILD_DIR)/$@ $^

run: $(TARGET)
	$(BUILD_DIR)/tools/register_decoder/$(TARGET)

format: $(SRC) $(SRC_H)
	@echo "Formatting source files..."
	$(VERBOSE_PREFIX)clang-format -i $^

clang-tidy: $(SRC)
	@echo "Running clang-tidy..."
	$(VERBOSE_PREFIX)clang-tidy $^ -- $(CFLAGS)

clang-tidy-fix: $(SRC)
	@echo "Running clang-tidy..."
	$(VERBOSE_PREFIX)clang-tidy $^ --fix -- $(CFLAGS)

clean:
	@echo "Cleaning build directory..."
	$(VERBOSE_PREFIX)rm -rf $(BUILD_DIR)/tools/register_decoder
