ASM=nasm
CC=~/opt/cross/bin/x86_64-elf-gcc
CFLAGS=-fno-stack-protector -mgeneral-regs-only -nostdlib -nodefaultlibs -ffreestanding -mno-red-zone -Isrc/libc/include -ggdb
LD=ld
SRCDIR=src
OBJDIR=obj
CFILES=$(shell find $(SRCDIR)/kernel/ -type f -name '*.c')
ASMFILES=$(shell find $(SRCDIR)/kernel/ -type f -name '*.asm')
LIBCFILES=$(shell find $(SRCDIR)/libc/ -type f -name '*.c')
KERNELOBJ=$(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(CFILES))
KERNELASMOBJ=$(patsubst $(SRCDIR)/%.asm,$(OBJDIR)/%.o,$(ASMFILES))
LIBCOBJ=$(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(LIBCFILES))

all: os.iso fat.img

debug: all
	qemu-system-x86_64 -cdrom os.iso -drive file=fat.img,format=raw -boot d -s -S

run: all
	qemu-system-x86_64 -cdrom os.iso -drive file=fat.img,format=raw -boot d

clean:
	rm -rf $(OBJDIR) isofiles/boot/kernel.bin os.iso

os.iso: isofiles/boot/kernel.bin
	grub-mkrescue -o os.iso isofiles

fat.img: hdfiles/* hdfiles/program
	sh build-fat.sh

hdfiles/program: $(SRCDIR)/user/program.c
	$(CC) $< -o $@ $(CFLAGS)

isofiles/boot/kernel.bin: $(KERNELASMOBJ) $(KERNELOBJ) $(LIBCOBJ)
	$(LD) -n -T link.ld -o isofiles/boot/kernel.bin $(KERNELASMOBJ) $(KERNELOBJ) $(LIBCOBJ)

$(OBJDIR)/%.o: $(SRCDIR)/%.c $(SRCDIR)/kernel/x86.h
	@mkdir -p "$(@D)"
	$(CC) -c $< -o $@ $(CFLAGS)

$(OBJDIR)/kernel/entry.o: $(SRCDIR)/kernel/entry.asm
	@mkdir -p "$(@D)"
	$(ASM) -f elf64 -g -F dwarf $< -o $@

$(OBJDIR)/kernel/entry_long.o: $(SRCDIR)/kernel/entry_long.asm
	@mkdir -p "$(@D)"
	$(ASM) -f elf64 -g -F dwarf $< -o $@