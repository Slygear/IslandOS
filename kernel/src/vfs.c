#include "vfs.h"
#include "pmm.h"
#include "ramfs.h"

static vfs_node_t nodes[VFS_MAX_NODES];
static int        node_count = 0;
static vfs_node_t* root_node = 0;

static vfs_node_t* alloc_node(void) {
    if (node_count >= VFS_MAX_NODES) return 0;
    vfs_node_t* node = &nodes[node_count++];
    // Zero out
    for (int i = 0; i < (int)sizeof(vfs_node_t); i++)
        ((uint8_t*)node)[i] = 0;
    return node;
}

static void strcpy_safe(char* dst, const char* src, int max) {
    int i = 0;
    while (src[i] && i < max - 1) {
        dst[i] = src[i];
        i++;
    }
    dst[i] = 0;
}

static int strcmp_simple(const char* a, const char* b) {
    while (*a && *b && *a == *b) { a++; b++; }
    return *a - *b;
}

void vfs_init(void) {
    node_count = 0;
    root_node  = alloc_node();
    strcpy_safe(root_node->name, "/", VFS_MAX_NAME);
    root_node->type        = VFS_DIR;
    root_node->inode       = 0;
    root_node->parent      = root_node;
    root_node->child_count = 0;
}

vfs_node_t* vfs_root(void) {
    return root_node;
}

static vfs_node_t* find_child(vfs_node_t* dir, const char* name) {
    for (int i = 0; i < dir->child_count; i++)
        if (strcmp_simple(dir->children[i]->name, name) == 0)
            return dir->children[i];
    return 0;
}

vfs_node_t* vfs_find(const char* path) {
    if (!path || path[0] != '/') return 0;
    if (path[1] == 0) return root_node;

    vfs_node_t* current = root_node;
    char part[VFS_MAX_NAME];
    int  pi = 0;
    int  i  = 1;

    while (path[i]) {
        if (path[i] == '/') {
            if (pi > 0) {
                part[pi] = 0;
                current  = find_child(current, part);
                if (!current) return 0;
                pi = 0;
            }
        } else {
            part[pi++] = path[i];
        }
        i++;
    }

    if (pi > 0) {
        part[pi] = 0;
        current  = find_child(current, part);
    }

    return current;
}

static vfs_node_t* vfs_create(const char* path, vfs_node_type_t type) {
    // Find parent
    char parent_path[VFS_MAX_PATH];
    char name[VFS_MAX_NAME];

    // Split path into parent + name
    int last_slash = 0;
    int len = 0;
    while (path[len]) {
        if (path[len] == '/') last_slash = len;
        len++;
    }

    // Copy parent path
    for (int i = 0; i < last_slash; i++)
        parent_path[i] = path[i];
    parent_path[last_slash == 0 ? 1 : last_slash] = 0;
    if (last_slash == 0) { parent_path[0] = '/'; parent_path[1] = 0; }

    // Copy name
    int ni = 0;
    for (int i = last_slash + 1; i < len; i++)
        name[ni++] = path[i];
    name[ni] = 0;

    vfs_node_t* parent = vfs_find(parent_path);
    if (!parent) return 0;

    vfs_node_t* node = alloc_node();
    if (!node) return 0;
    
    if (type == VFS_FILE)
        node->ops = ramfs_get_ops();

    strcpy_safe(node->name, name, VFS_MAX_NAME);
    node->type        = type;
    node->inode       = node_count;
    node->parent      = parent;
    node->child_count = 0;

    if (parent->child_count < 16)
        parent->children[parent->child_count++] = node;

    return node;
}

vfs_node_t* vfs_mkdir(const char* path) {
    return vfs_create(path, VFS_DIR);
}

vfs_node_t* vfs_mkfile(const char* path) {
    return vfs_create(path, VFS_FILE);
}

size_t vfs_read(vfs_node_t* node, uint8_t* buf, size_t size, size_t offset) {
    if (!node || !node->ops || !node->ops->read) return 0;
    return node->ops->read(node, buf, size, offset);
}

size_t vfs_write(vfs_node_t* node, uint8_t* buf, size_t size, size_t offset) {
    if (!node || !node->ops || !node->ops->write) return 0;
    return node->ops->write(node, buf, size, offset);
}