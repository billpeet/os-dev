#include "fat.h"
#include "vga.h"
#include "frame_allocator.h"

void get_header(u8 drive_number, fat32_boot_sector_t *header)
{
    read_sectors_lba(drive_number, 0, 1, header);
}

void display_header(u8 drive_number)
{
    fat32_boot_sector_t *header = allocate_frame();
    get_header(drive_number, header);

    u32 first_data_sector = header->reserved_sectors + (header->fat_copies * header->sectors_per_fat);
    writeString("First data sector: ");
    writeInt(first_data_sector);
    writeNewLine();

    u32 root_cluster = header->root_cluster;
    writeString("Root cluster: ");
    writeInt(root_cluster);
    writeNewLine();

    u32 first_sector_of_cluster = ((0 - 2))

    // u8 *ptr = (u8 *)header;
    // writeString("First 20 bytes: ");
    // for (u32 i = 0; i < 20; i++)
    // {
    //     writeHexInt(ptr[i]);
    // }
    // writeNewLine();

    // writeString("Code: ");
    // writeHexInt(header->code[0]);
    // writeHexInt(header->code[1]);
    // writeHexInt(header->code[2]);
    // writeString("\nOEM Id: ");
    // writeHexInt(header->oem_id);
    // writeString("\nBytes per sector: ");
    // writeInt(header->bytes_per_sector);
    // writeString("\nLogical drive number: ");
    // writeInt(header->logical_drive_number);
    // writeString("\nHidden sectors: ");
    // writeInt(header->hidden_sectors);
    // writeString("\nSystem ID: ");
    // for (int i = 0; i < 8; i++)
    //     writeChar(header->system_id[i]);
    // writeNewLine();
}