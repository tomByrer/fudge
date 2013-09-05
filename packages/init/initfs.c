#include <fudge.h>

static struct args
{

    char *position;
    unsigned int count;

} args[32];

static unsigned int parse(unsigned int count, void *buffer)
{

    char *b = buffer;
    char *begin = buffer;
    unsigned int nargs = 0;
    unsigned int i;

    for (i = 0; i < count; i++)
    {

        if (b[i] != '\n' && b[i] != ' ')
            continue;

        b[i] = '\0';
        args[nargs].position = begin;
        args[nargs].count = b + i - begin;
        begin = b + i + 1;
        nargs++;

    }

    return nargs;

}

void main()
{

    char buffer[FUDGE_BSIZE];
    unsigned int count = call_read(CALL_DI, 0, FUDGE_BSIZE, buffer);
    unsigned int nargs = parse(count, buffer);
    unsigned int i;

    for (i = 0; i < nargs; i += 3)
    {

        unsigned int num = string_number(args[i].position, 10);

        call_open(CALL_DO, CALL_DR, args[i + 1].count - 1, args[i + 1].position + 1);
        call_open(CALL_DI, CALL_DR, args[i + 2].count - 1, args[i + 2].position + 1);
        call_mount(num, CALL_DO, CALL_DI);

    }

}

