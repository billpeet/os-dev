#include "fat.h"
#include "vga.h"
#include "frame_allocator.h"
#include "alloc.h"
#include "string.h"

fat32_boot_sector_t last_header;
u32 last_drive_number;

void get_header(u8 drive_number, fat32_boot_sector_t *header)
{
    read_sectors_lba(drive_number, 0, 1, (lba_sector_t *)header);
}

int load_header(u32 drive_number)
{
    if (last_drive_number != drive_number || last_header.jump_inst[0] == 0)
    {
        // Load new header into cache
        get_header(drive_number, &last_header);
        if (last_header.sig != 0xaa55)
        {
            writeString("Invalid FAT disk!\n");
            return 0;
        }
    }
    return 1;
}

u32 *read_fat()
{
    u32 *fat_ptr = malloc(last_header.sectors_per_fat * SECTOR_SIZE);
    read_sectors_lba(last_drive_number, last_header.reserved_sectors, last_header.sectors_per_fat, (lba_sector_t *)fat_ptr);
    return fat_ptr;
}

void dump_header(fat32_boot_sector_t *header)
{
    writeString("Jump inst: ");
    for (u32 i = 0; i < 3; i++)
        writeHexInt(header->jump_inst[i]);
    writeString("\nOEM Id: ");
    for (u32 i = 0; i < 8; i++)
        writeChar(header->oem_id[i]);
    writeStrInt("\nBytes per sector: ", header->bytes_per_sector);
    writeStrInt("Sectors per cluster: ", header->sectors_per_cluster);
    writeStrInt("Reserved sectors: ", header->reserved_sectors);
    writeStrInt("FAT copies: ", header->fat_copies);
    writeStrHexInt("Media descriptor: ", header->media_descriptor);
    writeStrInt("Sectors per track: ", header->sectors_per_track);
    writeStrInt("Heads: ", header->heads);
    writeStrInt("Hidden sectors: ", header->hidden_sectors);
    writeStrInt("Sectors in partition: ", header->sectors_in_partition);
    writeStrInt("Sectors per FAT: ", header->sectors_per_fat);
    writeStrHexInt("Flags: ", header->flags);
    writeStrInt("FAT version: ", header->fat_version);
    writeStrHexInt("Root cluster: ", header->root_cluster);
    writeStrHexInt("FSI sector: ", header->fsi_sector);
    writeStrHexInt("Backup boot sector: ", header->backup_boot_sector);
    writeStrHexInt("Logical drive number: ", header->logical_drive_number);
    writeStrInt("Current head: ", header->current_head);
    writeStrHexInt("Signature: ", header->signature);
    writeStrInt("ID: ", header->id);
    writeString("Volume label: ");
    for (u32 i = 0; i < 11; i++)
        writeChar(header->volume_label[i]);
    writeString("\nSystem ID: ");
    for (u32 i = 0; i < 8; i++)
        writeChar(header->system_id[i]);
    writeStrHexInt("\nFinal signature: ", header->sig);
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
    writeChar(entry->flags & 0b10000 ? 'd' : '-');
    writeChar(entry->flags & 0b100000 ? 'a' : '-');
    writeChar(entry->flags & 0b1000 ? 'v' : '-');
    writeChar(entry->flags & 0b100 ? 's' : '-');
    writeChar(entry->flags & 0b10 ? 'h' : '-');
    writeChar(entry->flags & 0b1 ? 'r' : '-');
    writeString("     ");

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
    writeNewLine();
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
    writeNewLine();
    writeString("Mode       Name\n");
    writeString("----       ----\n");

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

    writeNewLine();
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

void *read_file(u8 drive_number, fat32_entry_t *current_dir, char *file_name, u32 *size)
{
    u32 *fat; // = read_fat();
    if (!load_header(drive_number))
        return NULLPTR;

    fat32_entry_t *entry = find_entry(current_dir, file_name, drive_number);

    u32 cluster_number = get_cluster_number(entry);
    writeStrInt("Cluster ", cluster_number);

    // writeStrHexInt("FAT value at this cluster: ", fat[cluster_number]);
    // free(fat);

    u32 cluster_begin = last_header.reserved_sectors + (last_header.fat_copies * last_header.sectors_per_fat);

    u32 sector_number = cluster_begin + (cluster_number - 2) * last_header.sectors_per_cluster;
    writeStrInt("Sector ", sector_number);

    *size = entry->file_size;
    writeStrInt("File size: ", *size);
    u64 alloc_size = *size - (*size % sizeof(lba_sector_t)) + sizeof(lba_sector_t);
    writeStrInt("Alloc size ", alloc_size);

    lba_sector_t *cluster = malloc(alloc_size);
    read_sectors_lba(drive_number, sector_number, alloc_size / SECTOR_SIZE, cluster);
    return cluster;
}