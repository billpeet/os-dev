#include "ata.h"
#include <stdio.h>
#include "kernel.h"
#include "x86.h"
#include "task.h"

int lba_delay()
{
    for (u8 i = 0; i < 5; i++)
        inb(0x1f7);
}

int lba_wait()
{
    int r;
    while (((r = inb(0x1f7)) & (ATA_SR_BSY | ATA_SR_DRDY)) != ATA_SR_DRDY)
        ;
    if (r & (ATA_SR_DF | ATA_SR_ERR) != 0)
        return -1;
    return 0;
}

u16 get_port(u8 drive_num)
{
    switch (drive_num)
    {
    case 0:
    case 1:
        return 0x1F0;
        break;
    case 2:
    case 3:
        return 0x170;
        break;
    case 4:
    case 5:
        return 0x1E8;
        break;
    case 6:
    case 7:
        return 0x178;
        break;
    default:
        panic("Invalid drive number %u\n", drive_num);
    }
}

// 24-bit LBA
void read_sectors_ata(u8 drive_num, u32 sector_number, u8 sector_count, lba_sector_t *dest)
{
    u16 base = get_port(drive_num);
    u8 drive = 0x40;
    if (drive_num % 2)
        // Slave
        drive |= 0x10;

    outb(base + ATA_REG_FEA, 0x00);
    outb(base + ATA_SEC_CNT, sector_count);
    outb(base + ATA_REG_LOW, (u8)(sector_number));
    outb(base + ATA_REG_MID, (u8)(sector_number >> 8));
    outb(base + ATA_REG_HI, (u8)(sector_number >> 16));
    outb(base + ATA_REG_DRV, ((u8)(sector_number >> 24)) | drive);
    outb(base + ATA_REG_CMD, ATA_CMD_READ_PIO);

    // TODO: replace with IRQ
    lba_delay();
    lba_wait();

    insl(base, dest, sector_count * SECTOR_SIZE);
}

void write_sectors_ata(u8 drive_num, u32 sector_number, u8 sector_count, lba_sector_t *dest)
{
    u16 base = get_port(drive_num);
    u8 drive = 0x40;
    if (drive_num % 2)
        drive |= 0x10;

    outb(base + ATA_REG_FEA, 0x00);
    outb(base + ATA_SEC_CNT, sector_count);
    outb(base + ATA_REG_LOW, (u8)(sector_number));
    outb(base + ATA_REG_MID, (u8)(sector_number >> 8));
    outb(base + ATA_REG_HI, (u8)(sector_number >> 16));
    outb(base + ATA_REG_DRV, ((sector_number >> 24) & 0x0F) | drive);
    outb(base + ATA_REG_CMD, ATA_CMD_WRITE_PIO);

    // TODO: replace with IRQ
    lba_delay();
    lba_wait();

    u16 *buffer = (u16 *)dest;
    for (u32 i = 0; i < sector_count * SECTOR_SIZE / 2; i++)
        outw(base, buffer[i]);
}