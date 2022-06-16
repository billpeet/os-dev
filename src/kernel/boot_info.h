#ifndef BOOT_INFO_H
#define BOOT_INFO_H

#include <stddef.h>
#include <stdint.h>

struct boot_info
{
    uint32_t total_size;
    uint32_t _reserved;
};

struct boot_tag
{
    uint32_t type;
    uint32_t size;
};

struct memory_map_entry
{
    uint64_t base_addr;
    uint64_t length;
    uint32_t type;
    uint32_t reserved;
};

struct elf_section_entry
{
    uint32_t name;
    uint32_t type;
    uint64_t flags;
    uint64_t address;
    uint64_t file_offset;
    uint64_t size;
    uint32_t link;
    uint32_t info;
    uint64_t addr_align;
    uint64_t entry_size;
};

typedef struct boot_info boot_info_t;
typedef struct boot_tag boot_tag_t;
typedef struct memory_map_entry memory_map_entry_t;
typedef struct elf_section_entry elf_section_entry_t;

void init_boot_info(boot_info_t *boot_info);

extern size_t kernel_start;
extern size_t kernel_end;
extern size_t multiboot_start;
extern size_t multiboot_end;
extern memory_map_entry_t *memory_map;
extern size_t memory_map_count;

#endif