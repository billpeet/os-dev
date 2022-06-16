#include "fat.h"
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <ctype.h>
#include <stdio.h>
#include <stdbool.h>
#include "frame_allocator.h"
#include "kernel.h"

#define SECTORS_PER_FAT 1539
#define FAT_ENTRIES SECTORS_PER_FAT *SECTOR_SIZE / sizeof(uint32_t)
#define INTCEIL(x, y) (x + y - 1) / y
#define END_CLUSTER 0xFFFFFFF

#define FILENAME_LENGTH 8
#define EXTENSION_LENGTH 3

typedef struct fat32_disk_cache
{
    fat32_boot_sector_t boot_sector;
    uint32_t fat[FAT_ENTRIES];
    fat32_directory_t root_dir;
} fat32_disk_cache_t;

fat32_disk_cache_t fat32_disks[10];

// Gets string with '[FILENAME].[EXT]' from separate filename/ext fields
void get_filename(const char *filename, const char *ext, char *dest)
{
    int space = -1, i;

    strcpy(dest, filename);
    for (i = 0; i < 8; i++)
    {
        if (filename[i] == ' ')
        {
            if (space == -1)
                space = i;
        }
        else
            // non-space, reset last space pointer
            space = -1;
    }
    if (space == -1)
        space = 8;

    if (ext[0] == ' ')
    {
        // no extension
        dest[space] = '\0';
    }
    else
    {
        dest[space++] = '.';
        for (i = 0; i < 3; i++)
        {
            if (ext[i] == ' ')
                break;
            dest[i + space] = ext[i];
        }
        dest[i + space] = '\0';
    }
}

fat32_boot_sector_t *get_bs(uint8_t drive_number)
{
    return &fat32_disks[drive_number].boot_sector;
}

uint32_t *get_fat(uint8_t drive_number)
{
    return fat32_disks[drive_number].fat;
}

fat32_directory_t *get_root(uint8_t drive_number)
{
    return &fat32_disks[drive_number].root_dir;
}

// Reads boot sector header from disk
void read_header(uint8_t drive_number, fat32_boot_sector_t *header)
{
    read_sectors_ata(drive_number, 0, 1, (lba_sector_t *)header);
}

// Reads FAT from disk
void read_fat(uint8_t drive_number, uint32_t *fat)
{
    fat32_boot_sector_t *boot_sector = get_bs(drive_number);
    for (uint32_t i = 0; i < SECTORS_PER_FAT; i++)
        read_sectors_ata(drive_number, boot_sector->reserved_sectors + i, 1, (lba_sector_t *)fat + i);
}

// Saves cached FAT to disk
void save_fat(uint8_t drive_number)
{
    fat32_boot_sector_t *boot_sector = get_bs(drive_number);
    lba_sector_t *fat_sector = (lba_sector_t *)get_fat(drive_number);
    for (uint32_t i = 0; i < SECTORS_PER_FAT; i++)
        // should be reserved sectors
        write_sectors_ata(drive_number, boot_sector->reserved_sectors + i, 1, fat_sector + i);
}

// Reads boot sector header and FAT table into cache
int init_drive(uint32_t drive_number)
{
    // Load new header into cache
    fat32_disk_cache_t *disk_cache = &fat32_disks[drive_number];

    read_header(drive_number, &disk_cache->boot_sector);
    read_fat(drive_number, disk_cache->fat);
    read_directory(drive_number, 0, "/", &disk_cache->root_dir);

    if (disk_cache->boot_sector.sig != 0xaa55)
    {
        printf("Invalid FAT disk!\n");
        return 0;
    }
    return 1;
}

int is_directory(fat32_entry_t *entry)
{
    return entry->flags & 0b10000;
}

int is_long_name(fat32_entry_t *entry)
{
    return !(~entry->flags & 0b1111);
}

