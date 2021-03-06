#include <fudge.h>
#include "resource.h"
#include "binary.h"
#include "mailbox.h"
#include "task.h"
#include "core.h"
#include "service.h"
#include "kernel.h"

static struct task tasks[KERNEL_TASKS];
static struct mailbox mailboxes[KERNEL_MAILBOXES];
static struct service_descriptor descriptors[KERNEL_DESCRIPTORS * KERNEL_TASKS];
static struct service_mount mounts[KERNEL_MOUNTS];
static struct list usedtasks;
static struct list freetasks;
static struct list blockedtasks;
static struct list usedmounts;
static struct list freemounts;
static struct list usedmailboxes;
static struct list freemailboxes;
static struct core *(*coreget)(void);
static void (*coreassign)(struct task *task);

static unsigned int walkmount(struct service_descriptor *descriptor, struct service_mountpoint *from, struct service_mountpoint *to)
{

    if (descriptor->backend == from->backend && descriptor->protocol == from->protocol && descriptor->id == from->id)
    {

        descriptor->backend = to->backend;
        descriptor->protocol = to->protocol;
        descriptor->id = to->id;

        return 1;

    }

    return 0;

}

static void walkmountparent(struct service_descriptor *descriptor)
{

    struct list_item *current;

    spinlock_acquire(&usedmounts.spinlock);

    for (current = usedmounts.head; current; current = current->next)
    {

        struct service_mount *mount = current->data;

        if (walkmount(descriptor, &mount->child, &mount->parent))
            break;

    }

    spinlock_release(&usedmounts.spinlock);

}

static void walkmountchild(struct service_descriptor *descriptor)
{

    struct list_item *current;

    spinlock_acquire(&usedmounts.spinlock);

    for (current = usedmounts.head; current; current = current->next)
    {

        struct service_mount *mount = current->data;

        if (walkmount(descriptor, &mount->parent, &mount->child))
            break;

    }

    spinlock_release(&usedmounts.spinlock);

}

unsigned int kernel_walk(struct service_descriptor *descriptor, char *path, unsigned int length)
{

    unsigned int offset = 0;

    while (offset < length)
    {

        char *cp = path + offset;
        unsigned int cl = length - offset;

        cl = memory_findbyte(cp, cl, '/');

        if (cl == 2 && cp[0] == '.' && cp[1] == '.')
        {

            walkmountparent(descriptor);

            descriptor->id = descriptor->protocol->parent(descriptor->backend, &descriptor->state, descriptor->id);

        }

        else
        {

            descriptor->id = descriptor->protocol->child(descriptor->backend, &descriptor->state, descriptor->id, cp, cl);

            walkmountchild(descriptor);

        }

        offset += cl + 1;

    }

    return offset >= length;

}

struct core *kernel_getcore(void)
{

    return coreget();

}

void kernel_setcallback(struct core *(*get)(void), void (*assign)(struct task *task))
{

    coreget = get;
    coreassign = assign;

}

struct task *kernel_picktask(void)
{

    struct list_item *current = list_picktail(&freetasks);

    return (current) ? current->data : 0;

}

struct service_mount *kernel_pickmount(void)
{

    struct list_item *current = list_picktail(&freemounts);

    return (current) ? current->data : 0;

}

struct mailbox *kernel_pickmailbox(void)
{

    struct list_item *current = list_picktail(&freemailboxes);

    return (current) ? current->data : 0;

}

void kernel_usetask(struct task *task)
{

    list_add(&usedtasks, &task->item);

}

void kernel_usemount(struct service_mount *mount)
{

    list_add(&usedmounts, &mount->item);

}

void kernel_usemailbox(struct mailbox *mailbox)
{

    list_add(&usedmailboxes, &mailbox->item);

}

void kernel_freetask(struct task *task)
{

    list_add(&freetasks, &task->item);

}

void kernel_freemount(struct service_mount *mount)
{

    list_add(&freemounts, &mount->item);

}

void kernel_freemailbox(struct mailbox *mailbox)
{

    list_add(&freemailboxes, &mailbox->item);

}

void kernel_assign(void)
{

    struct list_item *current;

    spinlock_acquire(&blockedtasks.spinlock);

    for (current = blockedtasks.head; current; current = current->next)
    {

        struct task *task = current->data;

        if (ring_count(&mailboxes[task->id].ring))
        {

            list_remove_nolock(&blockedtasks, current);
            coreassign(task);

        }

    }

    spinlock_release(&blockedtasks.spinlock);
    spinlock_acquire(&usedtasks.spinlock);

    for (current = usedtasks.head; current; current = current->next)
    {

        struct task *task = current->data;

        list_remove_nolock(&usedtasks, current);
        coreassign(task);

    }

    spinlock_release(&usedtasks.spinlock);

}

