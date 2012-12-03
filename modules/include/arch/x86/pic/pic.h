#define PIC_ROUTINE_SLOTS              16

#define PIC_COMMAND0                   0x20
#define PIC_DATA0                      0x21
#define PIC_COMMAND1                   0xA0
#define PIC_DATA1                      0xA1

#define PIC_COMMAND_CONFIG             0x11
#define PIC_COMMAND_EOI                0x20
#define PIC_DATA_8086                  0x01
#define PIC_DATA_VECTOR0               0x20
#define PIC_DATA_VECTOR1               0x28

struct pic_general_registers
{

    unsigned int edi;
    unsigned int esi;
    unsigned int ebp;
    unsigned int esp;
    unsigned int ebx;
    unsigned int edx;
    unsigned int ecx;
    unsigned int eax;

};

struct pic_interrupt_registers
{

    unsigned int eip;
    unsigned int cs;
    unsigned int eflags;
    unsigned int esp;
    unsigned int ss;

};

struct pic_registers
{

    unsigned int ds;
    struct pic_general_registers general;
    unsigned int index;
    unsigned int slave;
    struct pic_interrupt_registers interrupt;

};

struct pic_routine
{

    struct base_device *device;
    void (*callback)(struct base_device *device);

};

void pic_routine00();
void pic_routine01();
void pic_routine02();
void pic_routine03();
void pic_routine04();
void pic_routine05();
void pic_routine06();
void pic_routine07();
void pic_routine08();
void pic_routine09();
void pic_routine0A();
void pic_routine0B();
void pic_routine0C();
void pic_routine0D();
void pic_routine0E();
void pic_routine0F();
void pic_interrupt(struct pic_registers *registers);
void pic_enable();
void pic_disable();
unsigned int pic_set_routine(unsigned int index, struct base_device *device, void (*callback)(struct base_device *device));
unsigned int pic_unset_routine(unsigned int index, struct base_device *device);
