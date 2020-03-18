#ifndef FAT_H
#define FAT_H

#include "types.h"
#include "lba.h"

typedef struct fat32_boot_sector
{
    u8 jump_inst[3];
    u8 oem_id[8];
    u16 bytes_per_sector;
    u8 sectors_per_cluster;
    u16 reserved_sectors;
    u8 fat_copies;
    u32 not_used_1;
    u8 media_descriptor;
    u16 not_used_2;
    u16 sectors_per_track;
    u16 heads;
    u32 hidden_sectors;
    u32 sectors_in_partition;
    u32 sectors_per_fat;
    u16 flags;
    u16 fat_version;
    u32 root_cluster;
    u16 file_system_information_sector;
    u16 backup_boot_sector;
    u8 reserved[12];
    u8 logical_drive_number;
    u8 current_head;
    u8 signature;
    u32 id;
    u8 volume_label[11];
    char system_id[8];
    u8 code[420];
    u16 sig;
} __attribute__((__packed__)) fat32_boot_sector_t;

typedef struct fat32_fs_info
{
    u32 lead_signature;
    u8 reserved[480];
    u32 signature;
    u32 free_cluster_count;
    u32 available_cluster_hint;
    u8 reserved_2[12];
    u32 trail_signature;
} __attribute__((__packed__)) fat32_fs_info_t;

typedef struct fat32_fat
{
    u8 entries[SECTOR_SIZE];
} __attribute__((__packed__)) fat32_fat_t;

void get_header(u8 drive_number, fat32_boot_sector_t *header);

void display_header(u8 drive_number);

#endif