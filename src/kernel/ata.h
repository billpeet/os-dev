#ifndef ATA_H
#define ATA_H

#include "types.h"

#define SECTOR_SIZE 512

#define ATA_MASTER 0xA0
#define ATA_SLAVE 0xB0

// Registers
#define ATA_REG_FEA 0x01 // FEATURES register
#define ATA_SEC_CNT 0x02 // Sector count register
#define ATA_REG_LOW 0x03 // LBA low register OR sector number
#define ATA_REG_MID 0x04 // LBA mid number OR Cyl low
#define ATA_REG_HI 0x05  // LBA high number OR Cyl high
#define ATA_REG_DRV 0x06 // Drive number and 4 bits of LBA
#define ATA_REG_CMD 0x07 // Command register

// Status register bitmasks
#define ATA_SR_BSY 0x80  // Busy
#define ATA_SR_DRDY 0x40 // Drive ready
#define ATA_SR_DF 0x20   // Drive write fault
#define ATA_SR_DSC 0x10  // Drive seek complete
#define ATA_SR_DSQ 0x08  // Data request ready
#define ATA_SR_CORR 0x04 // Corrected data
#define ATA_SR_IDX 0x02  // Index
#define ATA_SR_ERR 0x01  // Error

// Error values
#define ATA_ER_BBK 0x80   // Bad block
#define ATA_ER_UNC 0x40   // Uncorrectable data
#define ATA_ER_MC 0x20    // Media changed
#define ATA_ER_IDNF 0x10  // ID mark not found
#define ATA_ER_MCR 0x08   // Media change request
#define ATA_ER_ABRT 0x04  // Command aborted
#define ATA_ER_TK0NF 0x02 // Track 0 not found
#define ATA_ER_AMNF 0x01  // No address mark

// Commands
#define ATA_CMD_READ_PIO 0x20
#define ATA_CMD_READ_PIO_EXT 0x24
#define ATA_CMD_READ_DMA 0xC8
#define ATA_CMD_READ_DMA_EXT 0x25
#define ATA_CMD_WRITE_PIO 0x30
#define ATA_CMD_WRITE_PIO_EXT 0x34
#define ATA_CMD_WRITE_DMA 0xCA
#define ATA_CMD_WRITE_DMA_EXT 0x35
#define ATA_CMD_CACHE_FLUSH 0xE7
#define ATA_CMD_CACHE_FLUSH_EXT 0xEA
#define ATA_CMD_PACKET 0xA0
#define ATA_CMD_IDENTIFY_PACKET 0xA1
#define ATA_CMD_IDENTIFY 0xEC

typedef struct lba_sector
{
    u8 bytes[SECTOR_SIZE];
} lba_sector_t;

void ata_init(void);
void read_sectors_ata(u8 drive_num, u32 sector_number, u8 sector_count, lba_sector_t *dest);
void write_sectors_ata(u8 drive_num, u32 sector_number, u8 sector_count, lba_sector_t *dest);

#endif