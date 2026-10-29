#include "bytes.h"

enum ab_status ab_write(uint64_t a, unsigned b, uint32_t c, uint64_t *d)
{
    (void)a;
    (void)b;
    (void)c;
    (void)d;
    return AB_UNSUPPORTED;
}

enum ab_status ab_word(const unsigned char *a, size_t b, unsigned c,
                       uint32_t *d)
{
    (void)a;
    (void)b;
    (void)c;
    (void)d;
    return AB_UNSUPPORTED;
}

enum ab_status ab_address(uint64_t a, uint64_t b, unsigned c, int64_t d,
                          unsigned e, uint64_t f, uint64_t *g, uint64_t *h)
{
    (void)a;
    (void)b;
    (void)c;
    (void)d;
    (void)e;
    (void)f;
    (void)g;
    (void)h;
    return AB_UNSUPPORTED;
}

enum ab_status ab_record_decode(const unsigned char *a, size_t b, unsigned c,
                                struct aw_record *d)
{
    (void)a;
    (void)b;
    (void)c;
    (void)d;
    return AB_UNSUPPORTED;
}

enum ab_status ab_record_encode(const struct aw_record *a, unsigned b,
                                unsigned char *c, size_t d)
{
    (void)a;
    (void)b;
    (void)c;
    (void)d;
    return AB_UNSUPPORTED;
}

enum ab_status ab_record_copy(struct aw_record *a, const struct aw_record *b)
{
    (void)a;
    (void)b;
    return AB_UNSUPPORTED;
}

unsigned ab_thumb_length(uint16_t a)
{
    (void)a;
    return 0;
}

enum ab_status ab_x86(const unsigned char *a, size_t b, uint64_t c, unsigned d,
                      struct ab_instruction *e)
{
    (void)a;
    (void)b;
    (void)c;
    (void)d;
    (void)e;
    return AB_UNSUPPORTED;
}

enum ab_status ab_thumb(const unsigned char *a, size_t b, uint32_t c,
                        unsigned d, struct ab_instruction *e)
{
    (void)a;
    (void)b;
    (void)c;
    (void)d;
    (void)e;
    return AB_UNSUPPORTED;
}
