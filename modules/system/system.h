enum system_nodetype
{

    SYSTEM_NODETYPE_NORMAL              = 0,
    SYSTEM_NODETYPE_GROUP               = 1,
    SYSTEM_NODETYPE_MULTI               = 2,
    SYSTEM_NODETYPE_HIDDEN              = 4

};

struct system_header
{

    char id[12];
    struct system_node *root;

};

struct system_node
{

    struct list_item item;
    struct system_node *parent;
    struct list children;
    enum system_nodetype type;
    const char *name;
    unsigned int index;
    unsigned int used;
    unsigned int (*open)(struct system_node *self);
    unsigned int (*close)(struct system_node *self);
    unsigned int (*read)(struct system_node *self, unsigned int offset, unsigned int count, void *buffer);
    unsigned int (*write)(struct system_node *self, unsigned int offset, unsigned int count, void *buffer);
    unsigned int (*child)(struct system_node *self, unsigned int count, const char *path);

};

void system_addchild(struct system_node *group, struct system_node *node);
void system_removechild(struct system_node *group, struct system_node *node);
void system_registernode(struct system_node *node);
void system_unregisternode(struct system_node *node);
void system_initnode(struct system_node *node, unsigned int type, const char *name);
