#include "file.h"
#include "config.h"
#include "memory/memory.h"
#include "memory/heap/kheap.h"
#include "string/string.h"
#include "disk/disk.h"
#include "fat/fat16.h"
#include "status.h"
#include "kernel.h"

struct filesystem *filesystems[PEACHOS_MAX_FILESYSTEMS];
struct file_descriptor *file_descriptors[PEACHOS_MAX_FILE_DESCRIPTORS];

static struct filesystem **fs_get_free_filesystem() {
    int i = 0;
    for (i=0; i < PEACHOS_MAX_FILESYSTEMS; i++) {
        if (filesystems[i] == 0) {
            return &filesystems[i]; // return addr of empty slot in array
        }
    }

    return 0;
}

// allow drivers to insert own filesystems
void fs_insert_filesystem(struct filesystem *filesystem) {
    struct filesystem **fs;
    fs = fs_get_free_filesystem();
    if (!fs) { // failed to insert filesystem
        print("Problem inserting filesystem");
        while(1) {}
    }

    *fs = filesystem; // set val in array returned to addr of filesystem provided
}

static void fs_static_load() {
    fs_insert_filesystem(fat16_init());
}

void fs_load() {
    memset(filesystems, 0, sizeof(filesystems));
    fs_static_load();
}

void fs_init() {
    memset(file_descriptors, 0, sizeof(file_descriptors));
    fs_load();
}

static int file_new_descriptor(struct file_descriptor **desc_out) {
    int res = -ENOMEM;
    for (int i=0; i < PEACHOS_MAX_FILE_DESCRIPTORS; i++) {
        if (file_descriptors[i] == 0) {
            struct file_descriptor *desc = kzalloc(sizeof(struct file_descriptor));
            // descriptors start at 1
            desc->index = i+1;
            file_descriptors[i] = desc;
            *desc_out = desc;
            res = 0;
            break;
        }
    }

    return res;
}

static struct file_descriptor *file_get_descriptor(int fd) {
    if (fd <= 0 || fd >= PEACHOS_MAX_FILE_DESCRIPTORS) { // invalid info passed
        return 0; 
    }

    // descriptors start at 1
    int index = fd - 1; // finding index in array
    return file_descriptors[index];
}

struct filesystem *fs_resolve(struct disk *disk) {
    struct filesystem *fs = 0;
    for (int i=0; i < PEACHOS_MAX_FILESYSTEMS; i++) {
        if (filesystems[i] != 0 && filesystems[i]->resolve(disk) == 0) {
            fs = filesystems[i];
            break;
        }
    }
    return fs;
}

FILE_MODE file_get_mode_by_string(const char *str) {
    FILE_MODE mode = FILE_MODE_INVALID;
    
    // choosing correct binary mode based on string provided
    if (strncmp(str, "r", 1) == 0) {
        mode = FILE_MODE_READ;
    } else if (strncmp(str, "w", 1) == 0) {
        mode = FILE_MODE_WRITE;
    } else if (strncmp(str, "a", 1) == 0) {
        mode = FILE_MODE_APPEND;
    }

    return mode;
}

int fopen(const char *filename, const char *mode_str) {
    int res = 0;   
    struct path_root *root_path = pathparser_parse(filename, NULL);
    if (!root_path) {
        res = -EINVARG;
        goto out;
    }

    // cannot have just a root path (i.e. 0:/)
    if (!root_path->first) {
        res = -EINVARG;
        goto out;
    }

    // ensuring that the disk that we're trying to get from does actually exist
    struct disk *disk = disk_get(root_path->drive_no);
    if (!disk) {
        res = -EIO;
        goto out;
    }

    // checking that the current disk actually has a valid filesystem
    if (!disk->filesystem) {
        res = -EIO;
        goto out;
    }

    FILE_MODE mode = file_get_mode_by_string(mode_str);
    if (mode == FILE_MODE_INVALID) {
        res = -EINVARG;
        goto out;
    }

    void *descriptor_private_data = disk->filesystem->open(disk, root_path->first, mode); // calling filesystem open function
    if (ISERR(descriptor_private_data)) {
        res = ERROR_I(descriptor_private_data);
        goto out;
    }

    struct file_descriptor *desc = 0;
    res = file_new_descriptor(&desc);
    if (res < 0) {
        goto out;
    }
    desc->filesystem = disk->filesystem;
    desc->private = descriptor_private_data;
    desc->disk = disk;
    res = desc->index;

out:
    // fopen shouldn't return negative values
    if (res < 0) {
        res = 0;
    }
    return res;
}

int fread(void *ptr, uint32_t size, uint32_t nmemb, int fd) {
    int res = 0;
    if (size == 0x0 || nmemb == 0x0 || fd < 1) {
        res = -EINVARG;
        goto out;
    }   

    struct file_descriptor *desc = file_get_descriptor(fd);
    if (!desc) {
        res = -EINVARG;
        goto out;
    }
    // call lower filesystem --> parse private desc data
    res = desc->filesystem->read(desc->disk, desc->private, size, nmemb, (char*)ptr);

out:
    return res;
}