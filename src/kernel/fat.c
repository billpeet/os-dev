#include "fat.h"
#include "vga.h"
#include "frame_allocator.h"
#include "alloc.h"
#include <string.h>
#include "kernel.h"

#define SECTORS_PER_FAT 1539

fat32_boot_sector_t last_header;
u32 last_drive_number;
u32 fat_cache[(SECTORS_PER_FAT * SECTOR_SIZE)];

void get_header(u8 drive_number, fat32_boot_sector_t *header)
{
    read_sectors_lba(drive_number, 0, 1, (lba_sector_t *)header);
}

void read_fat(u32 *fat)
{
    for (u32 i = 0; i < SECTORS_PER_FAT; i++)
        // should be reserved sectors
        read_sectors_lba(last_drive_number, 32 + i, 1, (lba_sector_t *)fat + i);
}

int load_header(u32 drive_number)
{
    if (last_drive_number != drive_number || last_header.jump_inst[0] == 0)
    {
        // Load new header into cache
        read_fat(fat_cache);
        get_header(drive_number, &last_header);

        if (last_header.sig != 0xaa55)
        {
            printf("Invalid FAT disk!\n");
            return 0;
        }
    }
    return 1;
}

void dump_header(fat32_boot_sector_t *header)
{
    printf("Jump inst: ");
    for (u32 i = 0; i < 3; i++)
        printf("%x", header->jump_inst[i]);
    printf("\nOEM Id: ");
    for (u32 i = 0; i < 8; i++)
        writeChar(header->oem_id[i]);
    printf("\nBytes per sector: %u\n", header->bytes_per_sector);
    printf("Sectors per cluster: %u\n", header->sectors_per_cluster);
    printf("Reserved sectors: %u\n", header->reserved_sectors);
    printf("FAT copies: %u\n", header->fat_copies);
    printf("Media descriptor: %x\n", header->media_descriptor);
    printf("Sectors per track: %u\n", header->sectors_per_track);
    printf("Heads: %u\n", header->heads);
    printf("Hidden sectors: %u\n", header->hidden_sectors);
    printf("Sectors in partition: %u\n", header->sectors_in_partition);
    printf("Sectors per FAT: %u\n", header->sectors_per_fat);
    printf("Flags: %x\n", header->flags);
    printf("FAT version: %u\n", header->fat_version);
    printf("Root cluster: %x\n", header->root_cluster);
    printf("FSI sector: %x\n", header->fsi_sector);
    printf("Backup boot sector: %x\n", header->backup_boot_sector);
    printf("Logical drive number: %x\n", header->logical_drive_number);
    printf("Current head: %u\n", header->current_head);
    printf("Signature: %x\n", header->signature);
    printf("ID: %u\n", header->id);
    printf("Volume label: ");
    for (u32 i = 0; i < 11; i++)
        writeChar(header->volume_label[i]);
    printf("\nSystem ID: ");
    for (u32 i = 0; i < 8; i++)
        writeChar(header->system_id[i]);
    printf("\nFinal signature: %x\n", header->sig);
}

int is_directory(fat32_entry_t *entry)
{
    return entry->flags & 0b10000;
}

int is_long_name(fat32_entry_t *entry)
{
    return !(~entry->flags & 0b1111);
}

void dump_fat32_entry(fat32_entry_t *entry)
{
    // printf("flags: %u ", entry->flags);
    writeChar(entry->flags & 0b10000 ? 'd' : '-');
    writeChar(entry->flags & 0b100000 ? 'a' : '-');
    writeChar(entry->flags & 0b1000 ? 'v' : '-');
    writeChar(entry->flags & 0b100 ? 's' : '-');
    writeChar(entry->flags & 0b10 ? 'h' : '-');
    writeChar(entry->flags & 0b1 ? 'r' : '-');
    printf("     ");

    for (u32 i = 0; i < 8; i++)
    {
        if (entry->filename[i] == ' ')
            break;
        writeChar(entry->filename[i]);
    }

    if (entry->extension[0] != ' ')
    {
        writeChar('.');
        for (u32 i = 0; i < 3; i++)
        {
            if (entry->extension[i] == ' ')
                break;
            writeChar(entry->extension[i]);
        }
    }
    writeChar('\n');
}

fat32_entry_t *read_directory(u8 drive_number, int cluster_number)
{
    if (!load_header(drive_number))
        return NULLPTR;

    u32 cluster_begin = last_header.reserved_sectors + (last_header.fat_copies * last_header.sectors_per_fat);

    if (cluster_number < 2)
        cluster_number = last_header.root_cluster;

    u32 sector_number = cluster_begin + (cluster_number - 2) * last_header.sectors_per_cluster;

    lba_sector_t *cluster = malloc(sizeof(lba_sector_t) * last_header.sectors_per_cluster);
    read_sectors_lba(drive_number, sector_number, last_header.sectors_per_cluster, cluster);

    return (fat32_entry_t *)cluster;
}

