TARGET   := firmware

LLVM     := /opt/homebrew/opt/llvm/bin
LLD      := /opt/homebrew/opt/lld/bin

CC       := $(LLVM)/clang
LD       := $(LLD)/ld.lld
OBJDUMP  := $(LLVM)/llvm-objdump
OBJCOPY  := $(LLVM)/llvm-objcopy
READELF  := $(LLVM)/llvm-readelf
GDB      := aarch64-elf-gdb

SRCS := crt0.S main.c
OBJS := $(SRCS:.c=.o)
OBJS := $(OBJS:.S=.o)

CFLAGS := \
   --target=aarch64-none-elf \
   -ffreestanding \
   -O0 \
   -g

LDFLAGS := \
   -e _start \
   -T linker.ld \
   -Map $(TARGET).map \
   --gc-sections

all: $(TARGET).elf $(TARGET).bin $(TARGET).lst $(TARGET).info

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.S
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET).elf: $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $(OBJS)

$(TARGET).bin: $(TARGET).elf
	$(OBJCOPY) -O binary $< $@

$(TARGET).lst: $(TARGET).elf
	$(OBJDUMP) -d -S $< > $@

$(TARGET).info: $(TARGET).elf
	$(READELF) -a $(TARGET).elf > $@

run: $(TARGET).elf
	qemu-system-aarch64 \
      -machine virt \
      -cpu cortex-a57 \
      -display none \
      -serial tcp::4444,server=on,wait=off \
      -kernel $(TARGET).elf \
      -S -gdb tcp::1234

dbg:
	$(GDB) -tui test.elf \
      -ex "target remote :1234" \
      -ex "layout split"

puart:
	@echo "Ctrl-C to exit, Ctrl-D to quit"
	@nc localhost 4444

clean:
	rm -f $(OBJS) $(TARGET).elf $(TARGET).bin $(TARGET).lst $(TARGET).info $(TARGET).map

.PHONY: all run dbg puart clean
