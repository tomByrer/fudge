#include <fudge.h>
#include <gfx/gfx.h>
#include <gfx/pcx.h>

static struct pcx_surface surface;

static void set_resolution(unsigned int count, char *resolution)
{

    call_open(3, FUDGE_ROOT, 27, "system/video/vga/resolution");
    call_write(3, 0, count, resolution);
    call_close(3);

}

static void set_colormap(unsigned int count, unsigned char *colormap)
{

    call_open(3, FUDGE_ROOT, 25, "system/video/vga/colormap");
    call_write(3, 0, count, colormap);
    call_close(3);

}

static void render(struct pcx_surface *surface)
{

    unsigned char colormap[768];
    unsigned int width = surface->header.xend - surface->header.xstart + 1;
    unsigned int height = surface->header.yend - surface->header.ystart + 1;
    unsigned int offset = 128;
    unsigned int row;

    gfx_convert_colormap(colormap, surface->colormap, 768);
    set_resolution(9, "320x200x8");
    set_colormap(768, colormap);
    call_open(3, FUDGE_ROOT, 21, "system/video/vga/data");

    for (row = 0; row < height; row++)
    {

        char buffer[FUDGE_BSIZE];

        offset += pcx_read(surface, offset, FUDGE_BSIZE, buffer);

        call_write(3, row * width, width, buffer);

    }

    call_close(3);

}

void main()
{

    pcx_init_surface(&surface, FUDGE_IN);
    pcx_load(&surface);
    render(&surface);

}

