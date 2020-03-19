global long_mode_start
global idt
global idtr
global load_idt
global read_port
global inb
global write_port
global write_port_16
global test_read

section .text

align 4
idt:
    resb 256*16

  idtr:
    dw (256*16)-1
    dq idt

extern kmain
extern code_selector

long_mode_start:    ; start of long mode
    mov ax, 0       ; zero out registers
    mov ss, ax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    jmp kmain       ; jump to kmain in kernel.c
    ret

read_port:
    mov rdx, rdi
    in ax, dx
    ret

inb:
    mov rdx, rdi
    in al, dx
    ret

write_port:
    mov rdx, rdi
    mov rax, rsi
    out dx, al
    ret

write_port_16:
    mov rdx, rdi
    mov rax, rsi
    out dx, ax
    ret

load_idt:
    lidt [idtr]
    sti
    ret
