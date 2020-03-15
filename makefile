ASM=nasm
CC=gcc
CFLAGS=-fno-stack-protector -mgeneral-regs-only -nostdlib -nodefaultlibs
LD=ld
SRCDIR=src
OBJDIR=obj
OBJ=$(OBJDIR)/kernel.o $(OBJDIR)/vga.o $(OBJDIR)/idt.o $(OBJDIR)/pong.o $(OBJDIR)/shell.o $(OBJDIR)/alloc.o $(OBJDIR)/string.o $(OBJDIR)/memory.o

all: kernel

run: kernel
	qemu-system-i386 -kernel kernel

clean:
	rm -f $(OBJDIR)/* kernel

kernel: $(OBJDIR)/kasm.o $(OBJ)
	$(LD) -m elf_i386 -T link.ld -o kernel $(OBJDIR)/kasm.o $(OBJ)

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) -m32 -c $< -o $@ $(CFLAGS)

$(OBJDIR)/kasm.o: $(SRCDIR)/kernel.asm
	$(ASM) -f elf32 $(SRCDIR)/kernel.asm -o $@