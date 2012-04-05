#include <lib/memory.h>
#include <kernel/modules.h>
#include <kernel/vfs.h>
#include <modules/nodefs/nodefs.h>
#include <modules/rtl8139/rtl8139.h>

static struct rtl8139_driver driver;
static struct nodefs_node mac;
static struct nodefs_node data;

static unsigned int mac_read(struct nodefs_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    memory_copy(buffer, driver.mac, 6);

    return 6;

}

static unsigned int data_read(struct nodefs_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    return driver.read(&driver, buffer);

}

static unsigned int data_write(struct nodefs_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    return driver.write(&driver, count, buffer);

}

void init()
{

    rtl8139_driver_init(&driver);
    modules_register_driver(&driver.base);

    struct nodefs_driver *nodefsDriver = (struct nodefs_driver *)modules_get_driver(NODEFS_DRIVER_TYPE);

    if (!nodefsDriver)
        return;

    nodefsDriver->register_node(nodefsDriver, &mac, "rtl8139/mac", &driver.base.base, mac_read, 0);
    nodefsDriver->register_node(nodefsDriver, &data, "rtl8139/data", &driver.base.base, data_read, data_write);

}

void destroy()
{

    modules_unregister_driver(&driver.base);

}

