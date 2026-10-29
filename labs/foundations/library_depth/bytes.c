#include "bytes.h"

enum lb_status lb_move(unsigned char *b, size_t z, size_t d, size_t s, size_t n)
{
    (void)b;
    (void)z;
    (void)d;
    (void)s;
    (void)n;
    return LB_NOT_IMPLEMENTED;
}

enum lb_status lb_copy(unsigned char *d, size_t z, const unsigned char *s,
                       size_t q, size_t n)
{
    (void)d;
    (void)z;
    (void)s;
    (void)q;
    (void)n;
    return LB_NOT_IMPLEMENTED;
}

enum lb_status lb_strlen(const unsigned char *s, size_t z, size_t *n)
{
    (void)s;
    (void)z;
    (void)n;
    return LB_NOT_IMPLEMENTED;
}

enum lb_status lb_display(const unsigned char *s, size_t z, char *d, size_t q,
                          size_t *n)
{
    (void)s;
    (void)z;
    (void)d;
    (void)q;
    (void)n;
    return LB_NOT_IMPLEMENTED;
}

enum lb_status lb_field_display(struct f_cursor *c, char *d, size_t q,
                                size_t *n)
{
    (void)c;
    (void)d;
    (void)q;
    (void)n;
    return LB_NOT_IMPLEMENTED;
}

enum lb_status lb_record(const unsigned char *s, size_t n, unsigned char *d,
                         size_t z)
{
    (void)s;
    (void)n;
    (void)d;
    (void)z;
    return LB_NOT_IMPLEMENTED;
}
