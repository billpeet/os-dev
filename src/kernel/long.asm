global long_mode_start
global switch_task

section .text

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

switch_task:
    ; push rbx
    ; push rsi
    ; push rbp

    mov rsp, [rdi+6*8]      ; set stack pointer - return address should be already configured at [rsp]
    mov rbp, [rdi+7*8]      ; rbp
    mov rax, [rsi+11*8]     ; CR3
    
    cmp rax, rcx            ; Has the virtual space changed? If not, don't bother setting CR3
    je .doneVAS
    ; mov cr3, rax

.doneVAS:

    ; pop rbp
    ; pop rsi
    ; pop rbx

    ; mov rax, [rdi+8*8]     ; Load the return address into the new stack
    ; mov [rsp], rax
    mov rcx, [rsp]
    int3

    ret


switch_task_old:
    push rax                ; push all registers
    push rcx
    push rdx
    push rbx
    push rsp
    push rbp    
    push rsi
    push rdi
    pushf                   ; push flags
    mov rax, cr3            ; push cr3
    push rax
    mov rax, [rsp+88]       ; first argument - pointer to old task
    mov [rax+8], rbx        ; old.regs.rbx
    mov [rax+16], rcx       ; old.regs.rcx
    mov [rax+24], rdx       ; old.regs.rdx
    mov [rax+32], rsi       ; old.regs.rsi
    mov [rax+40], rdi       ; old.regs.rdi
    mov rbx, [rsp+72]       ; rax
    mov rcx, [rsp+80]       ; ip
    mov rdx, [rsp+40]       ; rsp
    add rdx, 8              ; remove return value from rsp!
    mov rsi, [rsp+32]       ; rbp
    mov rdi, [rsp+8]        ; flags
    pop rbx                 ; grab CR3 again
    mov [rax+80], rbx       ; poke into old.regs.cr3
    push rbx                ; poke CR3 into stack again

    mov rax, [rsp+96]       ; second argument - pointer to new task
    mov rbx, [rax+8]        ; new.regs.rbx
    mov rcx, [rax+16]       ; new.regs.rcx
    mov rdx, [rax+24]       ; new.regs.rdx
    mov rsi, [rax+32]       ; new.regs.rsi
    mov rdi, [rax+40]       ; new.regs.rdi
    ; mov rbp, [rax+56]       ; new.regs.rbp
    push rax
    mov rax, [rax+72]       ; flags
    push rax
    popf                    ; pop into flags
    pop rax
    ; mov rsp, [rax+48]       ; new.regs.rsp
    ; push rax
    mov rax, [rax+80]       ; new.regs.cr3
    mov cr3, rax
    pop rax
    push rax
    mov rax, [rax+64]       ; new.regs.rip
    xchg rax, [rsp]         ; 
    mov rax, [rax]          ; new.regs.rax

    pop rax                 ; clean out stack
    pop rax
    pop rax
    pop rax
    pop rax
    pop rax
    pop rax
    pop rax
    pop rax
    pop rax


    ret