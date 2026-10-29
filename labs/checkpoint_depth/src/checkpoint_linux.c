#include "checkpoint_linux.h"

void cpl_init(struct cpl_reader *r, unsigned s, unsigned l)
{
    (void)r;
    (void)s;
    (void)l;
}

int cpl_open(struct cpl_reader *r, int e, int f, int t)
{
    (void)r;
    (void)e;
    (void)f;
    (void)t;
    return -1;
}

uint64_t cpl_token(const struct cpl_reader *r)
{
    (void)r;
    return 0;
}

enum cpl_status cpl_pump(struct cpl_reader *r, uint64_t t, struct cpa_queue *q)
{
    (void)r;
    (void)t;
    (void)q;
    return CPL_ERROR;
}

int cpl_close(struct cpl_reader *r)
{
    (void)r;
    return -1;
}
