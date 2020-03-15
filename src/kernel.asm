bits 32                     ; nasm directive - 32 bit
section .text
    ;multiboot spec
    align 4
    dd 0x1BADB002           ; magic number
    dd 0x00                 ; flags
    dd - (0x1BADB002 + 0x00); checksum - m+f+c should be 0

global start
global read_port
global write_port
global load_idt
global load_page_directory
global enable_paging

extern kmain                ; kmain is in the C file

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

load_page_directory:
    ; push ebp
    ; mov ebp, esp
    mov eax, [esp + 4]      ; grab first parameter into edx
    mov cr3, eax            ; set IDT location
    ; mov esp, ebp
    ; pop ebp
    ret

enable_paging:
    push ebp
    mov ebp, esp
    mov eax, cr0            ; set IDT location
    or eax, $80000000       ; enable paging (32nd bit)
    mov cr0, eax
    mov esp, ebp
    pop ebp
    ret

start:
    cli                     ; block all interrupts
    mov esp, stack_space    ; set stack pointer
    call kmain
    hlt                     ; halt

section .bss
resb 8192                   ; 8KB for stack
stack_space: