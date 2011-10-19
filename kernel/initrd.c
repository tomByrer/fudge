#include <lib/memory.h>
#include <lib/string.h>
#include <lib/tar.h>
#include <kernel/vfs.h>
#include <kernel/initrd.h>

static struct initrd_filesystem initrdFilesystem;
static struct vfs_node initrdRoot;

static unsigned int initrd_node_read(struct vfs_node *node, unsigned int count, void *buffer)
{

    struct initrd_node *initrdNode = &initrdFilesystem.nodes[node->id];

    if (count > initrdNode->size)
        count = initrdNode->size;

    memory_copy(buffer, initrdNode->data, count);

    return count;

}

static unsigned int initrd_get_file_size(const char *in)
{

    unsigned int size = 0;
    unsigned int j;
    unsigned int count = 1;

    for (j = 11; j > 0; j--, count *= 8)
        size += ((in[j - 1] - '0') * count);

    return size;

}

static unsigned int initrd_parse(unsigned int address)
{

    unsigned int i;

    for (i = 0; ; i++)
    {

        struct tar_header *header = (struct tar_header *)address;

        if (header->name[0] == '\0')
            break;

        unsigned int size = initrd_get_file_size(header->size);
        unsigned int start = string_index_reversed(header->name, '/', (header->typeflag[0] == TAR_FILETYPE_DIR) ? 1 : 0) + 1;

        struct initrd_node *initrdFileNode = &initrdFilesystem.nodes[i];
        string_copy(initrdFileNode->name, header->name + start);
        initrdFileNode->size = size;
        initrdFileNode->header = header;
        initrdFileNode->data = (void *)(address + TAR_BLOCK_SIZE);
        initrdFileNode->base.id = i;
        initrdFileNode->base.physical = (void *)address;
        initrdFileNode->base.operations.read = initrd_node_read;

        address += ((size / TAR_BLOCK_SIZE) + 1) * TAR_BLOCK_SIZE;

        if (size % TAR_BLOCK_SIZE)
            address += TAR_BLOCK_SIZE;

    }

    return i;

}

static struct vfs_node *initrd_filesystem_lookup(struct vfs_filesystem *filesystem, char *path)
{

    unsigned int i;

    for (i = 0; string_length(initrdFilesystem.nodes[i].name); i++)
    {

        unsigned int count = string_length(initrdFilesystem.nodes[i].name) + 1;

        if (!memory_compare(path, initrdFilesystem.nodes[i].name, count))
            return &initrdFilesystem.nodes[i].base;

    }

    return 0;

}

static unsigned int initrd_filesystem_read(struct vfs_node *node, unsigned int count, void *buffer)
{

    memory_set(buffer, 0, 1);
    unsigned int i;

    for (i = 0; i < initrdFilesystem.nodesCount; i++)
    {

        string_concat(buffer, initrdFilesystem.nodes[i].name);
        string_concat(buffer, "\n");

    }

    return string_length(buffer);

}

void initrd_init(unsigned int *address)
{

    string_copy(initrdFilesystem.base.name, "initrd");
    initrdFilesystem.base.root = &initrdRoot;
    initrdFilesystem.base.root->operations.read = initrd_filesystem_read;
    initrdFilesystem.base.lookup = initrd_filesystem_lookup;
    initrdFilesystem.nodesCount = initrd_parse(*address);
    vfs_register_filesystem(&initrdFilesystem.base);

}

