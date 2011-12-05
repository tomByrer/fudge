#include <kernel/runtime.h>
#include <kernel/arch/x86/idt.h>
#include <kernel/arch/x86/io.h>
#include <kernel/arch/x86/irq.h>
#include <kernel/arch/x86/isr.h>

static void *routines[IRQ_ROUTINE_SLOTS];

void irq_register_routine(unsigned char index, void (*routine)())
{

    routines[index] = routine;

}

void irq_unregister_routine(unsigned char index)
{

    routines[index] = 0;

}

void irq_handle(struct irq_registers *registers)
{

    struct runtime_task *task = runtime_get_running_task();

    if (task)
    {

        void *ip = (void *)registers->eip;
        void *sp = (void *)registers->useresp;
        void *sb = (void *)registers->ebp;

        task->save_registers(task, ip, sp, sb);

    }

    void (*routine)() = routines[registers->index];

    if (routine)
        routine();

    if (registers->slave)
        io_outb(0xA0, 0x20);

    io_outb(0x20, 0x20);

    struct runtime_task *atask = runtime_get_running_task();

    if (atask)
    {

        registers->eip = (unsigned int)atask->registers.ip;
        registers->useresp = (unsigned int)atask->registers.sp;
        registers->ebp = (unsigned int)atask->registers.sb;

    }

}

static void irq_remap()
{

    io_outb(0x20, 0x11);
    io_outb(0xA0, 0x11);
    io_outb(0x21, 0x20);
    io_outb(0xA1, 0x28);
    io_outb(0x21, 0x04);
    io_outb(0xA1, 0x02);
    io_outb(0x21, 0x01);
    io_outb(0xA1, 0x01);
    io_outb(0x21, 0x00);
    io_outb(0xA1, 0x00);

}

void irq_init()
{

    irq_remap();

    idt_set_gate(0x20, irq00, 0x08, 0x8E);
    idt_set_gate(0x21, irq01, 0x08, 0x8E);
    idt_set_gate(0x22, irq02, 0x08, 0x8E);
    idt_set_gate(0x23, irq03, 0x08, 0x8E);
    idt_set_gate(0x24, irq04, 0x08, 0x8E);
    idt_set_gate(0x25, irq05, 0x08, 0x8E);
    idt_set_gate(0x26, irq06, 0x08, 0x8E);
    idt_set_gate(0x27, irq07, 0x08, 0x8E);
    idt_set_gate(0x28, irq08, 0x08, 0x8E);
    idt_set_gate(0x29, irq09, 0x08, 0x8E);
    idt_set_gate(0x2A, irq0A, 0x08, 0x8E);
    idt_set_gate(0x2B, irq0B, 0x08, 0x8E);
    idt_set_gate(0x2C, irq0C, 0x08, 0x8E);
    idt_set_gate(0x2D, irq0D, 0x08, 0x8E);
    idt_set_gate(0x2E, irq0E, 0x08, 0x8E);
    idt_set_gate(0x2F, irq0F, 0x08, 0x8E);

}

