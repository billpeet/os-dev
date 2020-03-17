#include "paging.h"
#include "vga.h"
#include "frame_allocator.h"
#include "kernel.h"

void flush_tlb()
{
    asm volatile(
        "mov %cr3, %rax\n\t"
        "mov %rax, %cr3\n\t");
}

page_table_t *level_4_table()
{
    u64 cr3;
    asm volatile(
        "mov %%cr3, %%rax\n\t"
        "mov %%rax, %0\n\t"
        : "=m"(cr3)
        :
        : "rax");
    return (page_table_t *)cr3;
}

void *get_page_table_entry(page_table_t *page_table, u64 id, u64 *addr, u8 *flags)
{
    u64 entry = page_table->entries[id];
    *addr = (entry >> 8) << 8;
    *flags = entry & 0xFF;
}

page_table_t *get_page_table(page_table_t *page_table, u64 id)
{
    return (page_table_t *)((page_table->entries[id] >> 8) << 8);
}

page_table_t *create_table(page_table_t *parent, u64 index)
{
    if (!(parent->entries[index] & 0b1))
    {
        // Table doesn't exist - create one
        u64 frame = allocate_frame();          // grab a new frame for the new table
        parent->entries[index] = frame | 0b11; // present & writable flags

        // Zero out new table
        page_table_t *new_table = (page_table_t *)frame;
        for (u64 i = 0; i < ENTRY_COUNT; i++)
        {
            new_table->entries[i] = 0;
        }
    }
    return (page_table_t *)parent->entries[index];
}

void translate_address(void *virt_addr, u64 *p4_index, u64 *p3_index, u64 *p2_index, u64 *p1_index, u64 *offset)
{
    if ((u64)virt_addr >= 0x0000800000000000 && (u64)virt_addr < 0xffff800000000000)
    {
        // Invalid address - bits 48-64 must be the same as the 47th bit!
        writeString("Invalid address ");
        writeHexInt((u64)virt_addr);
        writeNewLine();
        panic(1);
    }

    *offset = (u64)virt_addr % PAGE_SIZE;
    u64 virt_page_addr = (u64)virt_addr / PAGE_SIZE;

    *p4_index = ((u64)virt_page_addr >> 9 >> 9 >> 9) & 0777;
    *p3_index = ((u64)virt_page_addr >> 9 >> 9) & 0777;
    *p2_index = ((u64)virt_page_addr >> 9) & 0777;
    *p1_index = ((u64)virt_page_addr) & 0777;

    writeString("Transforming virt_addr ");
    writeInt((u64)virt_addr);
    writeString(": P4 index ");
    writeInt(*p4_index);
    writeString(", P3 index ");
    writeInt(*p3_index);
    writeString(", P2 index ");
    writeInt(*p2_index);
    writeString(", P1 index ");
    writeInt(*p1_index);
    writeString(", offset ");
    writeInt(*offset);
    writeString("\n");
}

u64 get_physaddr(void *virt_addr)
{
    u64 p4_index;
    u64 p3_index;
    u64 p2_index;
    u64 p1_index;
    u64 offset;
    translate_address(virt_addr, &p4_index, &p3_index, &p2_index, &p1_index, &offset);

    page_table_t *p4 = level_4_table();
    page_table_t *p3 = get_page_table(p4, p4_index);
    page_table_t *p2 = get_page_table(p3, p3_index);

    u64 addr;
    u8 flags;
    get_page_table_entry(p2, p2_index, &addr, &flags);
    if (flags & 0x80)
    {
        // 4MiB page - return this
        writeString("offset before:");
        writeHexInt(offset);
        offset += p1_index << 12;
        writeString(", offset after:");
        writeHexInt(offset);
        writeNewLine();
        return addr + offset;
    }

    page_table_t *p1 = (page_table_t *)addr;
    get_page_table_entry(p1, p1_index, &addr, &flags);
    return addr + offset;
}

void map_page(u64 phys_addr, void *virt_addr, u8 flags)
{
    u64 p4_index;
    u64 p3_index;
    u64 p2_index;
    u64 p1_index;
    u64 offset;
    translate_address(virt_addr, &p4_index, &p3_index, &p2_index, &p1_index, &offset);

    page_table_t *p4 = level_4_table();
    page_table_t *p3 = create_table(p4, p4_index);
    page_table_t *p2 = create_table(p3, p3_index);
    if (flags & 0x80)
    {
        if (phys_addr & 63)
        {
            writeString("Invalid physical frame - must be 4096 byte aligned!\n");
            panic(2);
        }

        // Creating a 'huge' 4MiB page
        writeString("Mapping a huge page at P2 index ");
        writeInt(p2_index);
        writeNewLine();

        p2->entries[p2_index] = (u64)phys_addr | flags | 0x01;
    }
    else
    {
        if (phys_addr & 0b111)
        {
            writeString("Invalid physical frame - must be 4096 byte aligned!\n");
            panic(2);
        }

        writeString("Mapping a standard page at P1 index ");
        writeInt(p1_index);
        writeString(", to phys addr ");
        writeHexInt(phys_addr);
        writeString(", setting to ");
        writeHexInt(phys_addr | flags | 0x01);
        writeNewLine();

        page_table_t *p1 = get_page_table(p2, p2_index);
        p1->entries[p1_index] = phys_addr | flags | 0x01;
    }

    flush_tlb();
}
