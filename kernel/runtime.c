#include <lib/elf.h>
#include <lib/memory.h>
#include <lib/string.h>
#include <kernel/vfs.h>
#include <kernel/modules.h>
#include <kernel/runtime.h>
#include <kernel/arch/x86/mmu.h>
#include <modules/elf/elf.h>

struct runtime_task runtimeTasks[8];

struct runtime_task *runtime_get_running_task()
{

    return &runtimeTasks[0];

}

static unsigned int runtime_load(struct runtime_task *task, char *path, unsigned int argc, char **argv)
{

    struct mmu_header *pHeader = mmu_get_program_header();

    struct vfs_node *node = vfs_find(path);

    if (!(node && node->operations.read))
        return 0;

    node->operations.read(node, 0x10000, pHeader->address);

    struct elf_header *header = elf_get_header(pHeader->address);

    if (!header)
        return 0;

    struct elf_program_header *programHeader = elf_get_program_header(header);

    char **sa = pHeader->address + 0xFC00;
    void *ss = pHeader->address + 0xFD00;

    unsigned int i;
    unsigned int offset = 0;

    for (i = 0; i < argc; i++)
    {

        sa[i] = programHeader->virtualAddress + 0xFD00 + offset;

        unsigned int length = string_length(argv[i]);
        string_copy(ss + offset, argv[i]);

        offset += length + 2;

    }

    argv = programHeader->virtualAddress + 0xFC00;
    unsigned int eip = 0;

    memory_set(pHeader->address + 0xFFFF, ((unsigned int)argv & 0xFF000000) >> 24, 1);
    memory_set(pHeader->address + 0xFFFE, ((unsigned int)argv & 0x00FF0000) >> 16, 1);
    memory_set(pHeader->address + 0xFFFD, ((unsigned int)argv & 0x0000FF00) >> 8, 1);
    memory_set(pHeader->address + 0xFFFC, ((unsigned int)argv & 0x000000FF) >> 0, 1);
    memory_set(pHeader->address + 0xFFFB, (argc & 0xFF000000) >> 24, 1);
    memory_set(pHeader->address + 0xFFFA, (argc & 0x00FF0000) >> 16, 1);
    memory_set(pHeader->address + 0xFFF9, (argc & 0x0000FF00) >> 8, 1);
    memory_set(pHeader->address + 0xFFF8, (argc & 0x000000FF) >> 0, 1);
    memory_set(pHeader->address + 0xFFF7, (eip & 0xFF000000) >> 24, 1);
    memory_set(pHeader->address + 0xFFF6, (eip & 0x00FF0000) >> 16, 1);
    memory_set(pHeader->address + 0xFFF5, (eip & 0x0000FF00) >> 8, 1);
    memory_set(pHeader->address + 0xFFF4, (eip & 0x000000FF) >> 0, 1);

    mmu_map(pHeader, programHeader->virtualAddress, 0x10000, MMU_TABLE_FLAG_PRESENT | MMU_TABLE_FLAG_WRITEABLE | MMU_TABLE_FLAG_USERMODE, MMU_PAGE_FLAG_PRESENT | MMU_PAGE_FLAG_WRITEABLE | MMU_PAGE_FLAG_USERMODE);
    mmu_set_directory(&pHeader->directory);

    task->eip = header->entry;
    task->esp = programHeader->virtualAddress + 0xFFF4;

    return 1;

}

static struct vfs_descriptor *runtime_add_descriptor(struct runtime_task *task, struct vfs_node *node)
{

    unsigned int i;

    for (i = 0; i < 16; i++)
    {

        if (!task->descriptors[i].node)
        {

            task->descriptors[i].index = i;
            task->descriptors[i].node = node;
            task->descriptors[i].permissions = 0;

            return &task->descriptors[i];

        }

    }

    return 0;

}

static struct vfs_descriptor *runtime_get_descriptor(struct runtime_task *task, unsigned int index)
{

    return &task->descriptors[index];

}

static void runtime_remove_descriptor(struct runtime_task *task, unsigned int index)
{

    memory_set((void *)&task->descriptors[index], 0, sizeof (struct vfs_descriptor));

}

void runtime_init()
{

    unsigned int i;

    for (i = 0; i < 8; i++)
    {

        runtimeTasks[i].running = 0;
        runtimeTasks[i].load = runtime_load;
        runtimeTasks[i].add_descriptor = runtime_add_descriptor;
        runtimeTasks[i].get_descriptor = runtime_get_descriptor;
        runtimeTasks[i].remove_descriptor = runtime_remove_descriptor;

    }

}

