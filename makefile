ASM=nasm
CC=~/opt/cross/bin/x86_64-elf-gcc
CFLAGS=-fno-stack-protector -mgeneral-regs-only -nostdlib -nodefaultlibs -ffreestanding -mno-red-zone -Isrc/libc/include -ggdb
LD=ld
SRCDIR=src
OBJDIR=obj
CFILES=$(wildcard $(SRCDIR)/kernel/*.c)
LIBCFILES=$(wildcard $(SRCDIR)/libc/*.c)
KERNELOBJ=$(patsubst $(SRCDIR)/kernel/%.c,$(OBJDIR)/kernel/%.o,$(CFILES))
LIBCOBJ=$(patsubst $(SRCDIR)/libc/%.c,$(OBJDIR)/libc/%.o,$(LIBCFILES))

all: os.iso fat.img

debug: all
	qemu-system-x86_64 -cdrom os.iso -hda fat.img -boot d -s -S

run: all
	qemu-system-x86_64 -cdrom os.iso -hda fat.img -boot d

clean:
	rm -f $(OBJDIR)/kernel/* $(OBJDIR)/libc/* isofiles/boot/kernel.bin os.iso

os.iso: isofiles/boot/kernel.bin
	grub-mkrescue -o os.iso isofiles

fat.img: hdfiles/* hdfiles/program
	sh build-fat.sh

hdfiles/program: $(SRCDIR)/user/program.c
	$(CC) $< -o $@ $(CFLAGS)

isofiles/boot/kernel.bin: $(OBJDIR)/kernel/entry.o $(OBJDIR)/kernel/entry_long.o $(KERNELOBJ) $(LIBCOBJ)
	$(LD) -n -T link.ld -o isofiles/boot/kernel.bin $(OBJDIR)/kernel/entry.o $(OBJDIR)/kernel/entry_long.o $(KERNELOBJ) $(LIBCOBJ)

$(OBJDIR)/%.o: $(SRCDIR)/%.c $(SRCDIR)/kernel/x86.h
	$(CC) -c $< -o $@ $(CFLAGS)

$(OBJDIR)/kernel/entry.o: $(SRCDIR)/kernel/entry.asm
	$(ASM) -f elf64 -g -F dwarf $< -o $@

$(OBJDIR)/kernel/entry_long.o: $(SRCDIR)/kernel/entry_long.asm
	$(ASM) -f elf64 -g -F dwarf $< -o $@