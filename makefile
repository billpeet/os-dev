ASM=nasm
CC=gcc
CFLAGS=-fno-stack-protector -mgeneral-regs-only -nostdlib -nodefaultlibs -ffreestanding
LD=ld
SRCDIR=src
OBJDIR=obj
OBJ=$(OBJDIR)/kernel.o $(OBJDIR)/vga.o $(OBJDIR)/idt.o $(OBJDIR)/pong.o $(OBJDIR)/shell.o $(OBJDIR)/alloc.o $(OBJDIR)/string.o $(OBJDIR)/frame_allocator.o $(OBJDIR)/paging.o $(OBJDIR)/long_mode_loader.o $(OBJDIR)/boot_info.o

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