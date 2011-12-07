#include <lib/string.h>
#include <kernel/arch/x86/io.h>
#include <kernel/vfs.h>
#include <kernel/event.h>
#include <kernel/modules.h>
#include <kernel/kernel.h>
#include <modules/pit/pit.h>

static struct pit_device device;

static unsigned int pit_device_read_read(struct vfs_node *self, unsigned int count, void *buffer)
{

    string_write_format(buffer, "%d", device.jiffies);

    return string_length(buffer);

}

static struct vfs_node *pit_device_view_find_node(struct vfs_view *self, char *name)
{

    if (!string_compare("jiffies", name))
        return &device.read;

    return 0;

}

static struct vfs_node *pit_device_view_walk(struct vfs_view *self, unsigned int index)
{

    if (index == 0)
        return &device.read;

    return 0;

}

static char *pit_device_view_get_name(struct vfs_view *self, struct vfs_node *node)
{

    if (node->id == 0)
        return "jiffies";

    return 0;

}

void pit_device_init(struct pit_device *device)
{

    modules_device_init(&device->base, PIT_DEVICE_TYPE);
    device->divisor = PIT_HERTZ / PIT_FREQUENCY;
    device->jiffies = 0;

    vfs_node_init(&device->read, 0, 0, 0, pit_device_read_read, 0);
    vfs_view_init(&device->view, "pit", pit_device_view_find_node, pit_device_view_walk, pit_device_view_get_name);

    device->base.module.view = &device->view;

    io_outb(0x43, 0x36);
    io_outb(0x40, (unsigned char)(device->divisor & 0xFF));
    io_outb(0x40, (unsigned char)((device->divisor >> 8) & 0xFF));

}

static void handle_irq()
{

    struct pit_device *pit = &device;

    pit->jiffies += 1;

    event_handle(EVENT_IRQ_PIT);

}

void init()
{

    pit_device_init(&device);

    kernel_register_irq(0x00, handle_irq);

    modules_register_device(&device.base);

}

void destroy()
{

    kernel_unregister_irq(0x00);

    modules_unregister_device(&device.base);

}