// Reads directory from disk
void read_directory(uint8_t drive_number, int cluster_number, const char *path, fat32_directory_t *dir)
{
    dir->drive_number = drive_number;

    fat32_boot_sector_t *boot_sector = get_bs(drive_number);

    uint32_t cluster_start = boot_sector->reserved_sectors + (boot_sector->fat_copies * boot_sector->sectors_per_fat);

    dir->cluster = cluster_number;
    if (dir->cluster < 2)
        dir->cluster = boot_sector->root_cluster;

    uint32_t sector_number = cluster_start + (dir->cluster - 2) * boot_sector->sectors_per_cluster;

    lba_sector_t *cluster = calloc(boot_sector->sectors_per_cluster, sizeof(lba_sector_t));
    read_sectors_ata(drive_number, sector_number, boot_sector->sectors_per_cluster, cluster);

    dir->entries = (fat32_entry_t *)cluster;
    strcpy(dir->path, path);
}

// Reads root directory to cache
void read_root_directory(uint8_t drive_number, fat32_directory_t *dir)
{
    read_directory(drive_number, 0, "/", dir);
}

void read_cluster(uint32_t drive_number, uint32_t cluster_number, void *buffer)
{
    // printf("Reading %u sectors from cluster %u\n", sector_count, cluster_number);
    fat32_boot_sector_t *boot_sector = get_bs(drive_number);
    uint32_t cluster_begin = boot_sector->reserved_sectors + (boot_sector->fat_copies * boot_sector->sectors_per_fat);
    uint32_t sector_number = cluster_begin + (cluster_number - 2) * boot_sector->sectors_per_cluster;
    // printf("Reading sector %u\n", sector_number);
    read_sectors_ata(drive_number, sector_number, boot_sector->sectors_per_cluster, buffer);
}

// Saves cached directory table to disk
void save_cluster(uint8_t drive_number, uint32_t cluster_number, void *ptr)
{
    fat32_boot_sector_t *boot_sector = get_bs(drive_number);
    uint32_t cluster_start = boot_sector->reserved_sectors + (boot_sector->fat_copies * boot_sector->sectors_per_fat);
    uint32_t sector_number = cluster_start + (cluster_number - 2) * boot_sector->sectors_per_cluster;
    write_sectors_ata(drive_number, sector_number, boot_sector->sectors_per_cluster, (lba_sector_t *)ptr);
}

// Saves cached directory table to disk
void save_directory(fat32_directory_t *dir)
{
    save_cluster(dir->drive_number, dir->cluster, dir->entries);
}

#pragma region dumps

// Dumps boot sector header info
void dump_header(fat32_boot_sector_t *header)
{
    printf("Jump inst: ");
    for (uint32_t i = 0; i < 3; i++)
        printf("%x", header->jump_inst[i]);
    printf("\nOEM Id: ");
    for (uint32_t i = 0; i < 8; i++)
        putchar(header->oem_id[i]);
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
    for (uint32_t i = 0; i < 11; i++)
        putchar(header->volume_label[i]);
    printf("\nSystem ID: ");
    for (uint32_t i = 0; i < 8; i++)
        putchar(header->system_id[i]);
    printf("\nFinal signature: %x\n", header->sig);
}

void dump_fat32_entry_verbose(fat32_entry_t *entry)
{
    printf("%s ", is_directory(entry) ? "dir" : "file");

    char filename[15];
    get_filename(entry->filename, entry->extension, filename);
    printf("'%s' ", filename);

    printf("flags: %u ", entry->flags);
    putchar(entry->flags & 0b10000 ? 'd' : '-');
    putchar(entry->flags & 0b100000 ? 'a' : '-');
    putchar(entry->flags & 0b1000 ? 'v' : '-');
    putchar(entry->flags & 0b100 ? 's' : '-');
    putchar(entry->flags & 0b10 ? 'h' : '-');
    putchar(entry->flags & 0b1 ? 'r' : '-');
}

// Dumps file info in Windows-style 'DIR' fashion
void dump_fat32_entry_win(fat32_entry_t *entry)
{
    // printf("flags: %u ", entry->flags);
    putchar(entry->flags & 0b10000 ? 'd' : '-');
    putchar(entry->flags & 0b100000 ? 'a' : '-');
    putchar(entry->flags & 0b1000 ? 'v' : '-');
    putchar(entry->flags & 0b100 ? 's' : '-');
    putchar(entry->flags & 0b10 ? 'h' : '-');
    putchar(entry->flags & 0b1 ? 'r' : '-');
    printf("     ");

    char filename[15];
    get_filename(entry->filename, entry->extension, filename);
    printf("%s\n", filename);
}

