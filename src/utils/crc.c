#include <fudge.h>
#include <abi.h>

static struct crc s;

static void ondata(struct channel *channel)
{

    crc_read(&s, channel_getdata(channel), channel_getdatasize(channel));

}

static void onfile(struct channel *channel)
{

    struct event_file *file = channel_getdata(channel);
    unsigned char buffer[FUDGE_BSIZE];
    unsigned int count;

    file_open(file->descriptor);

    while ((count = file_read(file->descriptor, buffer, FUDGE_BSIZE)))
        crc_read(&s, buffer, count);

    file_close(file->descriptor);

}

void main(void)
{

    struct channel channel;
    unsigned char buffer[FUDGE_BSIZE];
    unsigned int result;

    channel_initsignals(&channel);
    channel_setsignal(&channel, EVENT_DATA, ondata);
    channel_setsignal(&channel, EVENT_FILE, onfile);
    channel_listen(&channel);

    result = crc_finalize(&s);

    channel_reply(&channel, EVENT_DATA);
    event_append(&channel.o, ascii_wvalue(buffer, 32, result, 10), buffer);
    event_append(&channel.o, 1, "\n");
    channel_place(channel.o.header.target, &channel.o);

}

