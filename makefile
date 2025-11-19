# -----------------------------
# Makefile for simple OS
# -----------------------------
# entry.s -> .o, bootloader.s -> flat 512-byte bootloader.bin
# kernel.c -> ELF -> flat binary -> combined bootable OS

# Tools
NASM    := nasm
CC      := i686-elf-gcc
LD      := ld
OBJCOPY := objcopy

CFLAGS  := -m32 -ffreestanding -O2 -Wall -Iinclude -pedantic -Wextra -isystem /usr/include
LDFLAGS := -m elf_i386 -T linker.ld

# Sources
BOOT_SRC   := bootloader.s
ENTRY_SRC  := entry.s
KERNEL_SRC := kernel.c $(shell find kernel -name '*.c')

# Objects
OBJ_ENTRY  := $(ENTRY_SRC:.s=.o)
OBJ_KERNEL := $(KERNEL_SRC:.c=.o)
OBJ_ALL    := $(OBJ_ENTRY) $(OBJ_KERNEL)

# Binaries
KERNEL_ELF     := kernel.elf
KERNEL_BIN     := kernel.bin
BOOTLOADER_BIN := bootloader.bin
BOOTABLE_BIN   := os.img

# -----------------------------
# Default target
# -----------------------------
all: bootable

# -----------------------------
# Compile entry + kernel
# -----------------------------
%.o: %.s
	$(NASM) -f elf32 $< -o $@ 

%.o: %.c
	@$(CC) $(CFLAGS) -c $< -o $@ 2>>build.log

# -----------------------------
# Link kernel ELF
# -----------------------------
kernel: $(OBJ_ALL)
	$(LD) $(LDFLAGS) -o $(KERNEL_ELF) $^ 2>>build.log

# Convert ELF -> flat binary
$(KERNEL_BIN): kernel
	$(OBJCOPY) -O binary $(KERNEL_ELF) $(KERNEL_BIN)
	# Align kernel.bin to 512 bytes
	@size=$$(stat -c%s "$(KERNEL_BIN)"); \
	pad=$$(( (512 - (size % 512)) % 512 )); \
	if [ $$pad -ne 0 ]; then \
		dd if=/dev/zero bs=1 count=$$pad >> $(KERNEL_BIN) 2>/dev/null; \
	fi

# -----------------------------
# Compile bootloader
# -----------------------------
bootloader: $(BOOT_SRC) $(KERNEL_BIN)
	# Calculate kernel size in 512-byte sectors
	@size=$$(stat -c%s "$(KERNEL_BIN)"); \
	sectors=$$((size / 512)); \
	echo "Kernel size: $$sectors sectors"; \
	$(NASM) -f bin -DKERNEL_SECTORS=$$sectors $< -o $(BOOTLOADER_BIN)
	@echo "Bootloader binary created: $(BOOTLOADER_BIN)"

# -----------------------------
# Create bootable OS
# -----------------------------
bootable: kernel bootloader
	# Combine bootloader + kernel
	cat $(BOOTLOADER_BIN) $(KERNEL_BIN) > $(BOOTABLE_BIN)
	@echo "Bootable OS created: $(BOOTABLE_BIN)"
	rm $(OBJ_ALL) *.elf *.bin

# -----------------------------
# Run in QEMU
# -----------------------------
run:
	qemu-system-x86_64 $(BOOTABLE_BIN) -m 4G

# -----------------------------
# Clean
# -----------------------------
clean:
	rm -f $(OBJ_ALL) $(KERNEL_ELF) $(KERNEL_BIN) $(BOOTLOADER_BIN) $(BOOTABLE_BIN)

.PHONY: all kernel bootloader bootable clean run
