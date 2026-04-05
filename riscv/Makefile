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
QEMU     := qemu-system-riscv32

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
   -O0 \
   -g

LDFLAGS := \
   -e _start \
   -T linker.ld \
   -Map $(TARGET).map \
   --gc-sections

SRCS := crt0.S \
         thread_switch.S \
         trap_entry.S \
         main.c \
         uart.c \
         cmds.c \
         printf.c \
         stdlib.c \
         string.c \
         cli.c \
         proc.c \
         progs.c \
         trap.c

OBJS := $(SRCS:.c=.o)
OBJS := $(OBJS:.S=.o)

all: clean $(TARGET).elf $(TARGET).bin $(TARGET).lst $(TARGET).info compile_commands.json

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.S
	$(CC) $(filter-out -g, $(CFLAGS)) -c $< -o $@

$(TARGET).elf: $(OBJS) linker.ld
	$(LD) $(LDFLAGS) -o $@ $(OBJS)

$(TARGET).bin: $(TARGET).elf
	$(OBJCOPY) -O binary $< $@

$(TARGET).lst: $(TARGET).elf
	$(OBJDUMP) -S -d $< > $@

$(TARGET).info: $(TARGET).elf
	$(READELF) -a $(TARGET).elf > $@

run: $(TARGET).elf
	@echo "Ctrl + c to exit"
	$(QEMU) \
      -machine virt \
      -display none \
      -bios none \
      -serial tcp::4444,server=on,wait=off \
      -kernel $(TARGET).elf \
      -S -gdb tcp::1234

puart:
	@echo "Welcome AITMOS \"Task Manager OS\" Console"
	@echo "Ctrl + ] to exit"
	@socat -,raw,echo=0,escape=0x1d tcp:localhost:4444

dbg:
	$(GDB) \
      -tui $(TARGET).elf -x debug.gdb

#-------------------------------------------------------------------------------
# Helper
#-------------------------------------------------------------------------------
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

gui:
	gdbgui $(TARGET).elf --gdb-cmd riscv64-elf-gdb

ddd:
	ddd --debugger riscv64-elf-gdb $(TARGET).elf

.PHONY: all run dbg puart compile_commands.json clean puart dbg
