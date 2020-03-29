#include "lba.h"
#include "stdio.h"
#include "kernel.h"
#include "x86.h"
#include "task.h"

#define CMD_MASTER 0xe0
#define CMD_SLAVE 0xf0

#define CMD_READ 0x20
#define CMD_READMUL 0xc4
#define CMD_WRITE 0x30
#define CMD_WRITEMUL 0xc5

#define BUSY_FLAG 0x80
#define RDY_FLAG 0x40
#define DF_FLAG 0x20
#define ERR_FLAG 0x01

void dump_sector(lba_sector_t *sector)
{
    for (u32 i = 0; i < SECTOR_SIZE; i++)
    {
        printf("%x", sector->bytes[i]);
    }
}

int lba_delay()
{
    for (u8 i = 0; i < 5; i++)
        inb(0x1f7);
}

int lba_wait()
{
    int r;
    while (((r = inb(0x1f7)) & (BUSY_FLAG | RDY_FLAG)) != RDY_FLAG)
        yield();
    if (r & (DF_FLAG | ERR_FLAG) != 0)
        return -1;
    return 0;
}

// 24-bit LBA
void read_sectors_lba(u8 drive_num, u32 sector_number, u8 sector_count, lba_sector_t *dest)
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
        printf("Invalid drive number %u\n", drive_num);
        panic(3);
    }

    u8 drive = 0x40;
    if (drive_num % 2)
    {
        drive |= 0x10;
    }

    outb(base + 6, CMD_MASTER | (sector_number >> 24));
    lba_wait();

    outb(base + 1, 0x00);
    outb(base + 2, sector_count);
    outb(base + 3, (u8)(sector_number));
    outb(base + 4, (u8)(sector_number >> 8));
    outb(base + 5, (u8)(sector_number >> 16));
    outb(base + 6, ((sector_number >> 24) & 0x0F) | drive);
    outb(base + 7, CMD_READ);

    lba_wait();

    u8 *buffer = (u8 *)dest;
    insl(base, buffer, sector_count * (SECTOR_SIZE / 4));
    lba_delay();
    lba_wait();

    if (sector_count > 1)
        return read_sectors_lba(drive_num, sector_number + 1, sector_count - 1, dest + 1);
}

void write_sectors_lba(u8 drive_num, u32 sector_number, u8 sector_count, lba_sector_t *dest)
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
        printf("Invalid drive number %u\n", drive_num);
        panic(3);
    }

    u8 drive = 0x40;
    if (drive_num % 2)
    {
        drive |= 0x10;
    }

    outb(base + 6, CMD_MASTER);
    outb(base + 1, 0x00);
    outb(base + 2, sector_count);
    outb(base + 3, (u8)(sector_number));
    outb(base + 4, (u8)(sector_number >> 8));
    outb(base + 5, (u8)(sector_number >> 16));
    outb(base + 6, ((sector_number >> 24) & 0x0F) | drive);
    outb(base + 7, CMD_WRITE);

    lba_wait();

    u8 *buffer = (u8 *)dest;

    for (u32 i = 0; i < SECTOR_SIZE / 4; i++)
        outb(base, buffer[i]);

    lba_delay();
    lba_wait();
    if (sector_count > 1)
        write_sectors_lba(drive_num, sector_number + 1, sector_count - 1, dest + 1);
}