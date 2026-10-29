#include "linux_port.h"

int mc_linux_init(struct mc_linux_port *p, struct mc_read *r, int f)
{
    (void)p;
    (void)r;
    (void)f;
    return 0;
}

struct co_async_port mc_linux_interface(struct mc_linux_port *p)
{
    (void)p;
    return (struct co_async_port){0};
}

int mc_linux_wait(struct mc_linux_port *p, unsigned t)
{
    (void)p;
    (void)t;
    return 0;
}

int mc_linux_dispatch(struct mc_linux_port *p)
{
    (void)p;
    return 0;
}

int mc_linux_request_cancel(struct mc_linux_port *p)
{
    (void)p;
    return 0;
}

enum msg_status mc_linux_finish_cancel(struct mc_linux_port *p)
{
    (void)p;
    return MSG_FAILED;
}

int mc_linux_quiescent(struct mc_linux_port *p)
{
    (void)p;
    return 0;
}

int mc_linux_destroy(struct mc_linux_port *p)
{
    (void)p;
    return 0;
}
