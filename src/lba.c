#include "lba.h"
#include "vga.h"
#include "kernel.h"

#define CMD_READ 0x20
#define CMD_WRITE 0x30
#define BUSY_FLAG 0x80

// 24-bit LBA
void read_sectors_lba(u8 drive_num, u32 sector_number, u8 sector_count, void *dest)
{
    u16 base;

    switch (drive_num)
    {
    case 0:
    case 1:
        base = 0x1F0;
        break;
    case 2:
    case 3:
        base = 0x170;
        break;
    case 4:
    case 5:
        base = 0x1E8;
        break;
    case 6:
    case 7:
        base = 0x178;
        break;
    default:
        writeString("Invalid drive number ");
        writeInt(drive_num);
        writeNewLine();
        panic(3);
    }

    u8 drive = 0x40;
    if (drive_num % 2)
    {
        drive |= 0x10;
    }

    while (read_port(base + 7) & BUSY_FLAG)
        ;

    write_port(base + 1, 0x00);
    write_port(base + 2, sector_count);
    write_port(base + 3, (u8)(sector_number & 0xff));
    write_port(base + 4, (u8)(sector_number & 0xff00) >> 8);
    write_port(base + 5, (u8)(sector_number & 0xff0000) >> 16);
    write_port(base + 6, ((sector_number >> 24) & 0x0F) | drive);
    write_port(base + 7, CMD_READ);

    while (read_port(base + 7) & BUSY_FLAG)
        ;

    u16 *buffer = (u16 *)dest;
    for (u16 i = 0; i < (sector_count * (512 / 2)); i++)
    {
        u16 res = read_port(base);
        buffer[i] = res;
    }
}

void write_sectors_lba(u8 drive_num, u32 sector_number, u8 sector_count, void *dest)
{
    u16 base;

    switch (drive_num)
    {
    case 0:
    case 1:
        base = 0x1F0;
        break;
    case 2:
    case 3:
        base = 0x170;
        break;
    case 4:
    case 5:
        base = 0x1E8;
        break;
    case 6:
    case 7:
        base = 0x178;
        break;
    default:
        writeString("Invalid drive number ");
        writeInt(drive_num);
        writeNewLine();
        panic(3);
    }

    u8 drive = 0x40;
    if (drive_num % 2)
    {
        drive |= 0x10;
    }

    while (read_port(base + 7) & BUSY_FLAG)
        ;

    write_port(base + 1, 0x00);
    write_port(base + 2, sector_count);
    write_port(base + 3, (u8)(sector_number & 0xff));
    write_port(base + 4, (u8)(sector_number & 0xff00) >> 8);
    write_port(base + 5, (u8)(sector_number & 0xff0000) >> 16);
    write_port(base + 6, ((sector_number >> 24) & 0x0F) | drive);
    write_port(base + 7, CMD_WRITE);

    while (read_port(base + 7) & BUSY_FLAG)
        ;

    u16 *buffer = (u16 *)dest;
    for (u16 i = 0; i < (sector_count * (512 / 2)); i++)
    {
        write_port_16(base, buffer[i]);
    }
}