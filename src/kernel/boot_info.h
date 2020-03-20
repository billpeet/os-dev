#ifndef BOOT_INFO_H
#define BOOT_INFO_H

#include "types.h"

struct boot_info
{
    u32 total_size;
    u32 _reserved;
};

struct boot_tag
{
    u32 type;
    u32 size;
};

struct memory_map_entry
{
    u64 base_addr;
    u64 length;
    u32 type;
    u32 reserved;
};

struct elf_section_entry
{
    u32 name;
    u32 type;
    u64 flags;
    u64 address;
    u64 file_offset;
    u64 size;
    u32 link;
    u32 info;
    u64 addr_align;
    u64 entry_size;
};

typedef struct boot_info boot_info_t;
typedef struct boot_tag boot_tag_t;
typedef struct memory_map_entry memory_map_entry_t;
typedef struct elf_section_entry elf_section_entry_t;

void init_boot_info(boot_info_t *boot_info);

extern u64 kernel_start;
extern u64 kernel_end;
extern u64 multiboot_start;
extern u64 multiboot_end;
extern memory_map_entry_t *memory_map;
extern u64 memory_map_count;

#endif