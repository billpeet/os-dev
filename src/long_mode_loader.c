#include "types.h"

void error(unsigned char error_code)
{
    char *vidptr = (char *)0xb8000;
    char *str = "ERR: ";
    int i = 0;
    for (; str[i] != '\0'; i++)
    {
        vidptr[i * 2] = str[i];
        vidptr[i * 2 + 1] = 0x4f;
    }
    vidptr[i * 2] = error_code + 48;
    vidptr[i * 2 + 1] = 0x4f;

    asm("hlt");
}

u64 p4_table[512] __attribute__((aligned(4096)));
u64 p3_table[512] __attribute__((aligned(4096)));
u64 p2_table[512] __attribute__((aligned(4096)));

void tester()
{
    char *vidptr = (char *)0xb8000;

    u64 z;
    unsigned char length = sizeof(p4_table[0]);

    vidptr[101] = length + 48;
    vidptr[102] = 0x4f;
    // u8 z;
    // unsigned char length = sizeof(z);
    // vidptr[0] = length + 48;
    // vidptr[1] = 0x4f;
    // vidptr[2] = ' ';
    // vidptr[3] = 0x4f;

    // error(1);

    // error(0);

    //asm("hlt");
}

void init_page_tables()
{
    u64 p3_addr = (u64)p3_table;
    p3_addr |= 0b11; // Set bits 0 (present) & 1 (writable)
    p4_table[0] = p3_addr;

    u64 p2_addr = (u64)p2_table;
    p2_addr |= 0b11; // Set bits 0 (present) & 1 (writable)
    p3_table[0] = p2_addr;

    for (u32 i = 0; i < 512; i++)
    {
        u64 table_entry = i * 0x200000; // Start address - 2 MiB per page
        table_entry |= 0b10000011;      // Set bits 0 (present), 1 (writable) and 7 (huge)
        p2_table[i] = table_entry;
    }
}

// extern u64 gdt[1];

// struct gdt_addr_s
// {
//     u16 length;
//     u32 zero;
//     u32 ptr;
// } gdt_addr;

// void *gdt_selector;

// void init_gdt()
// {
//     gdt[0] = 0;
//     u64 one = 1;
//     gdt[1] = one << 43 | one << 44 | one << 47 | one << 53;
//     gdt_addr.length = 1; // 2 entries - 1
//     gdt_addr.zero = 0;
//     gdt_addr.ptr = (u32)gdt;
//     gdt_selector = &gdt[1];
// }