ASM=nasm
CC=gcc
CFLAGS=-fno-stack-protector -mgeneral-regs-only -nostdlib -nodefaultlibs -ffreestanding -Isrc/libc/include
LD=ld
SRCDIR=src
OBJDIR=obj
CFILES=$(wildcard $(SRCDIR)/kernel/*.c)
LIBCFILES=$(wildcard $(SRCDIR)/libc/*.c)
KERNELOBJ=$(patsubst $(SRCDIR)/kernel/%.c,$(OBJDIR)/kernel/%.o,$(CFILES))
LIBCOBJ=$(patsubst $(SRCDIR)/libc/%.c,$(OBJDIR)/libc/%.o,$(LIBCFILES))

all: os.iso fat.img

run: kernel
	qemu-system-i386 -kernel kernel

clean:
	rm -f $(OBJDIR)/kernel/* $(OBJDIR)/libc/* isofiles/boot/kernel.bin os.iso

os.iso: isofiles/boot/kernel.bin
	grub-mkrescue -o os.iso isofiles

fat.img: hdfiles/*
	sh build-fat.sh

isofiles/boot/kernel.bin: $(OBJDIR)/kernel/kasm.o $(OBJDIR)/kernel/long.o $(KERNELOBJ) $(LIBCOBJ)
	$(LD) -n -T link.ld -o isofiles/boot/kernel.bin $(OBJDIR)/kernel/kasm.o $(OBJDIR)/kernel/long.o $(KERNELOBJ) $(LIBCOBJ)

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) -c $< -o $@ $(CFLAGS)

$(OBJDIR)/kernel/kasm.o: $(SRCDIR)/kernel/kernel.asm
	$(ASM) -f elf64 $< -o $@

$(OBJDIR)/kernel/long.o: $(SRCDIR)/kernel/long.asm
	$(ASM) -f elf64 $< -o $@