global long_mode_start
global switch_task

section .text

extern kmain
extern code_selector

long_mode_start:            ; start of long mode
    mov ax, 0               ; zero out registers
    mov ss, ax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    jmp kmain               ; jump to kmain in kernel.c
    ret

switch_task:
    ; push rbx
    ; push rsi
    ; push rbp

    mov rsp, [rdi+6*8]      ; set stack pointer - return address should be already configured at [rsp]
    mov rbp, [rdi+7*8]      ; rbp
    mov rax, [rdi+10*8]     ; CR3
    
    mov rcx, cr3            ; Load current cr3
    cmp rax, rcx            ; Has the virtual space changed? If not, don't bother setting CR3
    je .doneVAS

    mov cr3, rcx

.doneVAS:
    ; pop rbp
    ; pop rsi
    ; pop rbx
    ret