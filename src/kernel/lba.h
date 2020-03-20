#ifndef LBA_H
#define LBA_H

#include "types.h"

#define SECTOR_SIZE 512

typedef struct lba_sector
{
    u8 bytes[SECTOR_SIZE];
} lba_sector_t;

void read_sectors_lba(u8 drive_num, u32 sector_number, u8 sector_count, lba_sector_t *dest);
void write_sectors_lba(u8 drive_num, u32 sector_number, u8 sector_count, lba_sector_t *dest);
void dump_sector(lba_sector_t *sector);

#endif