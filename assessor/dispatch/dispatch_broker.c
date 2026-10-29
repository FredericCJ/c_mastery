#include "dispatch_broker.h"
#ifndef DS_BROKER_MUTANT
#define DS_BROKER_MUTANT 0
#endif
uint32_t ds_broker_u32(const unsigned char p[4])
{
    return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) |
           ((uint32_t)p[2] << 8) | p[3];
}

static void put32(unsigned char *p, uint32_t v)
{
    for (unsigned i = 0; i < 4; ++i)
        p[i] = (unsigned char)(v >> (24 - 8 * i));
}

void ds_broker_request(unsigned char p[16], unsigned op, unsigned target,
                       unsigned rights, uint32_t id, unsigned seq,
                       unsigned epoch, uint32_t value)
{
    for (unsigned i = 0; i < 16; ++i)
        p[i] = 0;
    p[0] = 1;
    p[1] = (unsigned char)op;
    p[2] = (unsigned char)target;
    p[3] = (unsigned char)rights;
    put32(p + 4, id);
    p[8] = (unsigned char)(seq >> 8);
    p[9] = (unsigned char)seq;
    p[10] = (unsigned char)epoch;
    put32(p + 12, value);
}

void ds_broker_init(struct ds_broker *b)
{
    *b = (struct ds_broker){
        .counters = {0, 100},
        .epoch = {1, 1},
        .next_sequence = {1, 1},
        .grants = {{0, 0, 3, 1, 1}, {0, 0, 1, 1, 1}, {1, 1, 3, 1, 1}, {0}}};
}

int ds_broker_revoke(struct ds_broker *b, unsigned target)
{
    if (!b || target >= 2)
        return DS_INVALID;
    if (b->epoch[target] >= 4)
        return DS_FULL;
    ++b->epoch[target];
    return DS_OK;
}

int ds_broker_apply(struct ds_broker *b, unsigned holder,
                    const unsigned char p[16], unsigned char out[16])
{
    if (!b || holder >= 2 || !p || !out)
        return DS_INVALID;
    unsigned op = p[1], target = p[2], rights = p[3],
             seq = ((unsigned)p[8] << 8) | p[9], epoch = p[10];
    uint32_t id = ds_broker_u32(p + 4), value = ds_broker_u32(p + 12),
             result = 0, returned_id = id;
    unsigned granted = 0;
    int status = DS_OK;
    if (p[0] != 1 || op < 1 || op > 4 || target >= 2 || rights > 3 || p[11] ||
        (op != DS_B_DERIVE && rights) ||
        ((op == DS_B_READ || op == DS_B_DERIVE) && value))
        status = DS_INVALID;
    else if (b->next_sequence[holder] > 65535 ||
             seq != b->next_sequence[holder])
        status = DS_STALE;
    else
    {
        ++b->next_sequence[holder];
        struct ds_broker_grant *g =
            id >= 1 && id <= 4 ? &b->grants[id - 1] : NULL;
        if (!g || !g->live || g->holder != holder)
            status = DS_DENIED;
        else if (g->target != target &&
                 !(DS_BROKER_MUTANT == 2 && op == DS_B_DEPUTY))
            status = DS_DENIED;
        else if (b->epoch[target] >= 4 || g->epoch != b->epoch[g->target] ||
                 epoch != g->epoch)
            status = DS_STALE;
        else if (op == DS_B_DERIVE)
        {
            if (rights & ~g->rights)
                status = DS_DENIED;
            else if (b->grants[3].live)
                status = DS_FULL;
            else
            {
                b->grants[3] =
                    (struct ds_broker_grant){holder, target, rights, epoch, 1};
                returned_id = 4;
                granted = rights;
            }
        }
        else
        {
            unsigned required = op == DS_B_READ ? 1 : 2;
            if (!(g->rights & required) &&
                !(DS_BROKER_MUTANT == 1 && op == DS_B_ADD))
                status = DS_DENIED;
            else if (op != DS_B_READ && value > 1000 - b->counters[target])
                status = DS_RANGE;
            else
            {
                if (op != DS_B_READ)
                    b->counters[target] += value;
                result = b->counters[target];
                granted = g->rights;
            }
        }
    }
    unsigned char reply[16] = {0};
    reply[0] = 1;
    reply[1] = (unsigned char)status;
    reply[2] = (unsigned char)target;
    reply[3] = (unsigned char)granted;
    put32(reply + 4, returned_id);
    reply[8] = (unsigned char)(seq >> 8);
    reply[9] = (unsigned char)seq;
    reply[10] = (unsigned char)(target < 2 ? b->epoch[target] : 0);
    put32(reply + 12, result);
    for (unsigned i = 0; i < 16; ++i)
        out[i] = reply[i];
    return DS_OK;
}