fat32_entry_t *read_root_directory(u8 drive_number)
{
    return read_directory(drive_number, 0);
}

// Windows-style 'dir' command
void dump_directory(fat32_entry_t *dir)
{
    if (dir == NULLPTR)
        return;
    writeChar('\n');
    printf("Mode       Name\n");
    printf("----       ----\n");

    for (u32 i = 0; dir[i].filename[0]; i++)
    {
        if (is_directory(&dir[i]) && !is_long_name(&dir[i]))
            dump_fat32_entry(dir + i);
    }

    for (u32 i = 0; dir[i].filename[0]; i++)
    {
        if (!is_directory(&dir[i]) && !is_long_name(&dir[i]))
            dump_fat32_entry(dir + i);
    }

    writeChar('\n');
}

u32 get_cluster_number(fat32_entry_t *f)
{
    return (f->first_cluster_high << 16) | f->first_cluster_low;
}

fat32_entry_t *find_entry(fat32_entry_t *current_dir, char *entry_name, u8 dir)
{
    for (u32 i = 0; current_dir[i].filename[0]; i++)
    {
        char filename[13];
        for (u32 j = 0; j < 8; j++)
        {
            if (current_dir[i].filename[j] == ' ')
            {
                filename[j] = '.';
                j++;
                for (u32 k = 0; k < 3; k++)
                {
                    if (current_dir[i].extension[k] == ' ')
                    {
                        filename[j] = '\0';
                        break;
                    }
                    filename[j + k] = current_dir[i].extension[k];
                }
                break;
            }
            filename[j] = current_dir[i].filename[j];
        }

        if (!(current_dir[i].flags & 0b10000) == !dir && !strcasecmp(entry_name, filename))
            return &current_dir[i];
    }
    return NULLPTR;
}

fat32_entry_t *find_sub_directory(fat32_entry_t *current_dir, char *subdir_name)
{
    return find_entry(current_dir, subdir_name, 1);
}

void *read_cluster(u32 drive_number, u32 cluster_number, u32 sector_count, void *buffer)
{
    printf("Reading %u sectors from cluster %u\n", sector_count, cluster_number);
    u32 cluster_begin = last_header.reserved_sectors + (last_header.fat_copies * last_header.sectors_per_fat);

    u32 sector_number = cluster_begin + (cluster_number - 2) * last_header.sectors_per_cluster;

    printf("Reading sector %u\n", sector_number);
    read_sectors_lba(drive_number, sector_number, sector_count, buffer);
}

void *read_file(u8 drive_number, fat32_entry_t *current_dir, char *file_name, u32 *size)
{
    if (!load_header(drive_number))
        return NULLPTR;

    fat32_entry_t *entry = find_entry(current_dir, file_name, drive_number);
    if (entry == NULLPTR)
        return NULLPTR;
    *size = entry->file_size;

    u32 cluster_number = get_cluster_number(entry);
    printf("Cluster %u\n", cluster_number);

    u32 cluster_begin = last_header.reserved_sectors + (last_header.fat_copies * last_header.sectors_per_fat);

    u32 sector_number = cluster_begin + (cluster_number - 2) * last_header.sectors_per_cluster;
    printf("Sector %u\n", sector_number);

    u64 alloc_size = *size - (*size % sizeof(lba_sector_t)) + sizeof(lba_sector_t);
    printf("Alloc size %u\n", alloc_size);

    u32 total_sectors = alloc_size / sizeof(lba_sector_t);
    printf("Total sectors: %u\n", total_sectors);

    lba_sector_t *alloc = malloc(alloc_size);

    u64 offset = 0;
    while (offset < total_sectors && cluster_number != 0xFFFFFFFF)
    {
        u32 sectors_in_cluster = last_header.sectors_per_cluster > 1 ? total_sectors - offset : 1;
        if (sectors_in_cluster > last_header.sectors_per_cluster)
            sectors_in_cluster = last_header.sectors_per_cluster;

        read_cluster(drive_number, cluster_number, sectors_in_cluster, alloc + offset);
        if (cluster_number < (SECTORS_PER_FAT * SECTOR_SIZE))
        {
            printf("FAT value at cluster %u: %u\n", cluster_number, fat_cache[cluster_number]);
            cluster_number = fat_cache[cluster_number];
        }
        else
        {
            printf("Cluster %u out of range!\n", cluster_number);
            panic(0);
        }

        offset += 3;
    }

    return alloc;
}