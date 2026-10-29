#include "replay_sink.h"
#ifndef RPR_MUTATION
#define RPR_MUTATION 0
#endif
int rpr_sink_apply(struct rpl_log *log, struct rpr_effect e, unsigned *out)
{
    unsigned i;
    int r;
    if (!log || !out || e.timeline != log->uuid ||
        log->schema != RPR_EFFECT_SCHEMA || e.tick < 1 || e.tick > 8 ||
        e.value < 0 || e.value > 67108350 || (e.value & 1))
    {
        return RPR_INVALID;
    }
    if (log->poisoned || log->pages->poisoned || log->pages->active)
    {
        return RPR_STATE;
    }
    if (log->memory.count > 8)
    {
        return RPR_CORRUPT;
    }
    for (i = 0; i < log->memory.count; ++i)
    {
        struct rp_event event = log->memory.events[i];
        if (log->kinds[i] != RPL_SET || event.position != i + 1 ||
            event.key != i + 1 || event.value < 0 || event.value > 67108350 ||
            (event.value & 1))
        {
            return RPR_CORRUPT;
        }
    }
    if (e.tick <= log->memory.count && RPR_MUTATION != 13)
    {
        if (RPR_MUTATION != 12 &&
            log->memory.events[e.tick - 1].value != e.value)
        {
            return RPR_CONFLICT;
        }
        *out = 0;
        return RPR_OK;
    }
    if (e.tick != log->memory.count + 1)
    {
        return RPR_STALE;
    }
    r = rpl_append(log, RPL_SET, e.tick, e.value);
    if (r)
    {
        return r == RPL_UNCERTAIN ? RPR_UNCERTAIN : RPR_IO;
    }
    *out = 1;
    return RPR_OK;
}
