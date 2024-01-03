# PeetOS - a basic OS kernel


## Web links

### Kernel
https://arjunsreedharan.org/post/82710718100/kernels-101-lets-write-a-kernel

### General Structure
https://wiki.osdev.org/Meaty_Skeleton

### ASCII
http://www.asciitable.com/

### Disk
http://www.osdever.net/tutorials/view/lba-hdd-access-via-pio

### FAT
http://www.maverick-os.dk/FileSystemFormats/FAT32_FileSystem.html#Signature
http://www.maverick-os.dk/FileSystemFormats/VFAT_LongFileNames.html
https://www.pjrc.com/tech/8051/ide/fat32.html

### Task Switching
https://github.com/torvalds/linux/blob/ab851d49f6bfc781edd8bd44c72ec1e49211670b/arch/x86/include/asm/switch_to.h
https://wiki.osdev.org/Brendan%27s_Multi-tasking_Tutorial
https://wiki.osdev.org/Kernel_Multitasking
http://xarnze.com/article/Entering%20User%20Mode

### Caller Conventions
https://en.wikipedia.org/wiki/X86_calling_conventions#List_of_x86_calling_conventions

## Overview

### Initial boot - using grub multiboot
* Loads with grub directory with grub.cfg file
* Grub is a boot loader - loads into 32 bit mode

### entry.asm - 32 bit entry point
* Grub loads entry.asm, running in 32 bit mode
* Starts with multiboot header, to keep grub bootloader happy
* check_multiboot - checks that bootloader is legit
* check_cpuid - checks CPUID is supported
* check_long_mode - checks 64 bit mode is available
* set_up_page_tables - sets up all page tables (paging required for 64 bit)
* enable_paging - enables paging/virtual memory
* Loads GDT - correct memory segments
* Sets up stack location
* Jumps to long_mode_start in entry_long.asm

### entry_long.asm - 64 bit entry point
* Loaded by entry.asm
* long_mode_start - clears out all CPU registers and jumps to kmain in kernel.c
* Also contains switch_task - util method for task switching

### kernel.c - start of kernel code
* kmain - main kernel loop, never returns
* Clears VGA console output
* Prints welcome message
<!-- * Initializes stdio -->
* Loads boot info
* Initializes frame allocator
* Initializes IDT (hardware interrupts)
* Initializes heap
* Initializes serial interface
* Initializes console
* Initializes hard drive cache
* Enables task allocator - converts current running code to main_task
* Creates task for main shell
* Launches scheduler

### vga.c - VGA driver
* Consoles VGA hardware - located at 0xb8000

### boot_info.c - loads boot info

