#include <fudge.h>
#include <kernel.h>
#include <base/base.h>
#include <system/system.h>
#include <timer/timer.h>
#include <arch/x86/pic/pic.h>
#include <arch/x86/io/io.h>
#include <arch/x86/platform/platform.h>

#define FREQUENCY                       1193182
#define REGISTERCHANNEL0                0x0000
#define REGISTERCHANNEL1                0x0001
#define REGISTERCHANNEL2                0x0002
#define REGISTERCOMMAND                 0x0003
#define COMMANDBINARY                   0x00
#define COMMANDBCD                      0x01
#define COMMANDMODE0                    0x00
#define COMMANDMODE1                    0x02
#define COMMANDMODE2                    0x04
#define COMMANDMODE3                    0x06
#define COMMANDMODE4                    0x08
#define COMMANDMODE5                    0x0A
#define COMMANDLATCH                    0x00
#define COMMANDLOW                      0x10
#define COMMANDHIGH                     0x20
#define COMMANDBOTH                     0x30
#define COMMANDCHANNEL0                 0x00
#define COMMANDCHANNEL1                 0x40
#define COMMANDCHANNEL2                 0x80
#define COMMANDREADBACK                 0xC0

static struct base_driver driver;
static struct timer_interface timerinterface;
static unsigned short io;
static unsigned short divisor;
static unsigned int jiffies;

static void handleirq(unsigned int irq)
{

    jiffies += 1;

    if (jiffies < 1000)
        return;

    timer_notify(&timerinterface, 4, 1, &jiffies);

    jiffies = 0;

}

static void driver_init()
{

    jiffies = 0;
    divisor = 1000;

    timer_initinterface(&timerinterface);

}

static unsigned int driver_match(unsigned int id)
{

    return id == PLATFORM_PIT;

}

static void driver_attach(unsigned int id)
{

    io = platform_getbase(id);

    io_outb(io + REGISTERCOMMAND, COMMANDCHANNEL0 | COMMANDBOTH | COMMANDMODE3 | COMMANDBINARY);
    io_outb(io + REGISTERCHANNEL0, divisor);
    io_outb(io + REGISTERCHANNEL0, divisor >> 8);
    timer_registerinterface(&timerinterface, id);
    pic_setroutine(platform_getirq(id), handleirq);

}

static void driver_detach(unsigned int id)
{

    timer_unregisterinterface(&timerinterface);
    pic_unsetroutine(platform_getirq(id));

}

void module_init()
{

    base_initdriver(&driver, "pit", driver_init, driver_match, driver_attach, driver_detach);

}

void module_register()
{

    base_registerdriver(&driver, PLATFORM_BUS);

}

void module_unregister()
{

    base_unregisterdriver(&driver, PLATFORM_BUS);

}

