#include <module.h>
#include <kernel/resource.h>
#include <system/system.h>
#include <base/base.h>
#include <console/console.h>
#include <video/video.h>
#include <arch/x86/io/io.h>
#include <arch/x86/pci/pci.h>
#include "registers.h"
#include "timing.h"

#define VGA_TEXT_LIMIT                  2000
#define VGA_COLORMAP_LIMIT              256

struct vga_character
{

    char character;
    char color;

};

static struct base_driver driver;
static struct console_interface consoleinterface;
static struct video_interface videointerface;
static struct {unsigned char color; unsigned short offset;} cursor;
static void *taddress;
static void *gaddress;

/* BIOS mode 0Dh - 320x200x16 */
/*
static const unsigned char g320x200x16[60] = {
    0x2D, 0x27, 0x28, 0x90, 0x2B, 0x80, 0xBF, 0x1F, 0x00, 0xC0, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x9C, 0x8E, 0x8F, 0x14, 0x00, 0x96, 0xB9, 0xE3,
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B,
    0x0C, 0x0D, 0x0E, 0x0F, 0x01, 0x00, 0x0F, 0x00, 0x00,
    0x00, 0x0F, 0x00, 0x20, 0x00, 0x00, 0x05, 0x0F, 0xFF,
    0x03, 0x09, 0x0F, 0x00, 0x06,
    0x63
};
*/

/* BIOS mode 0Eh - 640x200x16 */
/*
static const unsigned char g640x200x16[60] = {
    0x5F, 0x4F, 0x50, 0x82, 0x54, 0x80, 0xBF, 0x1F, 0x00, 0xC0, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x9C, 0x8E, 0x8F, 0x28, 0x00, 0x96, 0xB9, 0xE3,
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B,
    0x0C, 0x0D, 0x0E, 0x0F, 0x01, 0x00, 0x0F, 0x00, 0x00,
    0x00, 0x0F, 0x00, 0x20, 0x00, 0x00, 0x05, 0x0F, 0xFF,
    0x03, 0x01, 0x0F, 0x00, 0x06,
    0x63
};
*/

/* BIOS mode 10h - 640x350x16 */
/*
static const unsigned char g640x350x16[60] = {
    0x5F, 0x4F, 0x50, 0x82, 0x54, 0x80, 0xBF, 0x1F, 0x00, 0x40, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x83, 0x85, 0x5D, 0x28, 0x0F, 0x63, 0xBA, 0xE3,
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B,
    0x0C, 0x0D, 0x0E, 0x0F, 0x01, 0x00, 0x0F, 0x00, 0x00,
    0x00, 0x0F, 0x00, 0x20, 0x00, 0x00, 0x05, 0x0F, 0xFF,
    0x03, 0x01, 0x0F, 0x00, 0x06,
    0xA3
};
*/

/* BIOS mode 12h - 640x480x16 */
/*
static const unsigned char g640x480x16[60] = {
    0x5F, 0x4F, 0x50, 0x82, 0x54, 0x80, 0x0B, 0x3E, 0x00, 0x40, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0xEA, 0x8C, 0xDF, 0x28, 0x00, 0xE7, 0x04, 0xE3,
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B,
    0x0C, 0x0D, 0x0E, 0x0F, 0x01, 0x00, 0x0F, 0x00, 0x00,
    0x00, 0x0F, 0x00, 0x20, 0x00, 0x00, 0x05, 0x0F, 0xFF,
    0x03, 0x01, 0x0F, 0x00, 0x06,
    0xE3
};
*/

/* BIOS mode 13h - 320x200x256 */
static const unsigned char g320x200x256[60] = {
    0x5F, 0x4F, 0x50, 0x82, 0x54, 0x80, 0xBF, 0x1F, 0x00, 0x41, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x9C, 0x8E, 0x8F, 0x28, 0x40, 0x96, 0xB9, 0xA3,
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B,
    0x0C, 0x0D, 0x0E, 0x0F, 0x41, 0x00, 0x0F, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x05, 0x0F, 0xFF,
    0x03, 0x01, 0x0F, 0x00, 0x0E,
    0x63
};

/* non-BIOS mode - 320x240x256 */
/*
static const unsigned char g320x240x256[60] = {
    0x5F, 0x4F, 0x50, 0x82, 0x54, 0x80, 0x0D, 0x3E, 0x00, 0x41, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0xEA, 0xAC, 0xDF, 0x28, 0x00, 0xE7, 0x06, 0xE3,
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B,
    0x0C, 0x0D, 0x0E, 0x0F, 0x41, 0x00, 0x0F, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x05, 0x0F, 0xFF,
    0x03, 0x01, 0x0F, 0x00, 0x06,
    0xE3
};
*/

