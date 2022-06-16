#ifndef GDT_H
#define GDT_H

#include <stdint.h>
#include "gcc-attributes.h"

typedef struct gdt_entry
{
    uint16_t limit_low;                           // Bottom 2 bytes of limit
    unsigned int base_low : 24;              // Bottom 3 bytes of base
    unsigned int accessed : 1;               // set to 1 by CPU when accessed
    unsigned int read_write : 1;             // 1 = readable for code, 1 = writable for data
    unsigned int conforming_expand_down : 1; // conforming for code, expand down for data
    unsigned int code : 1;                   // 1 for code, 0 for data
    unsigned int descriptor : 1;             // descriptor type - 1 for code/data segments, 0 for TSS
    unsigned int DPL : 2;                    // privilege level
    unsigned int present : 1;                // 1 if entry is present
    unsigned int limit_high : 4;             // top nibble of limit
    unsigned int always_0 : 1;               //
    unsigned int islong : 1;                 // 1 for 64-bit code descriptor
    unsigned int big : 1;                    // 32 bit opcodes for code, uint32_t stack for data
    unsigned int gran : 1;                   // 1 to use 4k page addressing, 0 for byte addressing
    unsigned int base_high : 8;              // Top byte of base
} PACKED gdt_entry_t;

typedef struct tss_entry_32
{
    uint32_t link; // Previous TSS
    uint32_t esp0; // Stack pointer
    uint32_t ss0;  // Stack segment
    uint32_t esp1, ss1;
    uint32_t esp2, ss2;
    uint32_t cr3, eip, eflags, eax, ecx, edx, ebx, esp, ebp, esi, edi, es, cs, ss, ds, fs, gs, ldt;
    uint16_t trap, iomap_base;
} PACKED tss_entry_32_t;

typedef struct tss_entry
{
    uint32_t reserved_0;  // Reserved
    uint64_t rsp0;        // Stack pointer
    uint64_t rsp1;        // Stack pointer
    uint64_t rsp2;        // Stack pointer
    uint64_t reserved_1;  // Reserved
    uint64_t ist1;        // IST1
    uint64_t ist2;        // IST2
    uint64_t ist3;        // IST3
    uint64_t ist4;        // IST4
    uint64_t ist5;        // IST5
    uint64_t ist6;        // IST6
    uint64_t ist7;        // IST7
    uint64_t reserved_2;  // Reserved
    uint16_t reserved_3;  // Reserved
    uint16_t iopb_offset; // IOPB offset
} PACKED tss_entry_t;

extern gdt_entry_t *gdt64;

extern void dump_entry(gdt_entry_t *entry);

#endif