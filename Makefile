CC      := clang
LD      := ld.lld
OBJCOPY := llvm-objcopy

TARGET  := mips-unknown-elf
CFLAGS  := --target=$(TARGET) -march=mips3 -mabi=32 -EB
CFLAGS  += -ffreestanding -fno-builtin -fno-pic -mno-abicalls -G0
CFLAGS  += -O2 -g -Wall -Wextra
LDFLAGS := -m elf32btsmip -T n64.ld

BUILD   := build
ROM     := $(BUILD)/homebrew.z64
OBJS    := $(BUILD)/n64_header.o $(BUILD)/boot.o $(BUILD)/test.o

.PHONY: all clean
all: $(ROM)

$(BUILD):
	mkdir -p $@

$(BUILD)/%.o: %.S | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD)/%.o: %.c | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD)/homebrew.elf: $(OBJS) n64.ld
	$(LD) $(LDFLAGS) -o $@ $(OBJS)

$(ROM): $(BUILD)/homebrew.elf romfix.py
	$(OBJCOPY) -O binary $< $@
	python3 romfix.py $@

clean:
	rm -rf $(BUILD)