/* non-BIOS mode - 320x400x256 */
/*
static const unsigned char g320x400x256[60] = {
    0x5F, 0x4F, 0x50, 0x82, 0x54, 0x80, 0xBF, 0x1F, 0x00, 0x40, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x9C, 0x8E, 0x8F, 0x28, 0x00, 0x96, 0xB9, 0xE3,
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B,
    0x0C, 0x0D, 0x0E, 0x0F, 0x41, 0x00, 0x0F, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x05, 0x0F, 0xFF,
    0x03, 0x01, 0x0F, 0x00, 0x06,
    0x63
};
*/

/* non-BIOS mode - 360x480x256 */
/*
static const unsigned char g360x480x256[60] = {
    0x6B, 0x59, 0x5A, 0x8E, 0x5E, 0x8A, 0x0D, 0x3E, 0x00, 0x40, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0xEA, 0xAC, 0xDF, 0x2D, 0x00, 0xE7, 0x06, 0xE3,
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B,
    0x0C, 0x0D, 0x0E, 0x0F, 0x41, 0x00, 0x0F, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x05, 0x0F, 0xFF,
    0x03, 0x01, 0x0F, 0x00, 0x06,
    0xE7
};
*/

/* non BIOS mode - 720x348x2 based on mode 10h */
/*
static const unsigned char g720x348x2[60] = {
    0x6B, 0x59, 0x5A, 0x8E, 0x5E, 0x8A, 0xBF, 0x1F, 0x00, 0x40, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x83, 0x85, 0x5D, 0x2D, 0x0F, 0x63, 0xBA, 0xE3,
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B,
    0x0C, 0x0D, 0x0E, 0x0F, 0x01, 0x00, 0x0F, 0x00, 0x00,
    0x00, 0x0F, 0x00, 0x20, 0x00, 0x00, 0x05, 0x0F, 0xFF,
    0x03, 0x01, 0x0F, 0x00, 0x06,
    0xA7
};
*/

/* non-BIOS mode - 400x300x256 */
/*
static const unsigned char g400x300x256X[60] = {
    0x71, 0x63, 0x64, 0x92, 0x65, 0x82, 0x46, 0x1F, 0x00, 0x40, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x31, 0x80, 0x2B, 0x32, 0x00, 0x2F, 0x44, 0xE3,
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B,
    0x0C, 0x0D, 0x0E, 0x0F, 0x41, 0x00, 0x0F, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x05, 0x0F, 0xFF,
    0x03, 0x01, 0x0F, 0x00, 0x06,
    0xA7
};
*/

static void clear(unsigned int offset)
{

    struct vga_character *memory = taddress;
    unsigned int i;

    for (i = offset; i < VGA_TEXT_LIMIT; i++)
    {

        memory[i].character = ' ';
        memory[i].color = cursor.color;

    }

}

static unsigned int consoleinterface_rdata(unsigned int offset, unsigned int count, void *buffer)
{

    return 0;

}

static unsigned int consoleinterface_wdata(unsigned int offset, unsigned int count, void *buffer)
{

    struct vga_character *memory = taddress;
    unsigned int bytespp = videointerface.settings.bpp / 8;
    unsigned int linesize = videointerface.settings.w * bytespp;
    unsigned int fullsize = videointerface.settings.h * linesize;
    unsigned int i;

    for (i = 0; i < count; i++)
    {

        char c = ((char *)buffer)[i];

        if (c == '\b')
            cursor.offset--;

        if (c == '\t')
            cursor.offset = (cursor.offset + 8) & ~(8 - 1);

        if (c == '\r')
            cursor.offset -= (cursor.offset % 80);

        if (c == '\n')
            cursor.offset += 80 - (cursor.offset % 80);

        if (c >= ' ')
        {

            memory[cursor.offset].character = c;
            memory[cursor.offset].color = cursor.color;
            cursor.offset++;

        }

        if (cursor.offset >= VGA_TEXT_LIMIT)
        {

            memory_read(taddress, fullsize, taddress, fullsize, linesize);
            clear(80 * 24);
            cursor.offset -= 80;

        }

    }

    outcrt1(VGA_CRTINDEX_CRT0E, cursor.offset >> 8);
    outcrt1(VGA_CRTINDEX_CRT0F, cursor.offset);

    return count;

}

