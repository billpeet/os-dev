global long_mode_start
global idt
global idtr
global load_idt
global trigger_int
global setup_int
global write_port
global read_port

section .text
int_handler:
    mov dword [0xb8000], ') : '
    iretq

align 4
idt:
    resb 256*16

  idtr:
    dw (256*16)-1
    dq idt

extern kmain
extern code_selector

read_port:
    mov rdx, rdi
    in al, dx
    ret

write_port:
    mov rdx, rdi
    mov rax, rsi
    out dx, al
    ret

long_mode_start:
    mov ax, 0
    mov ss, ax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    jmp kmain
    ret

load_idt:
    lidt [idtr]
    sti
    ret

setup_int:
    mov qword rax, int_handler
    mov [idt+49*16], ax
    mov word [idt+49*16+2], code_selector
    mov word [idt+49*16+4], 0x8e00
    shr rax, 16
    mov [idt+49*16+6], ax
    shr rax, 16
    mov [idt+49*16+8], rax
    ret

trigger_int:
    int 49
    ret