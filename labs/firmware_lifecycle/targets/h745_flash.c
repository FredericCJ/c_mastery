#include "h745_flash.h"

enum fh_result fh_start(struct fh_session *s, struct fh_io io, uint32_t a,
                        const uint32_t w[8])
{
    (void)s;
    (void)io;
    (void)a;
    (void)w;
    return FH_REJECT;
}

enum fh_result fh_poll(struct fh_session *s)
{
    (void)s;
    return FH_REJECT;
}

int fh_record(const struct fw_peer *p, uint32_t n, const unsigned char h[64],
              const uint32_t v[2], const uint32_t ver[2],
              unsigned char out[128])
{
    (void)p;
    (void)n;
    (void)h;
    (void)v;
    (void)ver;
    (void)out;
    return 0;
}

int fh_record_valid(const unsigned char in[128])
{
    (void)in;
    return 0;
}

enum fh_result fh_append_begin(struct fh_append *a, struct fh_io io,
                               uint32_t base, const struct fw_peer *p,
                               uint32_t n, const unsigned char h[64],
                               const uint32_t v[2], const uint32_t ver[2])
{
    (void)a;
    (void)io;
    (void)base;
    (void)p;
    (void)n;
    (void)h;
    (void)v;
    (void)ver;
    return FH_REJECT;
}

enum fh_result fh_append_poll(struct fh_append *a)
{
    (void)a;
    return FH_REJECT;
}
