#include "gdt.h"
#include <stddef.h>
#include <string.h>
#include <stdio.h>

tss_entry_t tss_entry;

void dump_entry(gdt_entry_t *entry)
{
    uint32_t base, limit;
    base = entry->base_high << 24 | entry->base_low;
    limit = entry->limit_high << 16 | entry->limit_low;
    printf("base: 0x%x, limit 0x%x\n", base, limit);
}

void write_tss(gdt_entry_t *entry)
{
    size_t base = (size_t)&tss_entry;
    size_t limit = sizeof(tss_entry);

    entry->limit_low = limit & 0xFFFF;
    entry->base_low = base & 0xFFFFFF;           // Isolate bottom 24 bits
    entry->accessed = 1;                         // Indicates its a TSS not an LDT
    entry->read_write = 0;                       // Not busy
    entry->conforming_expand_down = 0;           // Always 0 for TSS
    entry->code = 1;                             // 1 for 32 bit, 0 for 16 bit
    entry->descriptor = 0;                       // Indicates this is a TSS
    entry->DPL = 3;                              // Ring 3
    entry->present = 1;                          // Present
    entry->limit_high = (limit & 0xF0000) >> 16; // Isolate top nibble
    entry->always_0 = 0;
    entry->islong = 0;
    entry->big = 0;
    entry->gran = 1;                              // GDT limit in bytes, not pages
    entry->base_high = (base & 0xFF000000) >> 24; // Top byte

    memset(&tss_entry, 0, sizeof(tss_entry)); // Fill with zeroes

    tss_entry.rsp0 = 0;
}

static inline void flush_tss()
{
    asm volatile(
        "mov 0x2B, %ax\n\t"
        "ltr %ax\n\t");
}

void init_ring3()
{
    gdt_entry_t *code;
    gdt_entry_t *data;

    code = &gdt64[3];
    data = &gdt64[4];

    code->limit_low = 0xFFFF;
    code->base_low = 0;
    code->accessed = 0;
    code->read_write = 1; // Readable for code segments
    code->conforming_expand_down = 0;
    code->code = 1; // Code segment
    code->descriptor = 1;
    code->DPL = 3; // Ring 3
    code->present = 1;
    code->limit_high = 0xF;
    code->always_0 = 0;
    code->islong = 1; // 64 bit
    code->big = 0;
    code->gran = 1; // page addressing
    code->base_high = 0;

    *data = *code;  // Copy from code - most of it is the same
    data->code = 0; // Change to data type

    write_tss(&gdt64[5]);

    flush_tss();
}