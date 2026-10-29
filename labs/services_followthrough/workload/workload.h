#ifndef SF_WORKLOAD_H
#define SF_WORKLOAD_H
#include "amp.h"

struct sf_work
{
    struct af_ring ring;
    _Atomic uint32_t phase;
    uint32_t offered, admitted, rejected, completed, uncertain, checksum;
    uint8_t state[128];
};

void sfw_init(struct sf_work *);
int sfw_offer(struct sf_work *, unsigned);
int sfw_work(struct sf_work *);
int sfw_quiescent(struct sf_work *, unsigned, unsigned);
int sfw_check(const struct sf_work *);
int sfw_run(struct sf_work *, unsigned);
#endif
