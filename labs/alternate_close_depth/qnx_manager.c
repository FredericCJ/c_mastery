/* Original bounded QNX8 teaching resource manager. Not built without real SDK. */
#ifndef __QNXNTO__
#error QNX_SDP8_TARGET_REQUIRED
#endif
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/iofunc.h>
#include <sys/resmgr.h>
#include <sys/dispatch.h>
#include <unistd.h>
static unsigned injected;

static int close_once(resmgr_context_t *ctp, io_close_t *msg, iofunc_ocb_t *ocb)
{
    /* Synthetic disconnect rcvid0 must go through real default cleanup. */
    if (ctp->rcvid > 0 && !injected)
    {
        injected = 1;
        puts("QC_MANAGER:injected-EINTR-before-default");
        fflush(stdout);
        return EINTR;
    }
    puts("QC_MANAGER:default-close");
    fflush(stdout);
    return iofunc_close_dup_default(ctp, msg, ocb);
}

int main(void)
{
    static resmgr_connect_funcs_t connect;
    static resmgr_io_funcs_t io;
    static iofunc_attr_t attr;
    resmgr_attr_t config = {0};
    dispatch_t *dispatch;
    dispatch_context_t *context;
    char path[64];
    int n =
        snprintf(path, sizeof path, "/dev/cmastery-close-%ld", (long)getpid());
    if (n < 0 || (size_t)n >= sizeof path)
        return 1;
    dispatch = dispatch_create();
    if (!dispatch)
        return 2;
    iofunc_func_init(_RESMGR_CONNECT_NFUNCS, &connect, _RESMGR_IO_NFUNCS, &io);
    io.close_dup = close_once;
    iofunc_attr_init(&attr, S_IFNAM | 0600, NULL, NULL);
    config.nparts_max = 1;
    config.msg_max_size = 2048;
    if (resmgr_attach(dispatch, &config, path, _FTYPE_ANY, 0, &connect, &io,
                      &attr) < 0)
    {
        perror("resmgr_attach");
        return 3;
    }
    context = dispatch_context_alloc(dispatch);
    if (!context)
        return 4;
    printf("QC_MANAGER_READY:%s\n", path);
    fflush(stdout);
    /* Single dispatch thread serializes the one-shot state; isolated guest owns process. */
    for (;;)
    {
        context = dispatch_block(context);
        if (!context)
            return 5;
        dispatch_handler(context);
    }
}
