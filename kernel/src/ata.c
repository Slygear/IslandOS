#include "ata.h"
#include "vga.h"

static ata_drive_t drives[4];
static int         drive_count = 0;

static void outb(uint16_t port, uint8_t val) {
    __asm__ volatile("outb %b0, %w1" :: "a"(val), "d"(port));
}

static void outw(uint16_t port, uint16_t val) {
    __asm__ volatile("outw %w0, %w1" :: "a"(val), "d"(port));
}

static uint8_t inb(uint16_t port) {
    uint8_t val;
    __asm__ volatile("inb %w1, %b0" : "=a"(val) : "d"(port));
    return val;
}

static uint16_t inw(uint16_t port) {
    uint16_t val;
    __asm__ volatile("inw %w1, %w0" : "=a"(val) : "d"(port));
    return val;
}

static void ata_delay(uint16_t ctrl) {
    (void)ctrl;
    for (volatile int i = 0; i < 10000; i++);
}

static int ata_wait(uint16_t base) {
    uint8_t status;
    int timeout = 100000;

    while (timeout--) {
        status = inb(base + ATA_REG_STATUS);
        if (!(status & ATA_STATUS_BSY)) break;
    }

    if (timeout <= 0) return -1;
    if (status & ATA_STATUS_ERR) return -2;
    if (status & ATA_STATUS_DF)  return -3;

    return 0;
}

static int ata_identify(uint16_t base, uint16_t ctrl, uint8_t drive_sel, ata_drive_t* out) {
    // Check if bus exists
    outb(base + ATA_REG_DRIVE, drive_sel);
    for (volatile int i = 0; i < 10000; i++);
    uint8_t status = inb(base + ATA_REG_STATUS);
    if (status == 0xFF || status == 0x7F) return 0;

    // Send IDENTIFY
    outb(base + ATA_REG_SECCOUNT, 0);
    outb(base + ATA_REG_LBA0,     0);
    outb(base + ATA_REG_LBA1,     0);
    outb(base + ATA_REG_LBA2,     0);
    outb(base + ATA_REG_COMMAND,  ATA_CMD_IDENTIFY);

    status = inb(base + ATA_REG_STATUS);
    if (status == 0 || status == 0xFF) return 0;

    int timeout = 100000;
    while (timeout--) {
        status = inb(base + ATA_REG_STATUS);
        if (!(status & ATA_STATUS_BSY)) break;
    }
    if (timeout <= 0) return 0;

    uint8_t lba1 = inb(base + ATA_REG_LBA1);
    uint8_t lba2 = inb(base + ATA_REG_LBA2);
    if (lba1 != 0 || lba2 != 0) return 0;

    timeout = 100000;
    while (timeout--) {
        status = inb(base + ATA_REG_STATUS);
        if (status & ATA_STATUS_DRQ) break;
        if (status & ATA_STATUS_ERR) return 0;
    }
    if (timeout <= 0) return 0;

    uint16_t identify[256];
    for (int i = 0; i < 256; i++)
        identify[i] = inw(base + ATA_REG_DATA);

    out->base   = base;
    out->ctrl   = ctrl;
    out->drive  = drive_sel;
    out->exists = 1;

    return 1;
}

int ata_init(void) {
    drive_count = 0;

    // Only try primary master for now
    if (ata_identify(ATA_PRIMARY_BASE, ATA_PRIMARY_CTRL, 0xA0, &drives[drive_count]))
        drive_count++;

    // Primary slave
    if (drive_count < 4)
        if (ata_identify(ATA_PRIMARY_BASE, ATA_PRIMARY_CTRL, 0xB0, &drives[drive_count]))
            drive_count++;

    return drive_count;
}

int ata_read(ata_drive_t* drive, uint32_t lba, uint8_t sectors, uint8_t* buf) {
    if (!drive || !drive->exists) return -1;

    // Select drive with LBA mode
    outb(drive->base + ATA_REG_DRIVE,
         0xE0 | (drive->drive == 0xB0 ? 0x10 : 0x00) | ((lba >> 24) & 0x0F));

    ata_delay(drive->ctrl);

    outb(drive->base + ATA_REG_SECCOUNT, sectors);
    outb(drive->base + ATA_REG_LBA0,     (lba)       & 0xFF);
    outb(drive->base + ATA_REG_LBA1,     (lba >> 8)  & 0xFF);
    outb(drive->base + ATA_REG_LBA2,     (lba >> 16) & 0xFF);
    outb(drive->base + ATA_REG_COMMAND,  ATA_CMD_READ_PIO);

    for (int s = 0; s < sectors; s++) {
        if (ata_wait(drive->base) < 0) return -1;

        // Read 256 words = 512 bytes
        uint16_t* ptr = (uint16_t*)(buf + s * 512);
        for (int i = 0; i < 256; i++)
            ptr[i] = inw(drive->base + ATA_REG_DATA);
    }

    return 0;
}

int ata_write(ata_drive_t* drive, uint32_t lba, uint8_t sectors, uint8_t* buf) {
    if (!drive || !drive->exists) return -1;

    outb(drive->base + ATA_REG_DRIVE,
         0xE0 | (drive->drive == 0xB0 ? 0x10 : 0x00) | ((lba >> 24) & 0x0F));

    ata_delay(drive->ctrl);

    outb(drive->base + ATA_REG_SECCOUNT, sectors);
    outb(drive->base + ATA_REG_LBA0,     (lba)       & 0xFF);
    outb(drive->base + ATA_REG_LBA1,     (lba >> 8)  & 0xFF);
    outb(drive->base + ATA_REG_LBA2,     (lba >> 16) & 0xFF);
    outb(drive->base + ATA_REG_COMMAND,  ATA_CMD_WRITE_PIO);

    for (int s = 0; s < sectors; s++) {
        if (ata_wait(drive->base) < 0) return -1;

        uint16_t* ptr = (uint16_t*)(buf + s * 512);
        for (int i = 0; i < 256; i++)
            outw(drive->base + ATA_REG_DATA, ptr[i]);
    }

    // Flush cache
    outb(drive->base + ATA_REG_COMMAND, 0xE7);
    ata_wait(drive->base);

    return 0;
}

ata_drive_t* ata_get_drive(int index) {
    if (index < 0 || index >= drive_count) return 0;
    return &drives[index];
}