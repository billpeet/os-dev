#ifndef FAT_H
#define FAT_H

#include "types.h"
#include "ata.h"
#include "gcc-attributes.h"

typedef struct fat32_boot_sector
{
    u8 jump_inst[3];          // 0x00 (0)
    u8 oem_id[8];             // 0x03 (3)
    u16 bytes_per_sector;     // 0x0B (11)
    u8 sectors_per_cluster;   // 0x0D (13)
    u16 reserved_sectors;     // 0x0E (14)
    u8 fat_copies;            // 0x10 (16)
    u32 not_used_1;           // 0x11 (17)
    u8 media_descriptor;      // 0x15 (21)
    u16 not_used_2;           // 0x16 (22)
    u16 sectors_per_track;    // 0x18 (24)
    u16 heads;                // 0x1A (26)
    u32 hidden_sectors;       // 0x1C (28)
    u32 sectors_in_partition; // 0x20 (32)
    u32 sectors_per_fat;      // 0x24 (36)
    u16 flags;                // 0x28 (42)
    u16 fat_version;          // 0x2A (44)
    u32 root_cluster;         // 0x2C (46)
    u16 fsi_sector;           // 0x30 (50)
    u16 backup_boot_sector;   // 0x32 (52)
    u8 reserved[12];          // 0x34 (54)
    u8 logical_drive_number;  // 0x40 (64)
    u8 current_head;          // 0x41 (65)
    u8 signature;             // 0x42 (66)
    u32 id;                   // 0x43 (67)
    u8 volume_label[11];      // 0x47 (71)
    char system_id[8];        // 0x52 (82)
    u8 code[420];             // 0x5A (90)
    u16 sig;                  // 0x1FE (510)
} PACKED fat32_boot_sector_t;

typedef struct fat32_fs_info
{
    u32 lead_signature;
    u8 reserved[480];
    u32 signature;
    u32 free_cluster_count;
    u32 available_cluster_hint;
    u8 reserved_2[12];
    u32 trail_signature;
} PACKED fat32_fs_info_t;

typedef struct fat32_entry
{
    u8 filename[8];
    u8 extension[3];        // 0x08 (8)
    u8 flags;               // 0x0B (12)
    u8 unused[8];           // 0x0C (13)
    u16 first_cluster_high; // 0x14 (20)
    u16 time;               // 0x16 (22)
    u16 date;               // 0x18 (24)
    u16 first_cluster_low;  // 0x1A (26)
    u32 file_size;          // 0x1C (28)
} PACKED fat32_entry_t;

typedef struct fat32_directory
{
    u8 drive_number;
    u32 cluster;
    fat32_entry_t *entries;
} fat32_directory_t;

int init_drive(u32 drive_number);

fat32_directory_t read_directory(u8 drive_number, int cluster_number);
fat32_directory_t read_root_directory(u8 drive_number);

u32 get_cluster_number(fat32_entry_t *f);

void dump_directory(fat32_directory_t *dir);

fat32_entry_t *find_sub_directory(fat32_directory_t *dir, char *subdir_name);

void *read_file(fat32_directory_t *dir, char *file_name, u32 *size);

u32 create_file(fat32_directory_t *dir, char *name, char *ext, u32 file_size, u8 flags);

int write_file(fat32_directory_t *dir, char *file_name, u8 *buffer, u32 size);

#endif