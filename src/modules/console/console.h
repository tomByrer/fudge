struct console_interface
{

    struct resource resource;
    struct system_node root;
    struct system_node ctrl;
    struct system_node data;
    struct system_node odata;
    struct system_node event;
    unsigned int id;

};

void console_notify(struct console_interface *interface, void *buffer, unsigned int count);
void console_notifydata(struct console_interface *interface, unsigned char data);
void console_registerinterface(struct console_interface *interface);
void console_unregisterinterface(struct console_interface *interface);
void console_initinterface(struct console_interface *interface, unsigned int id);
