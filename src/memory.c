#include "memory.h"

#define PAGE_SIZE 1024

unsigned int page_directory[PAGE_SIZE] __attribute__((aligned(4096)));

unsigned int first_page_table[PAGE_SIZE] __attribute__((aligned(4096)));

extern void load_page_directory(unsigned int *);
extern void enable_paging(void);

void init_page_entry(unsigned int *entry, unsigned int addr, unsigned char flags)
{
    *entry = (addr * 0x1000) | flags;
}

void init_paging()
{
    // Init page directory - all have present = 0
    for (unsigned int i = 0; i < PAGE_SIZE; i++)
    {
        init_page_entry(&page_directory[i], 0, 0b10);
    }

    // Map first page - identity map first 4MiB
    for (unsigned int i = 0; i < PAGE_SIZE; i++)
    {
        init_page_entry(&first_page_table[i], i, 0b11);
    }

    page_directory[0] = ((unsigned int)first_page_table) | 0b11;
    page_directory[PAGE_SIZE - 1] = ((unsigned int)page_directory) | 0b11;

    load_page_directory(page_directory);
    enable_paging();
}

void *get_physaddr(void *virt_addr)
{
    unsigned long pdindex = (unsigned long)virt_addr >> 22;
    unsigned long ptindex = (unsigned long)virt_addr >> 12 & 0x03FF;

    unsigned long *pd = (unsigned long *)0xFFFFF000;

    unsigned long *pt = (unsigned long *)0xFFC00000 + (0x400 * pdindex);

    return (void *)((pt[ptindex] & ~0xFFF) + ((unsigned long)virt_addr & 0xFFF));
}

void map_page(void *phys_addr, void *virt_addr, unsigned int flags)
{
    unsigned long pdindex = (unsigned long)virt_addr >> 22;
    unsigned long ptindex = (unsigned long)virt_addr >> 12 & 0x03FF;

    unsigned long *pd = (unsigned long *)0xFFFFF000;

    unsigned long *pt = (unsigned long *)0xFFC00000 + (0x400 * pdindex);

    pt[ptindex] = ((unsigned long)phys_addr) | (flags & 0xff) | 0x01;
}