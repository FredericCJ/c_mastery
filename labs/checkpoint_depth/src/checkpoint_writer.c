#include "checkpoint_writer.h"

struct cpw_result cpw_raw(long n)
{
    (void)n;
    return (struct cpw_result){CPW_ERROR, 0, 1};
}

struct cpw_result cpw_libc(long n, int e)
{
    (void)n;
    (void)e;
    return (struct cpw_result){CPW_ERROR, 0, 1};
}

struct cpw_result cpw_script(long n)
{
    (void)n;
    return (struct cpw_result){CPW_ERROR, 0, 1};
}

enum cpw_status cpw_begin(struct cpw_transfer *t, const unsigned char *d,
                          size_t n, unsigned o)
{
    (void)t;
    (void)d;
    (void)n;
    (void)o;
    return CPW_INVALID;
}

enum cpw_status cpw_step(struct cpw_transfer *t, cpw_write_fn w, void *c)
{
    (void)t;
    (void)w;
    (void)c;
    return CPW_INVALID;
}

enum cpw_status cpw_recover(struct cpw_transfer *t)
{
    (void)t;
    return CPW_INVALID;
}

void cpw_cleanup(struct cpw_transfer *t, int c)
{
    (void)t;
    (void)c;
}
