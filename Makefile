TARGET = monitor.elf
CC = clang
LD = ld.lld
OBJCOPY = llvm-objcopy

# --- Directory Setup ---
# This finds all subdirectories inside 'src' to use as include paths
SRC_DIR = src
INC_DIR = include
BUILD_DIR = build

# --- Automatic File Discovery ---
# Finds all .c and .S files in any subfolder of src/
SRCS := $(shell find $(SRC_DIR) -name '*.c' -o -name '*.S')
# Maps those source files to the build directory as .o files
OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)

# --- Flags ---
# -I$(INC_DIR) allows #include "mmu.h" if mmu.h is in include/
# We also add -Isrc to allow including files relative to the src folder
CFLAGS = --target=aarch64-none-elf -I$(INC_DIR) -I$(SRC_DIR) -g

all: $(TARGET)

$(TARGET): $(OBJS)
	$(LD) -T linker.ld $(OBJS) -o $(TARGET)
	$(OBJCOPY) -O binary $(TARGET) monitor.bin

# Rule for C files
$(BUILD_DIR)/%.c.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# Rule for Assembly files
$(BUILD_DIR)/%.S.o: %.S
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR) $(TARGET) monitor.bin

.PHONY: all clean

