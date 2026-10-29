/* Intentionally unsolved learner scaffold; reference implementations are withheld. */
#include "startup.h"

enum sd_status sd_stack_decode(const unsigned char *p, size_t n, unsigned w,
                               struct sd_stack *out)
{
    (void)p;
    (void)n;
    (void)w;
    (void)out;
    return SD_UNSUPPORTED;
}

enum sd_status sd_string_lookup(uint64_t a, const struct sd_mapping *m,
                                size_t n, const unsigned char **p, size_t *z)
{
    (void)a;
    (void)m;
    (void)n;
    (void)p;
    (void)z;
    return SD_UNSUPPORTED;
}

enum sd_status sd_parse_u32(const unsigned char *p, size_t n, uint32_t *v)
{
    (void)p;
    (void)n;
    (void)v;
    return SD_UNSUPPORTED;
}

enum sd_status sd_parse_i32(const unsigned char *p, size_t n, int32_t *v)
{
    (void)p;
    (void)n;
    (void)v;
    return SD_UNSUPPORTED;
}

enum sd_status sd_format_u32(uint32_t v, unsigned char *p, size_t n, int t,
                             size_t *z)
{
    (void)v;
    (void)p;
    (void)n;
    (void)t;
    (void)z;
    return SD_UNSUPPORTED;
}

enum sd_status sd_format_u32_nodiv(uint32_t v, unsigned char *p, size_t n,
                                   int t, size_t *z)
{
    return sd_format_u32(v, p, n, t, z);
}

enum sd_status sd_format_i32(int32_t v, unsigned char *p, size_t n, int t,
                             size_t *z)
{
    (void)v;
    (void)p;
    (void)n;
    (void)t;
    (void)z;
    return SD_UNSUPPORTED;
}

struct sd_result sd_linux_result(uint64_t v)
{
    struct sd_result r = {1, 0, 1};
    (void)v;
    return r;
}

enum sd_status sd_transfer_init(struct sd_transfer *x, unsigned char *p,
                                size_t n, int r)
{
    (void)x;
    (void)p;
    (void)n;
    (void)r;
    return SD_UNSUPPORTED;
}

enum sd_transfer_status sd_transfer_step(struct sd_transfer *x, sd_backend b,
                                         void *c, size_t n, int cancel)
{
    (void)x;
    (void)b;
    (void)c;
    (void)n;
    (void)cancel;
    return SD_BACKEND_BROKEN;
}
