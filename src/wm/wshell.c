#include <abi.h>
#include <fudge.h>
#include "box.h"
#include "element.h"
#include "send.h"
#include "keymap.h"

static struct element_text content;
static unsigned int contentrows;
static unsigned int quit;
static unsigned int keymod = KEYMOD_NONE;
static unsigned char textbuffer[FUDGE_BSIZE];
static unsigned int textcount;
static unsigned char inputbuffer[FUDGE_BSIZE];
static struct buffer input;
static struct box size;
static unsigned char databuffer[FUDGE_BSIZE];
static unsigned int datacount;
static void (*handlers[EVENTS])(struct event_header *header);

static void writeelement(unsigned int id, unsigned int type, unsigned int source, unsigned int z, unsigned int count)
{

    struct element element;

    element_init(&element, id, type, source, z, count);

    datacount += memory_write(databuffer, FUDGE_BSIZE, &element, sizeof (struct element), datacount);

}

static void writetext(unsigned int source, unsigned int z, struct element_text *text, void *buffer, unsigned int count)
{

    writeelement((unsigned int)text, ELEMENT_TYPE_TEXT, source, z, sizeof (struct element_text) + count);

    datacount += memory_write(databuffer, FUDGE_BSIZE, text, sizeof (struct element_text), datacount);
    datacount += memory_write(databuffer, FUDGE_BSIZE, buffer, count, datacount);

}

static void inserttext(void *buffer, unsigned int count)
{

    unsigned int i;
    unsigned char *b = buffer;

    for (i = 0; i < count; i++)
    {

        if (b[i] == '\n')
        {

            contentrows++;

            if (contentrows > 64)
            {

                unsigned int offset = memory_findbyte(textbuffer, textcount, '\n');

                memory_copy(textbuffer, textbuffer + offset, textcount - offset);

                textcount -= offset;

            }

        }

        textcount += memory_write(textbuffer, FUDGE_BSIZE, b + i, 1, textcount);

    }

    content.cursor = textcount - 1;

}

static void removetext(unsigned int count)
{

    unsigned int i;

    for (i = 0; i < count; i++)
    {

        if (!textcount)
            break;

        textcount -= 1;

        if (textbuffer[textcount] == '\n')
            contentrows--;

    }

    content.cursor = textcount - 1;

}

static void interpret(void)
{

    char command[FUDGE_BSIZE];
    unsigned int count = buffer_rcfifo(&input, FUDGE_BSIZE, command);

    /* This is a temporary fix */
    if (memory_match(command, "cd ", 3))
    {

        if (count < 4)
            return;

        command[count - 1] = '\0';

        if (file_walk(CALL_L1, command + 3))
        {

            file_duplicate(CALL_PW, CALL_L1);
            file_duplicate(CALL_CW, CALL_L1);

        }

        return;

    }

    if (!file_walk(CALL_CP, "/bin/slang"))
        return;

    if (!file_walk(CALL_L1, "/system/pipe/clone/"))
        return;

    file_walkfrom(CALL_L2, CALL_L1, "0");
    file_walkfrom(CALL_CI, CALL_L1, "1");
    file_walkfrom(CALL_CO, CALL_L1, "1");
    file_open(CALL_L2);
    file_writeall(CALL_L2, command, count);
    file_close(CALL_L2);
    call_spawn();
    file_open(CALL_L2);

    while ((count = file_read(CALL_L2, command, FUDGE_BSIZE)))
        inserttext(command, count);

    file_close(CALL_L2);

}

static void onkeypress(struct event_header *header)
{

    struct event_keypress keypress;
    struct keycode *keycode;

    file_readall(CALL_L0, &keypress, sizeof (struct event_keypress));

    switch (keypress.scancode)
    {

    case 0x2A:
    case 0x36:
        keymod |= KEYMOD_SHIFT;

        break;

    case 0x0E:
        if (!buffer_ecfifo(&input, 1))
            break;

        removetext(2);
        inserttext("\n", 1);
        writetext(header->destination, 1, &content, textbuffer, textcount);

        break;

    case 0x1C:
        keycode = getkeycode(KEYMAP_US, keypress.scancode, keymod);

        if (!buffer_wcfifo(&input, keycode->length, &keycode->value))
            break;

        interpret();
        inserttext("$ \n", 3);
        writetext(header->destination, 1, &content, textbuffer, textcount);

        break;

    default:
        keycode = getkeycode(KEYMAP_US, keypress.scancode, keymod);

        if (!buffer_wcfifo(&input, keycode->length, &keycode->value))
            break;

        removetext(1);
        inserttext(&keycode->value, keycode->length);
        inserttext("\n", 1);
        writetext(header->destination, 1, &content, textbuffer, textcount);

        break;

    }

}

static void onkeyrelease(struct event_header *header)
{

    struct event_keyrelease keyrelease;

    file_readall(CALL_L0, &keyrelease, sizeof (struct event_keyrelease));

    switch (keyrelease.scancode)
    {

    case 0x2A:
    case 0x36:
        keymod &= ~KEYMOD_SHIFT;

        break;

    }

}

static void onwmunmap(struct event_header *header)
{

    writetext(header->destination, 0, &content, textbuffer, textcount);

    quit = 1;

}

static void onwmresize(struct event_header *header)
{

    struct event_wmresize wmresize;

    file_readall(CALL_L0, &wmresize, sizeof (struct event_wmresize));
    box_setsize(&size, wmresize.x, wmresize.y, wmresize.w, wmresize.h);
    box_setsize(&content.size, size.x + 12, size.y + 12, size.w - 24, size.h - 24);

}

static void onwmshow(struct event_header *header)
{

    writetext(header->destination, 1, &content, textbuffer, textcount);

}

static void onwmhide(struct event_header *header)
{

    writetext(header->destination, 0, &content, textbuffer, textcount);

}

static void setup(void)
{

    buffer_init(&input, FUDGE_BSIZE, inputbuffer);
    element_inittext(&content, ELEMENT_TEXTTYPE_NORMAL, ELEMENT_TEXTFLOW_INPUT);

    inserttext("$ \n", 3);

}

void main(void)
{

    struct event_header header;
    unsigned int count;

    setup();

    handlers[EVENT_KEYPRESS] = onkeypress;
    handlers[EVENT_KEYRELEASE] = onkeyrelease;
    handlers[EVENT_WMUNMAP] = onwmunmap;
    handlers[EVENT_WMRESIZE] = onwmresize;
    handlers[EVENT_WMSHOW] = onwmshow;
    handlers[EVENT_WMHIDE] = onwmhide;

    if (!file_walk(CALL_L0, "/system/event/poll"))
        return;

    if (!file_walk(CALL_L1, "/system/event/wm"))
        return;

    file_open(CALL_PO);
    file_open(CALL_L0);
    file_open(CALL_L1);
    send_wmmap(CALL_L1, 0, 0);

    while ((count = file_readall(CALL_L0, &header, sizeof (struct event_header))))
    {

        if (!handlers[header.type])
            continue;

        handlers[header.type](&header);

        if (datacount)
        {

            file_writeall(CALL_PO, databuffer, datacount);

            datacount = 0;

        }

        if (quit)
            break;

    }

    file_close(CALL_L1);
    file_close(CALL_L0);
    file_close(CALL_PO);

}

