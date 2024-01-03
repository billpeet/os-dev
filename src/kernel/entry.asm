; 32-bit entry point
; Sets up initial GDT and page tables
bits 32                                     ; nasm directive - 32 bit

; multiboot header
section .multiboot_header
header_start:
    ;multiboot spec
    dd 0xe85250d6                           ; magic number
    dd 0x00                                 ; flags
    dd header_end - header_start            ; header length
    dd 0x100000000 - \
    (0xe85250d6 + header_end - header_start); checksum - magic + flags + c should be 0

    dw 0
    dw 0
    dd 8
header_end:

section .text

global start
global code_selector
global gdt64
extern long_mode_start
extern error

; run through basic checks
check_multiboot:                            ; Checks if was loaded by a Multiboot compliant bootloader
    cmp eax, 0x36d76289                     ; Check that magic number 0x36d76289 is loaded into eax
    jne .no_multiboot
    ret

.no_multiboot:
    mov eax, 1
    push eax
    call error

check_cpuid:                                ; Checks that CPUID is supported by attempting to flip ID bit (21) in FLAGS
    pushfd                                  ; Copy FLAGS into eax
    pop eax

    mov ecx, eax                            ; Save to ecx for later

    xor eax, 1 << 21                        ; Flip ID bit

    push eax                                ; Poke back into FLAGS
    popfd

    pushfd                                  ; Grab current result from FLAGS
    pop eax

    push ecx                                ; Restore FLAGS from old version in ecx
    popfd

    cmp eax, ecx                            ; Check that the bit was flipped - if not, CPUID isn't supported
    je .no_cpuid
    ret
.no_cpuid:
    mov eax, 1
    push eax
    call error

check_long_mode:                            ; Check if 64-bit mode is available
    ; Check if extended processor info is available
    mov eax, 0x80000000
    cpuid                                   ; Returns highest supported argument
    cmp eax, 0x8000001                      ; Highest support argument needs to be at least 0x80000001
    jb .no_long_mode

    ; Check if long mode is available
    mov eax, 0x80000001                     ; Argument for extended processor info
    cpuid
    test edx, 1 << 29                       ; Test if LM-bit is test in the D-register (30th bit)
    jz .no_long_mode
    ret
.no_long_mode:
    mov eax, 2
    push eax
    call error

set_up_page_tables:
    ; map first P4 entry to P3 table
    mov eax, p3_table
    or eax, 0b11                            ; present + writable
    mov [p4_table], eax

    ; recursive page map
    mov eax, p4_table
    or eax, 0b11
    mov [p4_table + 511 * 8], eax

    ; map first P3 entry to P2 table
    mov eax, p2_table
    or eax, 0b11                            ; present + writable
    mov [p3_table], eax

    ; map each P2 entry to a huge 2MiB page
    mov ecx, 0                              ; counter variable

.map_p2_table:
    ; map ecx-th P1 entry to a huge page that starts at address 2MiB*ecx
    mov eax, 0x200000                       ; 2MiB
    mul ecx                                 ; start address of ecx-th page
    or eax, 0b10000011                      ; present + writable
    mov [p2_table + ecx*8], eax             ; map ecx-th entry

    inc ecx                                 ; increase counter
    cmp ecx, 512                            ; if counter == 512, the whole P2 table is mapped
    jne .map_p2_table                       ; else map the next entry

    ret

enable_paging:
    mov eax, p4_table
    mov cr3, eax                            ; set cr3 with P4 table address

    mov eax, cr4
    or eax, 1 << 5
    mov cr4, eax                            ; enable PAE flag of cr4

    mov ecx, 0xC0000080
    rdmsr
    or eax, 1 << 8
    wrmsr                                   ; set long mode bit in the EFER MSR

    mov eax, cr0
    or eax, 1 << 31
    mov cr0, eax                            ; set cr0

    ret

start:
    cli                                     ; block all interrupts
    mov esp, stack_top                      ; set stack pointer
    mov edi, ebx

    call check_multiboot                    ; check this has been loaded by a GRUB/multiboot bootloader
    call check_cpuid                        ; check CPUID flags are supported
    call check_long_mode                    ; check long mode is supported

    call set_up_page_tables                 ; set up P4, P3, P2 page tables to map first 
    call enable_paging                      ; enable paging and enter long mode
    lgdt [gdt64.pointer]                    ; load GDT

    jmp code_selector:long_mode_start       ; long jump to 64-bit code
    hlt                                     ; halt

section .rodata

; GDT
gdt64:
    dq 0                                    ; initial blank entry
code_selector: equ $ - gdt64
    dq (1<<43) | (1<<44) | (1<<47) | (1<<53); code segment - bits 43 (code), 44 (descriptor), 47 (present) and 53 (64 bit) set
    dq (1<<44) | (1<<47) | (1 << 53)        ; data segment - bits 44 (descriptor), 47 (present) and 53 (64 bit) set
    dq 0
    dq 0
.pointer:
    dw $ - gdt64 - 1
    dq gdt64

section .bss

; paging table
align 4096
p4_table:
    resb 4096
p3_table:
    resb 4096
p2_table:
    resb 4096

stack_bottom:
; kernel stack goes here
resb 8192                                   ; 8KB for stack
stack_top: