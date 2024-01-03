#ifndef FAT_H
#define FAT_H

#include <stdint.h>
#include <stdlib.h>
#include "drivers/ata.h"
#include "gcc-attributes.h"

typedef struct fat32_boot_sector
{
    uint8_t jump_inst[3];          // 0x00 (0)
    uint8_t oem_id[8];             // 0x03 (3)
    uint16_t bytes_per_sector;     // 0x0B (11)
    uint8_t sectors_per_cluster;   // 0x0D (13)
    uint16_t reserved_sectors;     // 0x0E (14)
    uint8_t fat_copies;            // 0x10 (16)
    uint32_t not_used_1;           // 0x11 (17)
    uint8_t media_descriptor;      // 0x15 (21)
    uint16_t not_used_2;           // 0x16 (22)
    uint16_t sectors_per_track;    // 0x18 (24)
    uint16_t heads;                // 0x1A (26)
    uint32_t hidden_sectors;       // 0x1C (28)
    uint32_t sectors_in_partition; // 0x20 (32)
    uint32_t sectors_per_fat;      // 0x24 (36)
    uint16_t flags;                // 0x28 (42)
    uint16_t fat_version;          // 0x2A (44)
    uint32_t root_cluster;         // 0x2C (46)
    uint16_t fsi_sector;           // 0x30 (50)
    uint16_t backup_boot_sector;   // 0x32 (52)
    uint8_t reserved[12];          // 0x34 (54)
    uint8_t logical_drive_number;  // 0x40 (64)
    uint8_t current_head;          // 0x41 (65)
    uint8_t signature;             // 0x42 (66)
    uint32_t id;                   // 0x43 (67)
    uint8_t volume_label[11];      // 0x47 (71)
    char system_id[8];        // 0x52 (82)
    uint8_t code[420];             // 0x5A (90)
    uint16_t sig;                  // 0x1FE (510)
} PACKED fat32_boot_sector_t;

typedef struct fat32_fs_info
{
    uint32_t lead_signature;
    uint8_t reserved[480];
    uint32_t signature;
    uint32_t free_cluster_count;
    uint32_t available_cluster_hint;
    uint8_t reserved_2[12];
    uint32_t trail_signature;
} PACKED fat32_fs_info_t;

typedef struct fat32_entry
{
    uint8_t filename[8];
    uint8_t extension[3];        // 0x08 (8)
    uint8_t flags;               // 0x0B (12)
    uint8_t unused[8];           // 0x0C (13)
    uint16_t first_cluster_high; // 0x14 (20)
    uint16_t time;               // 0x16 (22)
    uint16_t date;               // 0x18 (24)
    uint16_t first_cluster_low;  // 0x1A (26)
    uint32_t file_size;          // 0x1C (28)
} PACKED fat32_entry_t;

typedef struct fat32_directory
{
    uint8_t drive_number;
    char path[100];
    uint32_t cluster;
    fat32_entry_t *entries;
} fat32_directory_t;

int init_drive(uint32_t drive_number);

void read_directory(uint8_t drive_number, int cluster_number, const char *path, fat32_directory_t *dir);
void read_root_directory(uint8_t drive_number, fat32_directory_t *dir);

uint32_t get_cluster_number(fat32_entry_t *f);

void dump_directory_windows(fat32_directory_t *dir);
void dump_directory_unix(fat32_directory_t *dir);

int load_entry(fat32_directory_t *dir, uint32_t *fat, char *path, fat32_entry_t *entry);

int read_file(fat32_entry_t *entry, uint32_t *fat, uint8_t drive_number, void *buffer, uint32_t bytes_to_read);

uint32_t create_file(fat32_directory_t *dir, char *name, char *ext, uint32_t file_size, uint8_t flags);

int write_file(fat32_directory_t *dir, char *file_name, uint8_t *buffer, uint32_t size);

#endif