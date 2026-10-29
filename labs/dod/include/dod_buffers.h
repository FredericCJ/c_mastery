#ifndef CMASTERY_DOD_BUFFERS_H
#define CMASTERY_DOD_BUFFERS_H
#include "dod.h"

struct d_inline
{
    unsigned char bytes[16];
    size_t length;
};

int d_inline_append(struct d_inline *, const unsigned char *, size_t);
int d_inline_truncate(struct d_inline *, size_t);

struct d_quota
{
    unsigned char rows[4][128];
    size_t used[4], live, peak, attempts, releases, fail_at;
};

struct d_zero
{
    struct d_quota *owner;
    unsigned char *data;
    size_t length;
};

void d_zero_init(struct d_zero *, struct d_quota *);
int d_zero_append(struct d_zero *, const unsigned char *, size_t);
int d_zero_pair_append(struct d_zero *, const unsigned char *, size_t,
                       struct d_zero *, const unsigned char *, size_t);
int d_zero_move(struct d_zero *, struct d_zero *);
void d_zero_reset(struct d_zero *);
uint32_t d_swar16_signed(uint32_t, uint32_t);
#endif
