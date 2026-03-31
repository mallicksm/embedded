TARGET   := firmware
ARCH     := rv32imac
ABI      := ilp32

LLVM     := /opt/homebrew/opt/llvm/bin
LLD      := /opt/homebrew/opt/lld/bin

CC       := $(LLVM)/clang
LD       := $(LLD)/ld.lld
OBJCOPY  := $(LLVM)/llvm-objcopy
OBJDUMP  := $(LLVM)/llvm-objdump
READELF  := $(LLVM)/llvm-readelf
GDB      := riscv64-elf-gdb

COMMON_FLAGS := \

CFLAGS := \
   --target=riscv32-unknown-elf \
   -march=$(ARCH) \
   -mabi=$(ABI) \
   -ffreestanding \
   -fno-builtin \
   -fno-stack-protector \
   -fno-pic \
   -nostdlib \
   -Wall \
   -Wextra \
   -O2 \
   -g

LDFLAGS := \
   -e _start \
   -T linker.ld \
   -Map $(TARGET).map \
   --gc-sections

SRCS := crt0.S main.c uart.c cli_cmd.c printf.c stdlib.c
OBJS := $(SRCS:.c=.o)
OBJS := $(OBJS:.S=.o)

all: $(TARGET).elf $(TARGET).bin $(TARGET).lst $(TARGET).info compile_commands.json

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.S
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET).elf: $(OBJS) linker.ld
	$(LD) $(LDFLAGS) -o $@ $(OBJS)

$(TARGET).bin: $(TARGET).elf
	$(OBJCOPY) -O binary $< $@

$(TARGET).lst: $(TARGET).elf
	$(OBJDUMP) -d -S $< > $@

$(TARGET).info: $(TARGET).elf
	$(READELF) -a $(TARGET).elf > $@

run: $(TARGET).elf
	qemu-system-riscv32 \
      -machine virt \
      -display none \
      -bios none \
      -serial tcp::4444,server=on,wait=off \
      -kernel $(TARGET).elf \
      -S -gdb tcp::1234

dbg:
	$(GDB) -tui $(TARGET).elf \
      -ex "set pagination off" \
      -ex "target remote :1234" \
      -ex 'set $$pc = 0x80000000'

puart:
	@echo "Ctrl-C to exit, Ctrl-D to quit"
	@nc localhost 4444

compile_commands.json:
	@echo "[" > $@
	@for f in $(SRCS); do \
      echo "  {" >> $@; \
      echo "    \"directory\": \"$(CURDIR)\"," >> $@; \
      echo "    \"command\": \"$(CC) $(CFLAGS) -c $$f -o $${f%.*}.o\"," >> $@; \
      echo "    \"file\": \"$$f\"" >> $@; \
      echo "  }," >> $@; \
   done
	@sed -e '$$s/},/}/' $@ > $@.tmp && mv $@.tmp $@
	@echo "]" >> $@

clean:
	rm -f $(OBJS) $(TARGET).elf $(TARGET).bin $(TARGET).lst $(TARGET).info $(TARGET).map

.PHONY: all run dbg puart compile_commands.json clean
