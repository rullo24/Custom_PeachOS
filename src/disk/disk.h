#ifndef DISK_H
#define DISK_H

#include "fs/file.h"

typedef unsigned int PEACHOS_DISK_TYPE;

// represents a real physical hard disk
#define PEACHOS_DISK_TYPE_REAL 0

struct disk {
    PEACHOS_DISK_TYPE type;
    int sector_size;
    int id; // the id of the disk
    struct filesystem *filesystem;
    void *fs_private; // the private data of the filesystem
};

void disk_search_and_init();
struct disk *disk_get(int index);
int disk_read_block(struct disk *idisk, unsigned int lba, int total, void *buf);

#endif