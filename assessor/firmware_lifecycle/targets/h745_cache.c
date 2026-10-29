#include "h745_cache.h"
#include <limits.h>
#ifndef HC_MUTANT
#define HC_MUTANT 0
#endif
int fw_cache_range(uint32_t a, uint32_t n, enum fw_cache_op op,
                   fw_cache_emit_fn emit, void *ctx)
{
    if (!emit || (op != FW_CLEAN && op != H7_INVALIDATE))
    {
        return 0;
    }
    if (!n)
    {
        return 1;
    }
    if ((a & 31u) || (n & 31u) || n > 4096u || a > UINT32_MAX - (n - 1u))
    {
        return 0;
    }
    emit(ctx, FW_DSB, 0);
    for (uint32_t left = n;; left -= 32u, a += 32u)
    {
        emit(ctx, op, a);
        if (left == 32u)
        {
            break; /* No overflow even at the final physical line. */
        }
    }
    if (HC_MUTANT != 1)
    {
        emit(ctx, FW_DSB, 0);
    }
    emit(ctx, FW_ISB, 0);
    return 1;
}

enum fw_peer_result fw_peer_step(struct fw_peer *p, unsigned c, uint32_t g,
                                 enum fw_peer_op op)
{
    if (!p || c > 1u || (unsigned)op > FW_CONFIRM || !p->generation ||
        p->health > 3u || p->failed > 3u || p->confirmed > 1u ||
        p->reserved[0] || p->reserved[1])
    {
        return H7_INVALID;
    }
    if (HC_MUTANT != 2 && g != p->generation)
    {
        return H7_STALE;
    }
    switch (op)
    {
        case FW_BEGIN:
            if (p->failed)
            {
                return H7_FAILED;
            }
            if (p->confirmed || p->outstanding[c] == UINT32_MAX)
            {
                return H7_BUSY;
            }
            ++p->outstanding[c];
            p->health &= ~(1u << c);
            return H7_OK;
        case FW_END:
            if (!p->outstanding[c])
            {
                return H7_INVALID;
            }
            --p->outstanding[c];
            return H7_OK;
        case FW_HEALTHY:
            if (p->failed)
            {
                return H7_FAILED;
            }
            if (p->outstanding[c])
            {
                return H7_BUSY;
            }
            p->health |= 1u << c;
            return H7_OK;
        case FW_FAIL_PEER:
            p->failed |= 1u << (1u - c);
            p->health &= ~(1u << (1u - c));
            p->confirmed = 0;
            return H7_OK;
        case FW_CONFIRM:
            if (p->failed)
            {
                return H7_FAILED;
            }
            if (p->health != 3u ||
                (HC_MUTANT != 3 && (p->outstanding[0] || p->outstanding[1])))
            {
                return H7_BUSY;
            }
            p->confirmed = 1;
            return H7_OK;
    }
    return H7_INVALID;
}
