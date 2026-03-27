LLVM      := /opt/homebrew/opt/llvm/bin
LLD       := /opt/homebrew/opt/lld/bin

CC        := $(LLVM)/clang
LD        := $(LLD)/ld.lld
OBJDUMP   := $(LLVM)/llvm-objdump
READELF   := $(LLVM)/llvm-readelf

TARGET    := test.elf
OBJS      := startup.o main.o

CFLAGS    := --target=aarch64-none-elf -ffreestanding -O0 -g

.PHONY: all run clean dump elfinfo

all: $(TARGET)
	qemu-system-aarch64 \
		-machine virt \
		-cpu cortex-a57 \
		-nographic \
		-kernel $(TARGET) \
		-S -gdb tcp::1234

startup.o: startup.S
	$(CC) $(CFLAGS) -c $< -o $@

main.o: main.c
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJS)
	$(LD) -e _start -T linker.ld -o $@ $(OBJS)

dbg:
	gdb -tui test.elf \
		-ex "target remote :1234" \
		-ex "layout split"

dump: $(TARGET)
	$(OBJDUMP) -d $(TARGET)

elfinfo: $(TARGET)
	$(READELF) -a $(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)
