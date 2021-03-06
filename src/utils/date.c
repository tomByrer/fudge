#include <fudge.h>
#include <abi.h>

static void print(struct channel *channel, unsigned int source, struct ctrl_clocksettings *settings)
{

    channel_request(channel, EVENT_DATA);
    channel_appendvalue(channel, settings->year, 10, 4);
    channel_appendstring(channel, "-");
    channel_appendvalue(channel, settings->month, 10, 2);
    channel_appendstring(channel, "-");
    channel_appendvalue(channel, settings->day, 10, 2);
    channel_appendstring(channel, " ");
    channel_appendvalue(channel, settings->hours, 10, 2);
    channel_appendstring(channel, ":");
    channel_appendvalue(channel, settings->minutes, 10, 2);
    channel_appendstring(channel, ":");
    channel_appendvalue(channel, settings->seconds, 10, 2);
    channel_appendstring(channel, "\n");
    channel_place(channel, source);

}

static void onmain(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    if (file_walk2(FILE_L0, "/system/clock/if:0/ctrl"))
    {

        struct ctrl_clocksettings settings;

        file_open(FILE_L0);
        file_readall(FILE_L0, &settings, sizeof (struct ctrl_clocksettings));
        file_close(FILE_L0);
        print(channel, source, &settings);

    }

    channel_close(channel);

}

static void onmain2(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    channel_close(channel);

}

static void onfile(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    if (file_walk2(FILE_L0, mdata))
    {

        struct ctrl_clocksettings settings;

        file_open(FILE_L0);
        file_readall(FILE_L0, &settings, sizeof (struct ctrl_clocksettings));
        file_close(FILE_L0);
        print(channel, source, &settings);

    }

    channel_setsignal(channel, EVENT_MAIN, onmain2);

}

static void onredirect(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    struct event_redirect *redirect = mdata;

    channel_setredirect(channel, redirect->type, redirect->mode, redirect->id, source);

}

void main(void)
{

    struct channel channel;

    channel_init(&channel);
    channel_setsignal(&channel, EVENT_MAIN, onmain);
    channel_setsignal(&channel, EVENT_FILE, onfile);
    channel_setsignal(&channel, EVENT_REDIRECT, onredirect);
    channel_listen(&channel);

}

