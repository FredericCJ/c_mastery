#include "partition.h"

enum rp_status rp_fit(const struct rp_task *a, unsigned b, unsigned c,
                      uint32_t d, unsigned e, struct rp_plan *f)
{
    (void)a;
    (void)b;
    (void)c;
    (void)d;
    (void)e;
    (void)f;
    return RP_UNIMPLEMENTED;
}

enum rp_status rp_exact(const struct rp_task *a, unsigned b, unsigned c,
                        uint32_t d, unsigned e, struct rp_plan *f)
{
    (void)a;
    (void)b;
    (void)c;
    (void)d;
    (void)e;
    (void)f;
    return RP_UNIMPLEMENTED;
}

enum rp_status rp_verify(const struct rp_task *a, unsigned b,
                         const struct rp_plan *c, unsigned d)
{
    (void)a;
    (void)b;
    (void)c;
    (void)d;
    return RP_UNIMPLEMENTED;
}

enum rp_status rp_simulate(const struct rp_task *a, unsigned b,
                           const struct rp_plan *c, cmx_sim_result *d)
{
    (void)a;
    (void)b;
    (void)c;
    (void)d;
    return RP_UNIMPLEMENTED;
}

enum rp_status rp_lease_init(struct rp_lease *a, unsigned b)
{
    (void)a;
    (void)b;
    return RP_UNIMPLEMENTED;
}

enum rp_status rp_begin(struct rp_lease *a, unsigned b, uint64_t c)
{
    (void)a;
    (void)b;
    (void)c;
    return RP_UNIMPLEMENTED;
}

enum rp_status rp_finish(struct rp_lease *a, unsigned b, uint64_t c, uint32_t d)
{
    (void)a;
    (void)b;
    (void)c;
    (void)d;
    return RP_UNIMPLEMENTED;
}

enum rp_status rp_prepare(struct rp_lease *a, unsigned b, uint64_t c,
                          unsigned d)
{
    (void)a;
    (void)b;
    (void)c;
    (void)d;
    return RP_UNIMPLEMENTED;
}

enum rp_status rp_accept(struct rp_lease *a, unsigned b, uint64_t c)
{
    (void)a;
    (void)b;
    (void)c;
    return RP_UNIMPLEMENTED;
}

enum rp_status rp_abort(struct rp_lease *a, unsigned b, uint64_t c)
{
    (void)a;
    (void)b;
    (void)c;
    return RP_UNIMPLEMENTED;
}