// Windows-style 'dir' command
void dump_directory_windows(fat32_directory_t *dir)
{
    if (dir->entries == NULL)
        return;
    putchar('\n');
    printf("Mode       Name\n");
    printf("----       ----\n");

    fat32_entry_t *entries = dir->entries;
    for (uint32_t i = 0; entries[i].filename[0]; i++)
    {
        if (is_directory(&entries[i]) && !is_long_name(&entries[i]))
            dump_fat32_entry_win(entries + i);
    }

    for (uint32_t i = 0; entries[i].filename[0]; i++)
    {
        if (!is_directory(&entries[i]) && !is_long_name(&entries[i]))
            dump_fat32_entry_win(entries + i);
    }

    putchar('\n');
}

void dump_fat32_entry_unix(fat32_entry_t *entry)
{
    char filename[15];
    get_filename(entry->filename, entry->extension, filename);
    printf("%s", filename);
}

void dump_directory_unix(fat32_directory_t *dir)
{
    if (dir->entries == NULL)
        return;

    fat32_entry_t *entries = dir->entries;
    for (int i = 0; entries[i].filename[0]; i++)
    {
        if (!is_long_name(&entries[i]))
        {
            if (i > 0)
                printf(" ");
            dump_fat32_entry_unix(entries + i);
        }
    }

    putchar('\n');
}

#pragma endregion

uint32_t get_cluster_number(fat32_entry_t *f)
{
    return (f->first_cluster_high << 16) | f->first_cluster_low;
}

// Finds directory entry with specified name
fat32_entry_t *find_entry(fat32_directory_t *current_dir, const char *entry_name)
{
    char filename[15];
    for (int i = 0; current_dir->entries[i].filename[0]; i++)
    {
        get_filename(current_dir->entries[i].filename, current_dir->entries[i].extension, filename);
        if (!strcasecmp(entry_name, filename))
            return &current_dir->entries[i];
    }
    return NULL;
}

// int load_sub_directory(fat32_directory_t *current_dir, char *subdir_name, fat32_directory_t *dest)
// {
//     if (current_dir == NULL)
//     {
//         read_directory(0, 0, "/", current_dir);
//     }

//     if (strcmp(subdir_name, ".") == 0)
//     {
//         // Just copy directory across - caller needs to make sure they check before freeing current_dir's entries!
//         *dest = *current_dir;
//         return 0;
//     }

//     int slash_pos = (int)(strchr(subdir_name, "/") - subdir_name);
//     if (slash_pos > 0)
//     {
//         char new_str[slash_pos + 1];
//         memcpy(new_str, subdir_name, slash_pos);
//         return load_sub_directory(current_dir, new_str, dest);
//     }

//     fat32_entry_t *entry = find_sub_directory(current_dir, subdir_name);
//     if (entry == NULL)
//         return -1;
//     uint32_t cluster_number = get_cluster_number(entry);

//     char path[100];
//     strcpy(path, current_dir->path);
//     if (subdir_name[0] == '.')
//     {
//         // current dir '.' or parent dir '..'
//         path[strlen(path) - 1] = '\0'; // Remove last slash
//         char *tmp = strrchr(path, '/');
//         tmp++;
//         *tmp = '\0';
//     }
//     else
//     {
//         // subdir name - add subdir and '\' to the end of the path
//         strcat(path, subdir_name);
//         strcat(path, "/");
//     }

//     read_directory(current_dir->drive_number, cluster_number, path, dest);
//     return 0;
// }

