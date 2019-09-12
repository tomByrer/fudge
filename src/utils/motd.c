#include <fudge.h>
#include <abi.h>

void main(void)
{

    struct channel channel;
    unsigned int id;

    channel_init(&channel);

    if (!file_walk2(FILE_CP, "/bin/echo"))
        return;

    if (!file_walk2(FILE_C0, "/data/motd.txt"))
        return;

    id = call_spawn();

    if (!id)
        return;

    channel_listen(&channel);
    channel_forward(&channel, EVENT_FILE);
    event_addfile(&channel.o, FILE_P0);
    channel_place(id, &channel.o.header);
    channel_forward(&channel, EVENT_STOP);
    channel_place(id, &channel.o.header);

}

