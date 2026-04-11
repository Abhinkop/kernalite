# Toolchain definition
CROSS_COMPILE ?= aarch64-linux-gnu-
CC      = $(CROSS_COMPILE)gcc
AS      = $(CROSS_COMPILE)as
LD      = $(CROSS_COMPILE)ld
OBJCOPY = $(CROSS_COMPILE)objcopy
STRIP   = $(CROSS_COMPILE)strip

IMG_NAME ?= Image

# Flags
# -Wall -Wextra: Enable all warnings
# -ffreestanding: No standard library environment
# -nostdlib: Don't link against system libraries
CFLAGS  = -c -Wall -Wextra -ffreestanding -nostdlib -nostartfiles -Isrc/include
ASFLAGS = -c -x assembler-with-cpp
LDFLAGS = -T scripts/linker.ld

DEBUG_FLAGS = -g
RELEASE_FLAGS = -O3

# Project structure
BUILD_DIR ?= build
SRC_DIR   = src

# build type: debug or release (default: release)
BUILD_TYPE = release
ifeq ($(DEBUG),1)
  BUILD_TYPE = debug
else
  BUILD_TYPE = release
endif

# Output binary name
TARGET_ELF = $(BUILD_DIR)/images/$(IMG_NAME).elf
TARGET = $(BUILD_DIR)/images/$(IMG_NAME)

# Source files
SRCS_C  = $(SRC_DIR)/kernel/main.c
SRCS_AS = $(SRC_DIR)/boot/boot.s

OBJS_C  = $(SRCS_C:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)
OBJS_AS = $(SRCS_AS:$(SRC_DIR)/%.s=$(BUILD_DIR)/%.o)
OBJS    = $(OBJS_C) $(OBJS_AS)

# --- Verbosity Control ---
# If V is not 1, we prefix commands with @ to hide them.
ifeq ($(V),1)
  VERBOSE_PREFIX :=
else
  VERBOSE_PREFIX := @
endif

ifeq ($(BUILD_TYPE),debug)
  CFLAGS += $(DEBUG_FLAGS)
else
  CFLAGS += $(RELEASE_FLAGS)
  POST_BUILD = @echo "STRIP $@"; $(STRIP) --strip-all $< -o $@
endif

.PHONY: all clean
all: $(TARGET)
# Convert ELF to raw Binary
$(TARGET): $(TARGET_ELF)
	@mkdir -p $(dir $@)
	@echo "OBJCOPY $@"
	$(VERBOSE_PREFIX)$(OBJCOPY) -O binary $< $@
	$(VERBOSE_PREFIX)$(POST_BUILD)

# Compile C files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	@echo "CC  $<"
	$(VERBOSE_PREFIX)$(CC) $(CFLAGS) -c $< -o $@

# Compile assembly files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.s
	@mkdir -p $(dir $@)
	@echo "CC  $<"
	$(VERBOSE_PREFIX)$(CC) $(ASFLAGS) $< -o $@

# Link the kernel
$(TARGET_ELF): $(OBJS)
	@mkdir -p $(dir $@)
	@echo "LD  $@"
	$(VERBOSE_PREFIX)$(LD) $(LDFLAGS) $(OBJS) -o $@

clean:
	rm -rf $(BUILD_DIR) $(TARGET) $(TARGET_ELF)
