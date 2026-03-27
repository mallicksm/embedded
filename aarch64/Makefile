TARGET    := firmware

LLVM      := /opt/homebrew/opt/llvm/bin
LLD       := /opt/homebrew/opt/lld/bin

CC        := $(LLVM)/clang
LD        := $(LLD)/ld.lld
OBJDUMP   := $(LLVM)/llvm-objdump
READELF   := $(LLVM)/llvm-readelf
GDB = aarch64-elf-gdb

OBJS      := crt0.o main.o

CFLAGS    := --target=aarch64-none-elf -ffreestanding -O0 -g

.PHONY: all run clean dump elfinfo

all: $(TARGET).elf

crt0.o: crt0.S
	$(CC) $(CFLAGS) -c $< -o $@

main.o: main.c
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET).elf: $(OBJS)
	$(LD) -e _start -T linker.ld -o $@ $(OBJS)

run: $(TARGET).elf
	qemu-system-aarch64 \
		-machine virt \
		-cpu cortex-a57 \
		-nographic \
		-kernel $(TARGET).elf \
		-S -gdb tcp::1234

dbg:
	$(GDB) -tui test.elf \
		-ex "target remote :1234" \
		-ex "layout split"

puart:
	@echo "Ctrl-C to exit, Ctrl-D to quit"
	@nc localhost 4444

dump: $(TARGET).elf
	$(OBJDUMP) -d $(TARGET).elf

elfinfo: $(TARGET).elf
	$(READELF) -a $(TARGET).elf

clean:
	rm -f $(OBJS) $(TARGET).elf
