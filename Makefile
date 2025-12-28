TARGET = monitor.elf
CC = clang
LD = ld.lld
OBJCOPY = llvm-objcopy

SRC_DIR = src
INC_DIR = include
BUILD_DIR = build

SRCS := $(shell find $(SRC_DIR) -name '*.c' -o -name '*.S')
OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)

CFLAGS = --target=aarch64-none-elf -I$(INC_DIR) -I$(SRC_DIR) -g

all: $(TARGET)

$(TARGET): $(OBJS)
	# 2. Link the monitor objects and pull in the raw kernel binary blob
	$(LD) -T linker.ld $(OBJS) -o $(TARGET)
	# 3. Create the final bootable image
	$(OBJCOPY) -O binary $(TARGET) monitor.bin

$(BUILD_DIR)/%.o: %
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR) $(TARGET) monitor.bin
