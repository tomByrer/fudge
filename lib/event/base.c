#include <abi.h>
#include <fudge.h>
#include "base.h"

void *event_getforward(void *buffer)
{

    struct event_header *header = buffer;

    return header + 1;

}

void *event_getdata(void *buffer)
{

    struct event_header *header = buffer;
    struct event_forward *forward = event_getforward(buffer);

    return forward + header->forward;

}

unsigned int event_addheader(void *buffer, unsigned int type, unsigned int source, unsigned int target)
{

    struct event_header *header = buffer;

    header->type = type;
    header->source = source;
    header->target = target;
    header->length = sizeof (struct event_header);
    header->forward = 0;

    return header->length;
}

unsigned int event_addforward(void *buffer, unsigned int target)
{

    struct event_header *header = buffer;
    struct event_forward *forward = event_getforward(buffer);

    forward[header->forward].target = target;
    header->forward++;
    header->length += sizeof (struct event_forward);

    return header->length;

}

unsigned int event_addreply(void *buffer, struct event_header *header, unsigned int type)
{

    struct event_header *reply = buffer;

    event_addheader(buffer, type, header->target, header->source);

    if (header->forward)
    {

        struct event_forward *forward = event_getforward(header);
        unsigned int i;

        reply->target = forward[0].target;

        for (i = 1; i < header->forward; i++)
            event_addforward(buffer, forward[i].target);

    }

    return reply->length;

}

unsigned int event_adddata(void *buffer, unsigned int count, void *data)
{

    struct event_header *header = buffer;
    struct event_data *d = event_getdata(buffer);

    header->length += sizeof (struct event_data);
    d->count = memory_write(buffer, FUDGE_BSIZE, data, count, header->length);
    header->length += d->count;

    return header->length;

}

unsigned int event_addfile(void *buffer, unsigned int num)
{

    struct event_header *header = buffer;
    struct event_file *file = event_getdata(buffer);

    header->length += sizeof (struct event_file);
    file->num = num;

    return header->length;

}

struct event_header *event_read(unsigned int descriptor, void *data)
{

    struct event_header *header = data;

    while (file_readall(descriptor, header, sizeof (struct event_header)) != sizeof (struct event_header));
    while (file_readall(descriptor, header + 1, header->length - sizeof (struct event_header)) != header->length - sizeof (struct event_header));

    return header;

}

unsigned int event_reply(unsigned int descriptor, struct event_header *header, unsigned int type)
{

    char data[FUDGE_BSIZE];
    struct event_header *reply = (struct event_header *)data;

    event_addreply(data, header, type);

    return file_writeall(descriptor, reply, reply->length);

}

unsigned int event_replydata(unsigned int descriptor, struct event_header *header, unsigned int type, unsigned int count, void *buffer)
{

    char data[FUDGE_BSIZE];
    struct event_header *reply = (struct event_header *)data;

    event_addreply(data, header, type);
    event_adddata(data, count, buffer);

    return file_writeall(descriptor, reply, reply->length);

}

unsigned int event_replyfile(unsigned int descriptor, struct event_header *header, unsigned int type, unsigned int num)
{

    char data[FUDGE_BSIZE];
    struct event_header *reply = (struct event_header *)data;

    event_addreply(data, header, type);
    event_addfile(data, num);

    return file_writeall(descriptor, reply, reply->length);

}

unsigned int event_sendbuffer(unsigned int descriptor, void *buffer)
{

    struct event_header *message = buffer;

    return file_writeall(descriptor, message, message->length);

}

unsigned int event_send(unsigned int descriptor, struct event_header *header, unsigned int type, unsigned int source, unsigned int target, unsigned int length)
{

    event_initheader(header, type, source, target, length);

    return file_writeall(descriptor, header, header->length);

}

unsigned int event_sendinit(unsigned int descriptor, unsigned int source, unsigned int target)
{

    struct {struct event_header header;} message;

    return event_send(descriptor, &message.header, EVENT_INIT, source, target, 0);

}

unsigned int event_sendexit(unsigned int descriptor, unsigned int source, unsigned int target)
{

    struct {struct event_header header;} message;

    return event_send(descriptor, &message.header, EVENT_EXIT, source, target, 0);

}

unsigned int event_sendkill(unsigned int descriptor, unsigned int source, unsigned int target)
{

    struct {struct event_header header;} message;

    return event_send(descriptor, &message.header, EVENT_KILL, source, target, 0);

}

unsigned int event_senddata(unsigned int descriptor, unsigned int source, unsigned int target, unsigned int count, void *buffer)
{

    struct {struct event_header header; struct event_data data; char buffer[0x800];} message;

    message.data.count = memory_write(message.buffer, 0x800, buffer, count, 0);

    return event_send(descriptor, &message.header, EVENT_DATA, source, target, sizeof (struct event_data) + message.data.count);

}

unsigned int event_sendfile(unsigned int descriptor, unsigned int source, unsigned int target, unsigned char num)
{

    struct {struct event_header header; struct event_file file;} message;

    message.file.num = num;

    return event_send(descriptor, &message.header, EVENT_FILE, source, target, sizeof (struct event_file));

}

unsigned int event_sendchild(unsigned int descriptor, unsigned int source, unsigned int target)
{

    struct {struct event_header header;} message;

    return event_send(descriptor, &message.header, EVENT_CHILD, source, target, 0);

}

