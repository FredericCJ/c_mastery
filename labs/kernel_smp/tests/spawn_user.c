#include "cmk2_abi.h"
static volatile unsigned exit_status = 7;

void _start(void)
{
    if (cmk_call(CMK_BLOCK_COUNT, CMK_DISK_HANDLE, 0, 0) > 0)
    {
        struct cmk2_block_request r = {4, 0, 0};
        int64_t ticket;
        do
        {
            ticket =
                cmk_call(CMK2_BLOCK_SUBMIT, CMK_DISK_HANDLE, (uintptr_t)&r, 0);
            if (ticket == CMK2_EAGAIN)
                cmk_call(CMK_YIELD, 0, 0, 0);
        } while (ticket == CMK2_EAGAIN);
        if (ticket < 0)
            cmk2_thread_exit(91);
        int64_t status;
        do
        {
            status = cmk_call(CMK2_BLOCK_STATUS, (uint64_t)ticket, 0, 0);
            if (status == CMK2_EAGAIN)
                cmk_call(CMK_YIELD, 0, 0, 0);
        } while (status == CMK2_EAGAIN);
        if (status)
            cmk2_thread_exit(92);
        /* Exit deliberately without consuming BLOCK_WAIT. */
    }
    cmk2_thread_exit(exit_status);
}
