bits 32                     ; nasm directive - 32 bit
section .text
    ;multiboot spec
    align 4
    dd 0x1BADB002           ; magic number
    dd 0x00                 ; flags
    dd - (0x1BADB002 + 0x00); checksum - m+f+c should be 0

global start
global asm_double_fault_handler
global asm_breakpoint_handler
global asm_keyboard_handler
global asm_timer_handler
global read_port
global write_port
global load_idt

extern kmain                ; kmain is in the C file
extern double_fault_handler
extern breakpoint_handler
extern keyboard_handler
extern timer_handler

read_port:
    mov edx, [esp + 4]      ; move top of stack into dx
    in al, dx               ; ret port at dx into al (return address)
    ret

write_port:
    mov edx, [esp + 4]      ; move top of stack into dx
    mov al, [esp + 4 + 4]   ; move second param into al
    out dx, al              ; move value in al into port at dx
    ret

load_idt:
    mov edx, [esp + 4]      ; grab first parameter into edx
    lidt [edx]              ; set IDT location
    sti                     ; turn on interrupts
    ret

asm_double_fault_handler:
    call double_fault_handler
    hlt

asm_breakpoint_handler:
    call breakpoint_handler
    iretd

asm_keyboard_handler:
    call keyboard_handler
    iretd

start:
    cli                     ; block all interrupts
    mov esp, stack_space    ; set stack pointer
    call kmain
    hlt                     ; halt

section .bss
resb 8192                   ; 8KB for stack
stack_space: