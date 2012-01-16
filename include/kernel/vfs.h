#ifndef KERNEL_VFS_H
#define KERNEL_VFS_H

#define VFS_FILESYSTEM_SLOTS 8

struct vfs_node
{

    char *name;
    unsigned int id;
    void *physical;
    void (*open)(struct vfs_node *self);
    void (*close)(struct vfs_node *self);
    unsigned int (*read)(struct vfs_node *self, unsigned int count, void *buffer);
    unsigned int (*write)(struct vfs_node *self, unsigned int count, void *buffer);

};

struct vfs_view
{

    struct vfs_node *(*find_node)(struct vfs_view *self, char *name);
    struct vfs_node *(*walk)(struct vfs_view *self, unsigned int index);

};

struct vfs_filesystem
{

    struct vfs_view *(*find_view)(struct vfs_filesystem *self, char *name);

};

extern void vfs_register_filesystem(struct vfs_filesystem *filesystem);
extern struct vfs_filesystem *vfs_get_filesystem(unsigned int index);
extern struct vfs_node *vfs_find(char *viewname, char *nodename);
extern void vfs_view_init(struct vfs_view *view, struct vfs_node *(*find_node)(struct vfs_view *self, char *name), struct vfs_node *(*walk)(struct vfs_view *self, unsigned int index));
extern void vfs_node_init(struct vfs_node *node, char *name, unsigned int id, void (*open)(struct vfs_node *self), void (*close)(struct vfs_node *self), unsigned int (*read)(struct vfs_node *self, unsigned int count, void *buffer), unsigned int (*write)(struct vfs_node *self, unsigned int count, void *buffer));
extern void vfs_filesystem_init(struct vfs_filesystem *filesystem, struct vfs_view *(find_view)(struct vfs_filesystem *self, char *name));

#endif