int load_entry(fat32_directory_t *dir, uint32_t *fat, char *path, fat32_entry_t *entry)
{
    if (dir == NULL)
    {
        // printf("loading root directory\n");
        read_directory(0, 0, "/", dir);
    }

    int slash_pos = (int)(strchr(path, '/') - path);
    if (slash_pos > 0)
    {
        // printf("slash pos %u\n", slash_pos);
        char new_str[slash_pos + 1];
        memcpy(new_str, path, slash_pos + 1);
        // new_str[slash_pos] = '\0';

        // printf("loading subdir '%s'\n", new_str);

        fat32_entry_t *subdir = find_entry(dir, new_str);
        if (subdir == NULL || !is_directory(subdir))
        {
            subdir = NULL;
            printf("could not find subdir '%s'\n", path);
            return -1;
        }

        char *following_path = path + slash_pos + 1;
        read_directory(dir->drive_number, get_cluster_number(subdir), following_path, dir);

        if (fat == NULL)
        {
            // printf("loading FAT\n");
            fat = get_fat(dir->drive_number);
        }

        int res = load_entry(dir, fat, following_path, entry);
        return res;
    }

    // printf("loading %s\n", path);
    fat32_entry_t *entry_ptr = find_entry(dir, path);
    if (entry_ptr == NULL)
    {
        return -1;
    }
    memcpy(entry, entry_ptr, sizeof(fat32_entry_t));
    return 0;
}

int read_file(fat32_entry_t *entry, uint32_t *fat, uint8_t drive_number, void *buffer, uint32_t bytes_to_read)
{
    bytes_to_read = bytes_to_read > entry->file_size ? entry->file_size : bytes_to_read;
    // printf("%u bytes to read\n", bytes_to_read);
    if (bytes_to_read == 0)
    {
        return 0;
    }

    size_t alloc_size = bytes_to_read;
    if (alloc_size % SECTOR_SIZE != 0)
        alloc_size += SECTOR_SIZE - alloc_size % SECTOR_SIZE;
    // printf("Alloc size %u\n", alloc_size);

    lba_sector_t *sector = malloc(sizeof(lba_sector_t));
    size_t bytes_read = 0;

    uint32_t cluster_number = get_cluster_number(entry);

    while (cluster_number != 0xFFFFFFF && bytes_read < bytes_to_read)
    {
        // printf("Reading cluster %u\n", cluster_number);
        if (cluster_number >= FAT_ENTRIES)
            panic("Cluster %u out of range!\n", cluster_number);
        read_cluster(drive_number, cluster_number, sector);
        cluster_number = fat[cluster_number];
        size_t b = sizeof(lba_sector_t) > bytes_to_read - bytes_read ? bytes_to_read - bytes_read : sizeof(lba_sector_t);
        memcpy((char *)buffer + bytes_read, sector, b + 1);
        bytes_read += b;
    }

    free(sector);
    return bytes_read;
}

// Finds spare disk cluster in FAT
int find_spare_cluster(uint8_t drive_number, uint8_t start)
{
    uint32_t *fat = get_fat(drive_number);
    for (int i = start; i < FAT_ENTRIES; i++)
    {
        if (fat[i] == 0)
            return i;
    }
    return -1;
}

// Finds spare entry in directory
fat32_entry_t *find_spare_entry(fat32_directory_t *dir)
{
    fat32_boot_sector_t *boot_sector = get_bs(dir->drive_number);
    uint32_t dir_entries_per_sector = (boot_sector->sectors_per_cluster * SECTOR_SIZE) / sizeof(uint32_t);
    for (uint32_t i = 0; i < dir_entries_per_sector; i++)
    {
        if (dir->entries[i].filename[0] == '\0')
            return &dir->entries[i];
    }
    return NULL;
}