struct service_descriptor *kernel_getdescriptor(struct task *task, unsigned int descriptor)
{

    return &descriptors[task->id * KERNEL_DESCRIPTORS + (descriptor & (KERNEL_DESCRIPTORS - 1))];

}

static void copydescriptor(struct service_descriptor *tdescriptor, struct service_descriptor *sdescriptor, struct task *task)
{

    tdescriptor->backend = (sdescriptor) ? sdescriptor->backend : 0;
    tdescriptor->protocol = (sdescriptor) ? sdescriptor->protocol : 0;
    tdescriptor->id = (sdescriptor) ? sdescriptor->id : 0;

}

void kernel_copydescriptors(struct task *source, struct task *target)
{

    unsigned int i;

    for (i = 0x00; i < 0x04; i++)
    {

        copydescriptor(kernel_getdescriptor(target, i + 0x00), kernel_getdescriptor(source, i + 0x04), target);
        copydescriptor(kernel_getdescriptor(target, i + 0x04), kernel_getdescriptor(source, i + 0x04), target);

    }

}

void kernel_reset(unsigned int id)
{

    mailbox_reset(&mailboxes[id]);

}

unsigned int kernel_pick(unsigned int id, struct ipc_header *header, void *data)
{

    unsigned int count = mailbox_pick(&mailboxes[id], header, data);

    if (!count)
        list_add(&blockedtasks, &tasks[id].item);

    return count;

}

unsigned int kernel_place(unsigned int id, struct ipc_header *header, void *data)
{

    return mailbox_place(&mailboxes[id], header, data);

}

void kernel_notify(struct list *states, unsigned int type, void *buffer, unsigned int count)
{

    struct ipc_header header;
    struct list_item *current;

    ipc_init(&header, type, count);
    spinlock_acquire(&states->spinlock);

    for (current = states->head; current; current = current->next)
    {

        struct service_state *state = current->data;

        kernel_place(state->id, &header, buffer);

    }

    spinlock_release(&states->spinlock);

}

unsigned int kernel_setupbinary(struct task *task, unsigned int descriptor, unsigned int sp)
{

    struct service_descriptor *init = kernel_getdescriptor(task, descriptor);

    if (!init)
        return 0;

    task->node.address = init->protocol->map(init->backend, &init->state, init->id);

    if (!task->node.address)
        return 0;

    task->format = binary_findformat(&task->node);

    if (!task->format)
        return 0;

    task->thread.ip = task->format->findentry(&task->node);
    task->thread.sp = sp;

    return 1;

}

void kernel_setupinit(struct task *task)
{

    struct service_descriptor *init = kernel_getdescriptor(task, 0x04);
    struct service_descriptor *root = kernel_getdescriptor(task, 0x05);
    struct service_descriptor *work = kernel_getdescriptor(task, 0x06);

    root->backend = service_findbackend(1000);
    root->protocol = service_findprotocol(1000);
    root->id = root->protocol->root(root->backend, &root->state);
    work->backend = root->backend;
    work->protocol = root->protocol;
    work->id = work->protocol->root(work->backend, &work->state);
    init->backend = root->backend;
    init->protocol = root->protocol;
    init->id = init->protocol->root(init->backend, &init->state);
    init->id = init->protocol->child(init->backend, &init->state, init->id, "bin", 3);
    init->id = init->protocol->child(init->backend, &init->state, init->id, "init", 4);

}

void kernel_setup(unsigned int mbaddress, unsigned int mbsize)
{

    unsigned int i;

    for (i = 1; i < KERNEL_TASKS; i++)
    {

        struct task *task = &tasks[i];
        unsigned int j;

        task_init(task, i);
        resource_register(&task->resource);
        kernel_freetask(task);

        for (j = 0; j < KERNEL_DESCRIPTORS; j++)
        {

            struct service_descriptor *descriptor = &descriptors[i * KERNEL_DESCRIPTORS + j];

            service_initdescriptor(descriptor, task->id);

        }

    }

    for (i = 0; i < KERNEL_MAILBOXES; i++)
    {

        struct mailbox *mailbox = &mailboxes[i];

        mailbox_init(mailbox, (char *)(mbaddress + i * mbsize), mbsize);
        resource_register(&mailbox->resource);
        kernel_freemailbox(mailbox);

    }

    for (i = 0; i < KERNEL_MOUNTS; i++)
    {

        struct service_mount *mount = &mounts[i];

        service_initmount(mount);
        kernel_freemount(mount);

    }

}

