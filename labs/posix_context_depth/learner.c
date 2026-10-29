#include "context.h"
#include <errno.h>

int pc_launch(const char *p, char *const a[], char *const e[], int d, int f,
              struct pc_launch *o)
{
    (void)p;
    (void)a;
    (void)e;
    (void)d;
    (void)f;
    (void)o;
    return ENOSYS;
}

int pc_spawn(const char *p, char *const a[], char *const e[], int d, int f,
             struct pc_launch *o)
{
    (void)p;
    (void)a;
    (void)e;
    (void)d;
    (void)f;
    (void)o;
    return ENOSYS;
}

int pc_reap(struct pc_ticket *t, pc_wait_fn f, void *c, struct pc_launch *o)
{
    (void)t;
    (void)f;
    (void)c;
    (void)o;
    return ENOSYS;
}

int pc_replace(int d, int l, const unsigned char *b, size_t n, unsigned f,
               pc_step_fn s, void *c, struct pc_replace *o)
{
    (void)d;
    (void)l;
    (void)b;
    (void)n;
    (void)f;
    (void)s;
    (void)c;
    (void)o;
    return ENOSYS;
}

int pc_deadline(uint64_t n, uint64_t b, uint64_t *d)
{
    (void)n;
    (void)b;
    (void)d;
    return ENOSYS;
}

uint64_t pc_remaining(uint64_t d, uint64_t n)
{
    (void)d;
    (void)n;
    return 0;
}

int pc_choose(struct pc_capability *c, unsigned e, int p, int f, int i)
{
    (void)c;
    (void)e;
    (void)p;
    (void)f;
    (void)i;
    return ENOSYS;
}

int pc_operation_status(int e, size_t n)
{
    (void)e;
    (void)n;
    return 0;
}

int pc_ready(int b, int f)
{
    (void)b;
    (void)f;
    return -1;
}

int pc_request(unsigned a, unsigned r, int e, unsigned *o)
{
    (void)a;
    (void)r;
    (void)e;
    (void)o;
    return ENOSYS;
}
