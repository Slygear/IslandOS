#include "ramfs.h"
#include "pmm.h"

#define RAMFS_MAX_FILE_SIZE 4096

typedef struct {
    uint8_t data[RAMFS_MAX_FILE_SIZE];
    size_t  size;
} ramfs_file_t;

static vfs_ops_t ramfs_ops;

static size_t ramfs_read(vfs_node_t* node, uint8_t* buf, size_t size, size_t offset) {
    ramfs_file_t* file = (ramfs_file_t*)node->data;
    if (!file) return 0;
    if (offset >= file->size) return 0;
    if (offset + size > file->size) size = file->size - offset;

    for (size_t i = 0; i < size; i++)
        buf[i] = file->data[offset + i];

    return size;
}

static size_t ramfs_write(vfs_node_t* node, uint8_t* buf, size_t size, size_t offset) {
    ramfs_file_t* file = (ramfs_file_t*)node->data;
    if (!file) {
        file = (ramfs_file_t*)pmm_alloc();
        if (!file) return 0;
        for (int i = 0; i < (int)sizeof(ramfs_file_t); i++)
            ((uint8_t*)file)[i] = 0;
        node->data = file;
    }

    if (offset + size > RAMFS_MAX_FILE_SIZE)
        size = RAMFS_MAX_FILE_SIZE - offset;

    for (size_t i = 0; i < size; i++)
        file->data[offset + i] = buf[i];

    if (offset + size > file->size)
        file->size = offset + size;

    node->size = file->size;
    return size;
}

static int ramfs_open(vfs_node_t* node) {
    (void)node;
    return 0;
}

static int ramfs_close(vfs_node_t* node) {
    (void)node;
    return 0;
}

void ramfs_init(vfs_node_t* mount_point) {
    ramfs_ops.read  = ramfs_read;
    ramfs_ops.write = ramfs_write;
    ramfs_ops.open  = ramfs_open;
    ramfs_ops.close = ramfs_close;
    (void)mount_point;
}

vfs_node_t* ramfs_create_file(vfs_node_t* parent, const char* name) {
    (void)parent;
    (void)name;
    return 0;
}