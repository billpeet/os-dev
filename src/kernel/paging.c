#include "paging.h"
#include "stdio.h"
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

void *get_page_table_entry(page_table_t *page_table, size_t id, size_t *addr, u8 *flags)
{
    size_t entry = page_table->entries[id];
    *addr = (entry >> 8) << 8;
    *flags = entry & 0xFF;
}

page_table_t *get_page_table(page_table_t *page_table, size_t id)
{
    size_t entry = page_table->entries[id];
    if (!(entry & 0b1))
        panic("Page table %u does not exist!\n", id);
    return (page_table_t *)((entry >> 8) << 8);
}

page_table_t *create_table(page_table_t *parent, size_t index)
{
    if (!(parent->entries[index] & 0b1))
    {
        // Table doesn't exist - create one
        size_t frame = (size_t)allocate_frame(); // grab a new frame for the new table
        parent->entries[index] = frame | 0b11;   // present & writable flags

        // Zero out new table
        page_table_t *new_table = (page_table_t *)frame;
        for (int i = 0; i < ENTRY_COUNT; i++)
        {
            new_table->entries[i] = 0;
        }
    }
    return (page_table_t *)((parent->entries[index] >> 8) << 8);
}

void translate_address(void *virt_addr, size_t *p4_index, size_t *p3_index, size_t *p2_index, size_t *p1_index, size_t *offset)
{
    if ((size_t)virt_addr >= 0x0000800000000000 && (size_t)virt_addr < 0xffff800000000000)
        // Invalid address - bits 48-64 must be the same as the 47th bit!
        panic("Invalid address %p\n", virt_addr);

    *offset = (size_t)virt_addr % PAGE_SIZE;
    size_t virt_page_addr = (size_t)virt_addr / PAGE_SIZE;

    *p4_index = ((size_t)virt_page_addr >> 9 >> 9 >> 9) & 0777;
    *p3_index = ((size_t)virt_page_addr >> 9 >> 9) & 0777;
    *p2_index = ((size_t)virt_page_addr >> 9) & 0777;
    *p1_index = ((size_t)virt_page_addr) & 0777;

    // printf("Transforming virt_addr ");
    // writeInt((size_t)virt_addr);
    // printf(": P4 index ");
    // writeInt(*p4_index);
    // printf(", P3 index ");
    // writeInt(*p3_index);
    // printf(", P2 index ");
    // writeInt(*p2_index);
    // printf(", P1 index ");
    // writeInt(*p1_index);
    // printf(", offset ");
    // writeInt(*offset);
    // printf("\n");
}

size_t get_physaddr(void *virt_addr)
{
    size_t p4_index;
    size_t p3_index;
    size_t p2_index;
    size_t p1_index;
    size_t offset;
    translate_address(virt_addr, &p4_index, &p3_index, &p2_index, &p1_index, &offset);

    page_table_t *p4 = level_4_table();
    page_table_t *p3 = get_page_table(p4, p4_index);
    page_table_t *p2 = get_page_table(p3, p3_index);

    size_t addr;
    u8 flags;
    get_page_table_entry(p2, p2_index, &addr, &flags);
    if (!(flags & 0b1))
        panic("Page entry not present!\n");

    if (flags & 0x80)
    {
        // 4MiB page - return this
        offset += p1_index << 12;
        return addr + offset;
    }

    page_table_t *p1 = (page_table_t *)addr;
    get_page_table_entry(p1, p1_index, &addr, &flags);
    if (!(flags & 0b1))
        panic("Page entry not present!\n");

    return addr + offset;
}

void identity_map(void *virt_addr, u8 flags)
{
    map_page((size_t)virt_addr, virt_addr, flags);
}

void map_page(size_t phys_addr, void *virt_addr, u8 flags)
{
    size_t p4_index;
    size_t p3_index;
    size_t p2_index;
    size_t p1_index;
    size_t offset;
    translate_address(virt_addr, &p4_index, &p3_index, &p2_index, &p1_index, &offset);

    page_table_t *p4 = level_4_table();
    page_table_t *p3 = create_table(p4, p4_index);
    page_table_t *p2 = create_table(p3, p3_index);
    if (flags & 0x80)
    {
        if (phys_addr & 63)
            panic("Invalid physical frame - must be 4096 byte aligned!\n");

        p2->entries[p2_index] = (size_t)phys_addr | flags | 0x01;
    }
    else
    {
        if (phys_addr & 7)
            panic("Invalid physical frame - must be 4096 byte aligned!\n");

        page_table_t *p1 = create_table(p2, p2_index);
        p1->entries[p1_index] = phys_addr | flags | 0x01;
    }
}

void unmap_page(void *virt_addr)
{

    size_t p4_index;
    size_t p3_index;
    size_t p2_index;
    size_t p1_index;
    size_t offset;
    translate_address(virt_addr, &p4_index, &p3_index, &p2_index, &p1_index, &offset);

    page_table_t *p4 = level_4_table();
    page_table_t *p3 = get_page_table(p4, p4_index);
    page_table_t *p2 = get_page_table(p3, p3_index);

    page_table_t *p1 = get_page_table(p2, p2_index);
    p1->entries[p1_index] = 0; // reset to 0 (present bit = 0)

    flush_tlb();
}

void *allocate_page()
{
    size_t phys = (size_t)allocate_frame();
    size_t virt = phys;
    map_page(phys, (void *)virt, 0b10);
    return (void *)virt;
}