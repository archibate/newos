#ifndef _KERN_ASM_IDE_H
#define _KERN_ASM_IDE_H 1

/* Status */
#define IDE_BSY     0x80    // Busy
#define IDE_DRDY    0x40    // Ready
#define IDE_DF      0x20    // Write fault
#define IDE_ERR     0x01    // Error

/* Command */
#define IDE_CMD_READ  0x20
#define IDE_CMD_WRITE 0x30
#define IDE_CMD_RDMUL 0xc4
#define IDE_CMD_WRMUL 0xc5

/* Port */
#define IDE_DAT      0x1f0   // word data register (Read-Write)
#define IDE_ERROR    0x1f1   // byte error register (Read)
#define IDE_FEATURE  IDE_ERROR  // byte future register (Write)
#define IDE_SECTNR   0x1f2   // byte secount0 register (Read-Write)
// we use LBA28
#define IDE_LBA0     0x1f3   // byte LAB0 register (Read-Write)
#define IDE_LBA1     0x1f4   // byte LAB1 register (Read-Write)
#define IDE_LBA2     0x1f5   // byte LAB2 register (Read-Write)
#define IDE_CURR     0x1f6   // byte 101dhhhh d=drive hhhh=head (Read-Write)
#define IDE_STAT     0x1f7   // byte status register (Read)
#define IDE_CMD      IDE_STAT // byte status register (Write)
#define IDE_ALTSTAT  0x3f6   // same as IDE_STATUS but doesn't clear IRQ

#endif
