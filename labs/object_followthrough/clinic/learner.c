#include "clinic.h"

int mc_acquire(struct mc_owner *a, struct mc_allocator b, int c)
{
    (void)a;
    (void)b;
    (void)c;
    return -1;
}

int mc_read(const struct mc_owner *a, int *b)
{
    (void)a;
    (void)b;
    return -1;
}

int mc_snapshot(const struct mc_owner *a, int *b)
{
    (void)a;
    (void)b;
    return -1;
}

int mc_release(struct mc_owner *a)
{
    (void)a;
    return -1;
}

enum fd_status mc_move(unsigned char *a, size_t b, size_t c, size_t d, size_t e)
{
    (void)a;
    (void)b;
    (void)c;
    (void)d;
    (void)e;
    return FD_INVALID;
}

int mc_member(const struct mc_container *a, const int *b, int *c)
{
    (void)a;
    (void)b;
    (void)c;
    return -1;
}

int mc_initial(const struct mc_first *a, const int *b, int *c)
{
    (void)a;
    (void)b;
    (void)c;
    return -1;
}
