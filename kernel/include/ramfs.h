#ifndef RAMFS_H
#define RAMFS_H

#include "vfs.h"

void       ramfs_init(vfs_node_t* mount_point);
vfs_ops_t* ramfs_get_ops(void);

#endif