#include "alternatives.h"
#ifndef PA_MUTANT
#define PA_MUTANT 0
#endif
int pa_address(struct pa_empty b, unsigned v, struct pa_address *out)
{
    if (!out)
        return PT_INVALID;
    int r = pt_builder_set(&b.value, 0, v);
    if (!r)
        *out = (struct pa_address){b.value};
    return r;
}

int pa_length(struct pa_address b, unsigned v, struct pa_length *out)
{
    if (!out)
        return PT_INVALID;
    int r = pt_builder_set(&b.value, 1, v);
    if (!r)
        *out = (struct pa_length){b.value};
    return r;
}

int pa_timeout(struct pa_length b, unsigned v, struct pa_timeout *out)
{
    if (!out)
        return PT_INVALID;
    int r = pt_builder_set(&b.value, 2, v);
    if (!r)
        *out = (struct pa_timeout){b.value};
    return r;
}

int pa_finish(struct pa_timeout b, struct pt_config *out)
{
    if (PA_MUTANT == 1)
        b.value.mask = 0;
    return pt_builder_finish(&b.value, out);
}

int pa_thread_sum(struct pt_thread *p, unsigned available, uint32_t value,
                  uint32_t *out)
{
    if (!p || !out || p->count > 3 ||
        (p->pc != 0 && p->pc != 1 && p->pc != UINT32_MAX))
        return PT_INVALID;
    if (p->pc == UINT32_MAX)
        return PT_DONE;
    unsigned previous = p->pc;
    PT_BEGIN(p);
    while (p->count < 3)
    {
        PT_WAIT(p, 1, available);
        if (value > UINT32_MAX - p->sum)
        {
            p->pc = previous;
            return PT_RANGE;
        }
        p->sum += value;
        p->count++;
        if (p->count < 3)
        {
            p->pc = 0;
            return PT_PENDING;
        }
    }
    *out = PA_MUTANT == 2 ? 0 : p->sum;
    PT_END(p);
}

int pa_session_pair(unsigned count, unsigned *messages)
{
    if (!messages || count > 8)
        return PT_INVALID;
    struct pt_session a = {0, 0, count}, b = {1, 0, count};
    unsigned total = 0;
    const unsigned fixed[] = {PT_HELLO, PT_ACCEPT};
    for (unsigned i = 0; i < 2; i++)
    {
        unsigned direction = i == 0;
        if (pt_session_step(&a, direction, fixed[i]) ||
            pt_session_step(&b, !direction, fixed[i]))
            return PT_STATE;
        total++;
    }
    for (unsigned i = 0; i < count; i++)
    {
        if (pt_session_step(&a, 1, PT_DATA) ||
            pt_session_step(&b, 0, PT_DATA) || pt_session_step(&a, 0, PT_ACK) ||
            pt_session_step(&b, 1, PT_ACK))
            return PT_STATE;
        total += 2;
    }
    if (pt_session_step(&a, 1, PT_BYE) || pt_session_step(&b, 0, PT_BYE))
        return PT_STATE;
    if (pt_session_step(&a, 1, PT_BYE) != PT_STATE ||
        pt_session_step(&b, 0, PT_BYE) != PT_STATE)
        return PT_STATE;
    *messages = total + 1;
    return PT_OK;
}
