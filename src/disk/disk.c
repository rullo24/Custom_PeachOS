#include "io/io.h"

int disk_read_sector(int lba, int total, void *buf) { 
    outb(0x1f6, (lba >> 24) | 0xe0); // Set the high byte of the LBA and select the master drive.
    outb(0x1f2, total); // Set the number of sectors to be read or written.
    outb(0x1f3, (unsigned char)(lba & 0xff)); // Send the low byte (bits 0-7) of the LBA.
    outb(0x1f4, (unsigned char)(lba >> 8)); // Send the second byte (bits 8-15) of the LBA.
    outb(0x1f5, (unsigned char)(lba >> 16)); // Send the third byte (bits 16-23) of the LBA.
    outb(0x1f7, 0x20); // Issue the 'READ SECTORS' command to initiate the data transfer.

    unsigned short *ptr = (unsigned short*)buf; // reading 2 bytes at a time
    for (int b=0; b<total; b++) { 
        // wait for buffer to be ready
        char c = insb(0x1f7); // reading from bus
        while (!(c & 0x08)) { // checking for a flag
            c = insb(0x1f7);
        }

        // copy from hard disk --> memory
        for (int i=0; i < 256; i++) {
            *ptr = insw(0x1f0); // reading 2 bytes into buffer that is parsed
            ptr++; // increment pointer
        }

    }

    return 0;
}