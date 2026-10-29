#include "launch.h"
#include <errno.h>

int plx_launch(const char *p, char *const a[], struct plx_options o,
               struct plx_result *r)
{
    (void)p;
    (void)a;
    (void)o;
    (void)r;
    return ENOSYS;
}

int plx_redirect(const int s[2], const int d[2], unsigned f)
{
    (void)s;
    (void)d;
    (void)f;
    return ENOSYS;
}

int plx_tree(int w, int r, int a, int p)
{
    (void)w;
    (void)r;
    (void)a;
    (void)p;
    return ENOSYS;
}

int plx_read_all(int f, void *p, size_t n, size_t *u, unsigned *i)
{
    (void)f;
    (void)p;
    (void)n;
    (void)u;
    (void)i;
    return ENOSYS;
}

int plx_write_all(int f, const void *p, size_t n, size_t *u, unsigned *i)
{
    (void)f;
    (void)p;
    (void)n;
    (void)u;
    (void)i;
    return ENOSYS;
}

int plx_wait(pid_t p, int *s, unsigned *i)
{
    (void)p;
    (void)s;
    (void)i;
    return ENOSYS;
}
