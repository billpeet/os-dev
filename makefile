ASM=wsl nasm
CC=wsl gcc
CFLAGS=-fno-stack-protector -mgeneral-regs-only
LD=wsl ld
SRCDIR=src
OBJDIR=obj

all: kernel

run: kernel
	qemu-system-i386 -kernel kernel

clean:
	rm -f $(OBJDIR)/* kernel

kernel: $(OBJDIR)/kasm.o $(OBJDIR)/kc.o $(OBJDIR)/vga.o $(OBJDIR)/idt.o $(OBJDIR)/pong.o $(OBJDIR)/shell.o
	$(LD) -m elf_i386 -T link.ld -o kernel $(OBJDIR)/kasm.o $(OBJDIR)/kc.o $(OBJDIR)/vga.o $(OBJDIR)/idt.o $(OBJDIR)/pong.o $(OBJDIR)/shell.o

$(OBJDIR)/kasm.o: $(SRCDIR)/kernel.asm
	$(ASM) -f elf32 $(SRCDIR)/kernel.asm -o $@

$(OBJDIR)/kc.o: $(SRCDIR)/kernel.c $(SRCDIR)/vga.h
	$(CC) -m32 -c $(SRCDIR)/kernel.c -o $@ $(CFLAGS)

$(OBJDIR)/vga.o: $(SRCDIR)/vga.c $(SRCDIR)/vga.h
	$(CC) -m32 -c $(SRCDIR)/vga.c -o $@ $(CFLAGS)

$(OBJDIR)/idt.o: $(SRCDIR)/idt.c $(SRCDIR)/idt.h
	$(CC) -m32 -c $(SRCDIR)/idt.c -o $@ $(CFLAGS)

$(OBJDIR)/shell.o: $(SRCDIR)/shell.c $(SRCDIR)/shell.h
	$(CC) -m32 -c $(SRCDIR)/shell.c -o $@ $(CFLAGS)

$(OBJDIR)/pong.o: $(SRCDIR)/pong.c $(SRCDIR)/pong.h
	$(CC) -m32 -c $(SRCDIR)/pong.c -o $@ $(CFLAGS)