// Allocates a new file
uint32_t create_file(fat32_directory_t *dir, char *name, char *ext, uint32_t file_size, uint8_t flags)
{
    fat32_boot_sector_t *boot_sector = get_bs(dir->drive_number);
    uint32_t cluster_count = INTCEIL(file_size, SECTOR_SIZE * boot_sector->sectors_per_cluster);
    if (cluster_count == 0)
        cluster_count = 1;

    uint32_t clusters[cluster_count];
    uint32_t cluster = 0;
    for (int i = 0; i < cluster_count; i++)
    {
        cluster = find_spare_cluster(dir->drive_number, cluster);
        if (cluster == -1)
            panic("Disk full!\n");
        clusters[i] = cluster;
    }

    fat32_entry_t *new_entry = find_spare_entry(dir);
    if (new_entry == NULL)
    {
        // TODO: allocate a new cluster for next chunk of the directory
        panic("Directory is full, and I don't know how to handle that!\n");
    }

    // Copy file name to directory entry
    uint32_t i;
    for (i = 0; i < 8; i++)
    {
        if (name[i] == '\0')
            break;
        new_entry->filename[i] = toupper(name[i]);
    }
    // Pad rest of filename with spaces
    for (; i < 8; i++)
        new_entry->filename[i] = ' ';

    // Copy extension to directory entry
    for (i = 0; i < 3; i++)
    {
        if (ext[i] == '\0')
            break;
        new_entry->extension[i] = toupper(ext[i]);
    }
    // Pad rest of extension with spaces
    for (; i < 3; i++)
        new_entry->extension[i] = ' ';

    new_entry->first_cluster_high = (uint16_t)(clusters[0] >> 16);
    new_entry->first_cluster_low = (uint16_t)clusters[0];
    new_entry->file_size = file_size;
    new_entry->flags = flags;

    // Fill FAT
    uint32_t *fat = get_fat(dir->drive_number);
    for (int i = 0; i < cluster_count - 1; i++)
        fat[clusters[i]] = clusters[i + 1];
    fat[clusters[cluster_count - 1]] = END_CLUSTER;

    // Save to disk
    // printf("Saving to disk\n");
    save_fat(dir->drive_number);
    save_directory(dir);
}

// Writes contents of buffer into file
int write_file(fat32_directory_t *dir, char *file_name, uint8_t *buffer, uint32_t size)
{
    fat32_boot_sector_t *boot_sector = get_bs(dir->drive_number);
    uint32_t *fat = get_fat(dir->drive_number);

    fat32_entry_t *entry = find_entry(dir, file_name);
    if (entry == NULL)
    {
        printf("Can't find existing entry for %s\n", file_name);
        return 0;
    }

    uint32_t cluster_size = SECTOR_SIZE * boot_sector->sectors_per_cluster;
    uint32_t cluster_count = INTCEIL(size, cluster_size);
    // printf("cluster_count: %u\n", cluster_count);
    uint32_t clusters[cluster_count];
    clusters[0] = ((uint32_t)entry->first_cluster_high << 16) | entry->first_cluster_low;
    if (cluster_count == 1)
    {
        if (fat[clusters[0]] != END_CLUSTER)
        {
            // There's too many clusters allocated, unallocate the excess
            uint32_t curr_cluster = fat[clusters[0]];
            fat[clusters[0]] = END_CLUSTER;
            while (fat[curr_cluster] != END_CLUSTER)
            {
                // printf("Marking cluster %u as unallocated\n", curr_cluster);
                uint32_t next = fat[curr_cluster];
                fat[curr_cluster] = 0;
                curr_cluster = next;
            }
        }
    }
    else
    {
        panic("Not implemented\n");
        // for (int i = 0; i < cluster_count - 1; i++)
        // {
        //     if (fat[curr_cluster] == END_CLUSTER)
        //     {
        //         panic("Not implemented yet!\n");
        //     }
        //     else
        //     {
        //         clusters[i] = curr_cluster;
        //     }
        // }
    }

    entry->file_size = size;

    // Save file contents
    // printf("Saving file contents to disk...\n");
    for (uint32_t i = 0; i < cluster_count; i++)
    {
        // printf("Saving %s to cluster %u\n", buffer + i * cluster_size, clusters[i]);
        save_cluster(dir->drive_number, clusters[i], buffer + i * cluster_size);
    }

    // Save to disk
    save_directory(dir);
    save_fat(dir->drive_number);

    return 1;
}

int rename(const char *old_filename, const char *new_filename)
{
    // TODO: handle absolute path including subdirectories
    // TODO: find disk number from file path?
    // TODO: handle file extensions
    fat32_entry_t *file = find_entry(&fat32_disks[0].root_dir, old_filename);
    if (file == NULL || is_directory(file))
        return 0;
    bool filename_end = false;
    for (int i = 0; i < FILENAME_LENGTH; i++)
    {
        if (filename_end || new_filename[i] == '\0')
        {
            file->filename[i] = ' ';
            filename_end = true;
        }
        else
            file->filename[i] = new_filename[i];
    }
    save_directory(&fat32_disks[0].root_dir);
    return 1;
}