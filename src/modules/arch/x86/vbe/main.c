#include <fudge.h>
#include <kernel.h>
#include <modules/base/base.h>
#include <modules/system/system.h>
#include <modules/video/video.h>
#include <modules/arch/x86/pci/pci.h>

struct vbe_mode_info
{

    unsigned short attributes;
    unsigned char window_a;
    unsigned char window_b;
    unsigned short granularity;
    unsigned short window_size;
    unsigned short segment_a;
    unsigned short segment_b;
    unsigned int win_func_ptr;
    unsigned short pitch;
    unsigned short width;
    unsigned short height;
    unsigned char w_char;
    unsigned char y_char;
    unsigned char planes;
    unsigned char bpp;
    unsigned char banks;
    unsigned char memory_model;
    unsigned char bank_size;
    unsigned char image_pages;
    unsigned char reserved0;
    unsigned char red_mask;
    unsigned char red_position;
    unsigned char green_mask;
    unsigned char green_position;
    unsigned char blue_mask;
    unsigned char blue_position;
    unsigned char reserved_mask;
    unsigned char reserved_position;
    unsigned char direct_color_attributes;
    unsigned int framebuffer;
    unsigned int off_screen_mem_off;
    unsigned short off_screen_mem_size;

} __attribute__ ((packed));

extern void *realmode_gdt;
extern void _get_video_mode(int);
extern void _set_video_mode(int);

static struct base_driver driver;
static struct video_interface videointerface;
static unsigned int fb;

static void run(void)
{

    struct vbe_mode_info *info = (struct vbe_mode_info *)0xd000;
    int (*getmode)(int) = (int (*)(int))0x8000;
    int (*setmode)(int) = (int (*)(int))0x8000;
    unsigned int mode_offset;

    debug_logs(DEBUG_INFO, "vbe loaded");
    memory_copy((void *)0x8000, (void *)(unsigned int)_get_video_mode, 0x1000);
    memory_copy((void *)0x9000, &realmode_gdt, 0x1000);
    getmode(0);
    debug_logs(DEBUG_INFO, "vbe worked!");
    debug_log16(DEBUG_INFO, "vbe width", info->width);
    debug_log16(DEBUG_INFO, "vbe height", info->height);
    debug_log8(DEBUG_INFO, "vbe bpp", info->bpp);
    debug_log8(DEBUG_INFO, "vbe framebuffer", info->framebuffer);

    ctrl_setvideosettings(&videointerface.settings, info->width, info->height, info->bpp);
    fb = info->framebuffer;

    memory_copy((void *)0x8000, (void *)(unsigned int)_set_video_mode, 0x1000);
    memory_copy((void *)0x9000, &realmode_gdt, 0x1000);
    setmode(0);
    debug_logs(DEBUG_INFO, "vbe worked!");

    for (mode_offset = 0; getmode(mode_offset); mode_offset += 2)
    {


    }

    video_notifymode(&videointerface, videointerface.settings.w, videointerface.settings.h, videointerface.settings.bpp);

}

static unsigned int videointerface_writectrl(struct system_node *self, struct system_node *current, struct service_state *state, void *buffer, unsigned int count, unsigned int offset)
{

    run();

    return count;

}

static unsigned int videointerface_writedata(struct system_node *self, struct system_node *current, struct service_state *state, void *buffer, unsigned int count, unsigned int offset)
{

    return memory_write((void *)fb, videointerface.settings.w * videointerface.settings.h * videointerface.settings.bpp, buffer, count, offset);

}

static unsigned int videointerface_readcolormap(struct system_node *self, struct system_node *current, struct service_state *state, void *buffer, unsigned int count, unsigned int offset)
{

    return 0;

}

static unsigned int videointerface_writecolormap(struct system_node *self, struct system_node *current, struct service_state *state, void *buffer, unsigned int count, unsigned int offset)
{

    return count;

}

static void driver_init(unsigned int id)
{

    video_initinterface(&videointerface, id);
    ctrl_setvideosettings(&videointerface.settings, 80, 25, 2);

    videointerface.ctrl.operations.write = videointerface_writectrl;
    videointerface.data.operations.write = videointerface_writedata;
    videointerface.colormap.operations.read = videointerface_readcolormap;
    videointerface.colormap.operations.write = videointerface_writecolormap;

}

static unsigned int driver_match(unsigned int id)
{

    return pci_inb(id, PCI_CONFIG_CLASS) == PCI_CLASS_DISPLAY && pci_inb(id, PCI_CONFIG_SUBCLASS) == PCI_CLASS_DISPLAY_VGA && pci_inb(id, PCI_CONFIG_INTERFACE) == 0x00;

}

static void driver_reset(unsigned int id)
{

}

static void driver_attach(unsigned int id)
{

    video_registerinterface(&videointerface);

}

static void driver_detach(unsigned int id)
{

    video_unregisterinterface(&videointerface);

}

void module_init(void)
{

    base_initdriver(&driver, "vbe", driver_init, driver_match, driver_reset, driver_attach, driver_detach);
    run();

}

void module_register(void)
{

    base_registerdriver(&driver, PCI_BUS);

}

void module_unregister(void)
{

    base_unregisterdriver(&driver, PCI_BUS);

}
