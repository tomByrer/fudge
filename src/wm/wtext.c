#include <abi.h>
#include <fudge.h>
#include "box.h"
#include "element.h"

static void (*renderers[16])(struct element *element, void *data);

static void printname(char *name)
{

    call_write(CALL_PO, ascii_length(name), name);
    call_write(CALL_PO, 2, ": ");

}

static void printvalue(char *name, unsigned int value)
{

    char num[32];

    printname(name);
    call_write(CALL_PO, ascii_wvalue(num, 32, value, 10, 0), num);

}

static void printstring(char *name, char *value)
{

    printname(name);
    call_write(CALL_PO, 1, "'");
    call_write(CALL_PO, ascii_length(value), value);
    call_write(CALL_PO, 1, "'");

}

static void printdata(char *name, unsigned int count, void *data)
{

    printname(name);
    call_write(CALL_PO, 1, "'");
    call_write(CALL_PO, count, data);
    call_write(CALL_PO, 1, "'");

}

static void printseperator(void)
{

    call_write(CALL_PO, 2, ", ");

}

static void printblockstart(void)
{

    call_write(CALL_PO, 1, "{");

}

static void printblockend(void)
{

    call_write(CALL_PO, 1, "}");

}

static void printnewline(void)
{

    call_write(CALL_PO, 1, "\n");

}

static void printbox(struct box *box)
{

    printblockstart();
    printvalue("x", box->x);
    printseperator();
    printvalue("y", box->y);
    printseperator();
    printvalue("w", box->w);
    printseperator();
    printvalue("h", box->h);
    printblockend();

}

static void printmouse(struct element *element, void *data)
{

    struct element_mouse *mouse = data;

    printblockstart();
    printvalue("id", element->id);
    printseperator();
    printstring("type", "mouse");
    printseperator();
    printvalue("source", element->source);
    printseperator();
    printvalue("count", element->count);
    printseperator();
    printname("size");
    printbox(&mouse->size);
    printblockend();

}

static void printpanel(struct element *element, void *data)
{

    struct element_panel *panel = data;

    printblockstart();
    printvalue("id", element->id);
    printseperator();
    printstring("type", "panel");
    printseperator();
    printvalue("source", element->source);
    printseperator();
    printvalue("count", element->count);
    printseperator();
    printname("size");
    printbox(&panel->size);
    printseperator();
    printvalue("active", panel->active);
    printblockend();

}

static void printtext(struct element *element, void *data)
{

    struct element_text *text = data;

    printblockstart();
    printvalue("id", element->id);
    printseperator();
    printstring("type", "text");
    printseperator();
    printvalue("source", element->source);
    printseperator();
    printvalue("count", element->count);
    printseperator();
    printname("size");
    printbox(&text->size);
    printseperator();
    printvalue("type", text->type);
    printseperator();
    printdata("string", element->count - sizeof (struct element_text), text + 1);
    printblockend();

}

static void printwindow(struct element *element, void *data)
{

    struct element_window *window = data;

    printblockstart();
    printvalue("id", element->id);
    printseperator();
    printstring("type", "window");
    printseperator();
    printvalue("source", element->source);
    printseperator();
    printvalue("count", element->count);
    printseperator();
    printname("size");
    printbox(&window->size);
    printseperator();
    printvalue("active", window->active);
    printblockend();

}

static struct element *nextelement(unsigned int count, void *data, struct element *element)
{

    if (element == 0)
        element = data;
    else
        element = (struct element *)((unsigned char *)(element + 1) + element->count);

    if ((unsigned int)element >= (unsigned int)data + count)
        return 0;

    return element;

}

void main(void)
{

    unsigned char buffer[FUDGE_BSIZE];
    unsigned int count;

    renderers[ELEMENT_TYPE_MOUSE] = printmouse;
    renderers[ELEMENT_TYPE_PANEL] = printpanel;
    renderers[ELEMENT_TYPE_TEXT] = printtext;
    renderers[ELEMENT_TYPE_WINDOW] = printwindow;

    call_open(CALL_PI);
    call_open(CALL_PO);

    while ((count = call_read(CALL_PI, FUDGE_BSIZE, buffer)))
    {

        struct element *element;

        while ((element = nextelement(count, buffer, element)))
        {

            renderers[element->type](element, element + 1);
            printnewline();

        }

    }

    call_close(CALL_PO);
    call_close(CALL_PI);

}
