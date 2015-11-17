#include <abi.h>
#include <fudge.h>
#include <lib/file.h>

void main(void)
{

    unsigned char date[FUDGE_BSIZE];
    unsigned char time[FUDGE_BSIZE];

    if (!call_walk(CALL_L0, CALL_PR, 21, "system/clock/rtc/date"))
        return;

    if (!call_walk(CALL_L1, CALL_PR, 21, "system/clock/rtc/time"))
        return;

    call_open(CALL_PO);
    call_open(CALL_L0);
    file_writeall(CALL_PO, date, file_read(CALL_L0, date, FUDGE_BSIZE));
    file_writeall(CALL_PO, " ", 1);
    call_close(CALL_L0);
    call_open(CALL_L1);
    file_writeall(CALL_PO, time, file_read(CALL_L1, time, FUDGE_BSIZE));
    file_writeall(CALL_PO, "\n", 1);
    call_close(CALL_L1);
    call_close(CALL_PO);

}

