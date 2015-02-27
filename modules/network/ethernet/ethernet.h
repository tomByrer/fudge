struct ethernet_interface
{

    struct resource resource;
    struct system_node root;
    struct system_node ctrl;
    struct system_node data;
    unsigned int id;
    unsigned char mac[6];
    unsigned int (*send)(unsigned int count, void *buffer);

};

struct ethernet_protocol
{

    struct resource resource;
    struct system_node root;
    struct system_node data;
    unsigned int type;
    void (*addinterface)(struct ethernet_interface *interface);
    void (*removeinterface)(struct ethernet_interface *interface);
    void (*notify)(struct ethernet_interface *interface, unsigned int count, void *buffer);

};

void ethernet_notify(struct ethernet_interface *interface, unsigned int count, void *buffer);
void ethernet_registerinterface(struct ethernet_interface *interface, unsigned int id);
void ethernet_registerprotocol(struct ethernet_protocol *protocol);
void ethernet_unregisterinterface(struct ethernet_interface *interface);
void ethernet_unregisterprotocol(struct ethernet_protocol *protocol);
void ethernet_initinterface(struct ethernet_interface *interface, const char *name, unsigned int (*send)(unsigned int count, void *buffer));
void ethernet_initprotocol(struct ethernet_protocol *protocol, const char *name, unsigned short type, void (*addinterface)(struct ethernet_interface *interface), void (*removeinterface)(struct ethernet_interface *interface), void (*notify)(struct ethernet_interface *interface, unsigned int count, void *buffer));
