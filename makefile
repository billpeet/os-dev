ASM=nasm
CC=gcc
CFLAGS=-fno-stack-protector -mgeneral-regs-only -nostdlib -nodefaultlibs -ffreestanding
LD=ld
SRCDIR=src
OBJDIR=obj
CFILES=$(wildcard $(SRCDIR)/*.c)
OBJ=$(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(CFILES))

all: os.iso

run: kernel
	qemu-system-i386 -kernel kernel

clean:
	rm -f $(OBJDIR)/* isofiles/boot/kernel.bin os.iso

os.iso: isofiles/boot/kernel.bin
	grub-mkrescue -o os.iso isofiles

isofiles/boot/kernel.bin: $(OBJDIR)/kasm.o $(OBJDIR)/long.o $(OBJ)
	$(LD) -n -T link.ld -o isofiles/boot/kernel.bin $(OBJDIR)/kasm.o $(OBJDIR)/long.o $(OBJ)

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) -c $< -o $@ $(CFLAGS)

$(OBJDIR)/kasm.o: $(SRCDIR)/kernel.asm
	$(ASM) -f elf64 $(SRCDIR)/kernel.asm -o $@

$(OBJDIR)/long.o: $(SRCDIR)/long.asm
	$(ASM) -f elf64 $(SRCDIR)/long.asm -o $@