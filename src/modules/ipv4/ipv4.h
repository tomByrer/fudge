#define IPV4_PROTOCOL                   0x0800
#define IPV4_ADDRSIZE                   4

struct ipv4_header
{

    unsigned char version;
    unsigned char dscp;
    unsigned char length[2];
    unsigned char id[2];
    unsigned char flags[2];
    unsigned char ttl;
    unsigned char protocol;
    unsigned char checksum[2];
    unsigned char sip[4];
    unsigned char tip[4];

};

struct ipv4_arpentry
{

    unsigned char haddress[ETHERNET_ADDRSIZE];
    unsigned char paddress[IPV4_ADDRSIZE];

};

struct ipv4_protocol
{

    struct resource resource;
    struct system_node root;
    struct system_node data;
    struct list datalinks;
    unsigned char id;
    void (*notify)(struct ethernet_interface *interface, struct ipv4_header *header, void *buffer, unsigned int count);

};

void *ipv4_writeheader(void *buffer, unsigned char *sip, unsigned char *tip);
void ipv4_registerprotocol(struct ipv4_protocol *protocol);
void ipv4_unregisterprotocol(struct ipv4_protocol *protocol);
void ipv4_initprotocol(struct ipv4_protocol *protocol, char *name, unsigned char id, void (*notify)(struct ethernet_interface *interface, struct ipv4_header *header, void *buffer, unsigned int count));
