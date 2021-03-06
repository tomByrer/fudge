#include <fudge.h>
#include <kernel.h>
#include <modules/system/system.h>

static struct system_node eventnode;
static struct system_node multicastnode;

static unsigned int multicast(struct service_state *source, struct list *targets, struct ipc_header *header, void *data)
{

    struct list_item *current;

    spinlock_acquire(&targets->spinlock);

    for (current = targets->head; current; current = current->next)
    {

        struct service_state *target = current->data;

        header->source = source->id;

        kernel_place(target->id, header, data);

    }

    spinlock_release(&targets->spinlock);

    return header->length;

}

static unsigned int eventnode_seek(struct system_node *self, struct service_state *state, unsigned int offset)
{

    return 0;

}

static unsigned int multicastnode_write(struct system_node *self, struct system_node *current, struct service_state *state, void *buffer, unsigned int count, unsigned int offset)
{

    struct ipc_header *header = buffer;

    return multicast(state, &eventnode.states, header, header + 1);

}

static unsigned int multicastnode_seek(struct system_node *self, struct service_state *state, unsigned int offset)
{

    return 0;

}

void module_init(void)
{

    system_initnode(&eventnode, SYSTEM_NODETYPE_NORMAL, "event");

    eventnode.operations.seek = eventnode_seek;

    system_initnode(&multicastnode, SYSTEM_NODETYPE_NORMAL, "multicast");

    multicastnode.operations.write = multicastnode_write;
    multicastnode.operations.seek = multicastnode_seek;

}

void module_register(void)
{

    system_registernode(&eventnode);
    system_registernode(&multicastnode);

}

void module_unregister(void)
{

    system_unregisternode(&eventnode);
    system_unregisternode(&multicastnode);

}

