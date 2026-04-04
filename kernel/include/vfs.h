#ifndef VFS_H
#define VFS_H

#include <stdint.h>
#include <stddef.h>

#define VFS_MAX_NAME  64
#define VFS_MAX_PATH  256
#define VFS_MAX_NODES 32

typedef enum {
    VFS_FILE,
    VFS_DIR,
    VFS_MOUNTPOINT
} vfs_node_type_t;

struct vfs_node;

typedef struct {
    size_t (*read) (struct vfs_node*, uint8_t*, size_t, size_t);
    size_t (*write)(struct vfs_node*, uint8_t*, size_t, size_t);
    int    (*open) (struct vfs_node*);
    int    (*close)(struct vfs_node*);
} vfs_ops_t;

typedef struct vfs_node {
    char             name[VFS_MAX_NAME];
    vfs_node_type_t  type;
    uint64_t         size;
    uint64_t         inode;
    vfs_ops_t*       ops;
    void*            data;
    struct vfs_node* parent;
    struct vfs_node* children[16];
    int              child_count;
} vfs_node_t;

void        vfs_init(void);
vfs_node_t* vfs_find(const char* path);
vfs_node_t* vfs_mkdir(const char* path);
vfs_node_t* vfs_mkfile(const char* path);
size_t      vfs_read(vfs_node_t* node, uint8_t* buf, size_t size, size_t offset);
size_t      vfs_write(vfs_node_t* node, uint8_t* buf, size_t size, size_t offset);
vfs_node_t* vfs_root(void);

#endif