static void videointerface_setmode(unsigned int xres, unsigned int yres, unsigned int bpp)
{

    videointerface.settings.w = 320;
    videointerface.settings.h = 200;
    videointerface.settings.bpp = 8;

    io_inb(VGA_REGISTER_FCCCTRL);
    io_outb(VGA_REGISTER_ARINDEX, VGA_ARINDEX_DISABLE);

    vga_setregisters((unsigned char *)g320x200x256, 0);

    io_inb(VGA_REGISTER_FCCCTRL);
    io_outb(VGA_REGISTER_ARINDEX, VGA_ARINDEX_ENABLE);

}

static unsigned int videointerface_rdata(unsigned int offset, unsigned int count, void *buffer)
{

    unsigned int bytespp = videointerface.settings.bpp / 8;
    unsigned int linesize = videointerface.settings.w * bytespp;
    unsigned int fullsize = videointerface.settings.h * linesize;

    return memory_read(buffer, count, gaddress, fullsize, offset);

}

static unsigned int videointerface_wdata(unsigned int offset, unsigned int count, void *buffer)
{

    unsigned int bytespp = videointerface.settings.bpp / 8;
    unsigned int linesize = videointerface.settings.w * bytespp;
    unsigned int fullsize = videointerface.settings.h * linesize;

    return memory_write(gaddress, fullsize, buffer, count, offset);

}

static unsigned int videointerface_rcolormap(unsigned int offset, unsigned int count, void *buffer)
{

    char *c = buffer;
    unsigned int i;

    if (count > VGA_COLORMAP_LIMIT)
        count = VGA_COLORMAP_LIMIT;

    if (offset > count)
        return 0;

    for (i = offset; i < count; i += 3)
    {

        io_outb(VGA_REGISTER_DACRINDEX, i / 3);
        c[i + 0] = io_inb(VGA_REGISTER_DACDATA);
        c[i + 1] = io_inb(VGA_REGISTER_DACDATA);
        c[i + 2] = io_inb(VGA_REGISTER_DACDATA);

    }

    return i - offset;

}

static unsigned int videointerface_wcolormap(unsigned int offset, unsigned int count, void *buffer)
{

    char *c = buffer;
    unsigned int i;

    if (count > VGA_COLORMAP_LIMIT)
        count = VGA_COLORMAP_LIMIT;

    if (offset > count)
        return 0;

    for (i = offset; i < count; i += 3)
    {

        io_outb(VGA_REGISTER_DACWINDEX, i / 3);
        io_outb(VGA_REGISTER_DACDATA, c[i + 0]);
        io_outb(VGA_REGISTER_DACDATA, c[i + 1]);
        io_outb(VGA_REGISTER_DACDATA, c[i + 2]);

    }

    return i - offset;

}

static unsigned int driver_match(struct base_bus *bus, unsigned int id)
{

    if (bus->type != PCI_BUS_TYPE)
        return 0;

    return pci_inb(bus, id, PCI_CONFIG_CLASS) == PCI_CLASS_DISPLAY && pci_inb(bus, id, PCI_CONFIG_SUBCLASS) == PCI_CLASS_DISPLAY_VGA && pci_inb(bus, id, PCI_CONFIG_INTERFACE) == 0x00;

}

static void driver_attach(struct base_bus *bus, unsigned int id)
{

    console_initinterface(&consoleinterface, &driver, bus, id, consoleinterface_rdata, consoleinterface_wdata);
    console_registerinterface(&consoleinterface);
    video_initinterface(&videointerface, &driver, bus, id, videointerface_setmode, videointerface_rdata, videointerface_wdata, videointerface_rcolormap, videointerface_wcolormap);
    video_registerinterface(&videointerface);

    taddress = (void *)0x000B8000;
    gaddress = (void *)0x000A0000;
    cursor.color = 0x0F;
    consoleinterface.settings.scroll = 1;
    videointerface.settings.w = 80;
    videointerface.settings.h = 25;
    videointerface.settings.bpp = 16;

    clear(0);

}

static void driver_detach(struct base_bus *bus, unsigned int id)
{

    console_unregisterinterface(&consoleinterface);
    video_unregisterinterface(&videointerface);

}

void init()
{

    base_initdriver(&driver, "vga", driver_match, driver_attach, driver_detach);
    base_registerdriver(&driver);

}

void destroy()
{

    base_unregisterdriver(&driver);

}

