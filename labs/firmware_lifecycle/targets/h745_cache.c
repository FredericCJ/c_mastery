#include "h745_cache.h"

int fw_cache_range(uint32_t a, uint32_t n, enum fw_cache_op op,
                   fw_cache_emit_fn emit, void *ctx)
{
    (void)a;
    (void)n;
    (void)op;
    (void)emit;
    (void)ctx;
    return 0;
}

enum fw_peer_result fw_peer_step(struct fw_peer *p, unsigned c, uint32_t g,
                                 enum fw_peer_op op)
{
    (void)p;
    (void)c;
    (void)g;
    (void)op;
    return H7_INVALID;
}
