TARGET   = firmware
ARCH     = rv32imac
ABI      = ilp32

LLVM_BIN = /opt/homebrew/opt/llvm/bin

CC       = $(LLVM_BIN)/clang
OBJCOPY  = $(LLVM_BIN)/llvm-objcopy
OBJDUMP  = $(LLVM_BIN)/llvm-objdump

COMMON_FLAGS = \
   --target=riscv32-unknown-elf \
   -march=$(ARCH) \
   -mabi=$(ABI) \
   -ffreestanding \
   -fno-builtin \
   -fno-stack-protector \
   -fno-pic \
   -nostdlib

CFLAGS = \
   $(COMMON_FLAGS) \
   -O2 \
   -Wall \
   -Wextra

LDFLAGS = \
   $(COMMON_FLAGS) \
   -fuse-ld=lld \
   -Wl,-T,linker.ld \
   -Wl,-Map,$(TARGET).map \
   -Wl,--gc-sections

SRCS = crt0.S main.c

all: $(TARGET).elf $(TARGET).bin $(TARGET).lst

$(TARGET).elf: $(SRCS) linker.ld
	$(CC) $(CFLAGS) $(LDFLAGS) $(SRCS) -o $@

$(TARGET).bin: $(TARGET).elf
	$(OBJCOPY) -O binary $< $@

$(TARGET).lst: $(TARGET).elf
	$(OBJDUMP) -d -S $< > $@

clean:
	rm -f $(TARGET).elf $(TARGET).bin $(TARGET).lst $(TARGET).map

run: $(TARGET).elf
	qemu-system-riscv32 \
	   -machine virt \
	   -nographic \
	   -bios none \
	   -kernel $(TARGET).elf

.PHONY: all clean run
