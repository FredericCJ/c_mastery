#include "os.h"
#include <limits.h>
#ifndef OF_TIE_BAD
#define OF_TIE_BAD 0
#endif
#ifndef OF_IDLE_BAD
#define OF_IDLE_BAD 0
#endif
static void admit(const os_job *jobs, size_t n, unsigned time, unsigned *seen,
                  unsigned *tickets, unsigned *next)
{
    for (;;)
    {
        size_t best = n;
        for (size_t i = 0; i < n; i++)
            if (!seen[i] && jobs[i].arrival <= time &&
                (best == n || jobs[i].arrival < jobs[best].arrival ||
                 (jobs[i].arrival == jobs[best].arrival && i < best)))
                best = i;
        if (best == n)
            return;
        seen[best] = 1;
        tickets[best] = (*next)++;
    }
}

enum of_status of_schedule(const os_job *jobs, size_t n, os_policy policy,
                           unsigned quantum, unsigned overhead,
                           struct of_schedule *out)
{
    if (!out || (!jobs && n) || n > 16 || policy > OS_RR || policy < OS_FCFS ||
        !quantum || quantum > 256 || overhead > 16)
        return OF_INPUT;
    struct of_schedule r = {0};
    for (unsigned i = 0; i < 256; i++)
        r.schedule.timeline[i] = -1;
    for (size_t i = 0; i < n; i++)
        if (!jobs[i].service || jobs[i].service > 256 ||
            jobs[i].arrival >= 256 || !jobs[i].weight || jobs[i].weight > 16)
            return OF_INPUT;
    if (!overhead)
    {
        if (os_simulate(jobs, n, policy, quantum, &r.schedule))
            return OF_LIMIT;
        for (size_t i = 0; i < n; i++)
            r.queued[i] = r.schedule.waiting[i];
        *out = r;
        return OF_OK;
    }
    unsigned remaining[16] = {0}, seen[16] = {0}, tickets[16] = {0}, next = 1,
             time = 0, used = 0;
    int current = -1, previous = -1;
    for (size_t i = 0; i < n; i++)
    {
        remaining[i] = jobs[i].service;
        r.schedule.first[i] = UINT_MAX;
    }
    while (r.schedule.completed < n)
    {
        if (time >= 256)
            return OF_LIMIT;
        admit(jobs, n, time, seen, tickets, &next);
        if (current >= 0 &&
            (!remaining[current] || (policy == OS_RR && used >= quantum)))
        {
            if (remaining[current])
                tickets[current] = next++;
            current = -1;
            used = 0;
        }
        int best = -1;
        if (current >= 0 && policy != OS_SRTF)
            best = current;
        else
            for (size_t i = 0; i < n; i++)
                if (seen[i] && remaining[i])
                {
                    if (best < 0)
                    {
                        best = (int)i;
                        continue;
                    }
                    unsigned a = policy == OS_SJF    ? jobs[i].service
                                 : policy == OS_SRTF ? remaining[i]
                                                     : tickets[i],
                             b = policy == OS_SJF    ? jobs[best].service
                                 : policy == OS_SRTF ? remaining[best]
                                                     : tickets[best];
                    if (a < b ||
                        (a == b && (jobs[i].arrival < jobs[best].arrival ||
                                    (jobs[i].arrival == jobs[best].arrival &&
                                     (OF_TIE_BAD ? i > (size_t)best
                                                 : i < (size_t)best)))))
                        best = (int)i;
                }
        if (best < 0)
        {
            r.schedule.timeline[time++] = -1;
            previous = -1;
            current = -1;
            continue;
        }
        if (best != previous)
        {
            r.schedule.dispatches++;
            if (overhead > 256 - time)
                return OF_LIMIT;
            for (unsigned k = 0; k < overhead; k++)
                r.schedule.timeline[time++] = -2;
            r.dispatch_charge[best] += overhead;
            admit(jobs, n, time, seen, tickets, &next);
            if (time >= 256)
                return OF_LIMIT;
        }
        r.schedule.timeline[time] = best;
        if (r.schedule.first[best] == UINT_MAX)
            r.schedule.first[best] = time;
        if (best != current)
            used = 0;
        remaining[best]--;
        used++;
        time++;
        if (!remaining[best])
        {
            r.schedule.finish[best] = time;
            r.schedule.waiting[best] = time -
                                       (OF_IDLE_BAD ? 0 : jobs[best].arrival) -
                                       jobs[best].service;
            r.queued[best] = r.schedule.waiting[best] - r.dispatch_charge[best];
            r.schedule.completed++;
        }
        current = best;
        previous = best;
    }
    r.schedule.ticks = time;
    *out = r;
    return OF_OK;
}

enum of_status of_fair(unsigned stride, unsigned ticks,
                       const unsigned weights[3], uint64_t seed, unsigned scale,
                       const struct of_event *events, size_t count,
                       unsigned mutant, struct of_fair *out)
{
    if (!out || !weights || ticks > 256 || !ticks || stride > 1 || !scale ||
        scale > 65536 || count > 768 || (!events && count) || mutant > 1)
        return OF_INPUT;
    for (unsigned i = 0; i < 3; i++)
        if (!weights[i] || weights[i] > 16 || weights[i] > scale)
            return OF_INPUT;
    for (size_t i = 0; i < count; i++)
        if (events[i].time >= ticks || events[i].job >= 3 ||
            events[i].kind > OF_WEIGHT ||
            (events[i].kind == OF_WEIGHT &&
             (!events[i].value || events[i].value > 16 ||
              events[i].value > scale)) ||
            (events[i].kind == OF_SLEEP && events[i].value > 256))
            return OF_INPUT;
    struct of_fair r = {0};
    unsigned weight[3], level[3] = {0}, used[3] = {0}, wake[3] = {0},
                        ticket[3] = {1, 2, 3}, next = 4;
    uint64_t pass[3];
    for (unsigned i = 0; i < 3; i++)
    {
        weight[i] = weights[i];
        pass[i] = seed;
    }
    int current = -1;
    for (unsigned t = 0; t < ticks; t++)
    {
        for (unsigned i = 0; i < 3; i++)
            if (wake[i] && wake[i] == t)
            {
                ticket[i] = next++;
                uint64_t minimum = UINT64_MAX;
                for (unsigned j = 0; j < 3; j++)
                    if (j != i && wake[j] <= t && pass[j] < minimum)
                        minimum = pass[j];
                if (minimum != UINT64_MAX && pass[i] < minimum)
                    pass[i] = minimum;
            }
        if (!stride && t && t % 16 == 0)
        {
            for (unsigned i = 0; i < 3; i++)
            {
                level[i] = used[i] = 0;
            }
            current = -1;
            r.boosts++;
        }
        if (!stride && current >= 0 && used[current] >= (1u << level[current]))
        {
            if (level[current] < 2)
                level[current]++;
            used[current] = 0;
            ticket[current] = next++;
            current = -1;
        }
        for (size_t e = 0; e < count; e++)
            if (events[e].time == t)
            {
                unsigned i = events[e].job;
                if (events[e].kind == OF_WEIGHT)
                    weight[i] = events[e].value;
                else if (events[e].kind == OF_SLEEP)
                {
                    wake[i] = t + events[e].value;
                    ticket[i] = next++;
                    if (current == (int)i)
                        current = -1;
                }
                else if (current == (int)i)
                {
                    ticket[i] = next++;
                    if (mutant)
                        used[i] = 0;
                    current = -1;
                }
            }
        int best = -1;
        for (unsigned i = 0; i < 3; i++)
            if (wake[i] <= t)
            {
                if (best < 0 ||
                    (stride ? pass[i] < pass[best] : level[i] < level[best]) ||
                    ((stride ? pass[i] == pass[best]
                             : level[i] == level[best]) &&
                     ticket[i] < ticket[best]))
                    best = (int)i;
            }
        if (!stride && current >= 0 && wake[current] <= t && best >= 0 &&
            level[current] == level[best])
        {
            best = current;
        }
        r.timeline[t] = best;
        if (best < 0)
        {
            current = -1;
            continue;
        }
        r.level[t] = level[best];
        r.used[t] = used[best];
        r.service[best]++;
        used[best]++;
        if (stride)
        {
            uint64_t step = scale / weight[best];
            if (pass[best] > UINT64_MAX - step)
            {
                uint64_t minimum = pass[best];
                for (unsigned i = 0; i < 3; i++)
                    pass[i] = pass[i] >= minimum ? pass[i] - minimum : 0;
                r.normalizations++;
            }
            pass[best] += step;
        }
        current = best;
    }
    for (unsigned i = 0; i < 3; i++)
        r.final_pass[i] = pass[i];
    *out = r;
    return OF_OK;
}

enum of_status of_pages(const struct of_reference *refs, size_t n,
                        unsigned frames, enum of_page_policy policy,
                        struct of_pages *out)
{
    if (!out || (!refs && n) || n > 64 || !frames || frames > 8 ||
        policy > OF_OPT || policy < OF_FIFO)
        return OF_INPUT;
    for (size_t i = 0; i < n; i++)
        if (refs[i].write > 1)
            return OF_INPUT;
    struct of_pages r = {0};
    uint8_t pages[8] = {0};
    unsigned valid = 0, dirty = 0, referenced = 0, hand = 0, born[8] = {0},
             last[8] = {0};
    for (size_t i = 0; i < n; i++)
    {
        unsigned slot = frames;
        for (unsigned j = 0; j < frames; j++)
            if ((valid & (1u << j)) && pages[j] == refs[i].page)
            {
                slot = j;
                break;
            }
        struct of_page_step *s = &r.steps[i];
        s->victim = -1;
        s->hit = slot < frames;
        if (slot == frames)
        {
            r.faults++;
            for (unsigned j = 0; j < frames; j++)
                if (!(valid & (1u << j)))
                {
                    slot = j;
                    break;
                }
            if (slot == frames)
            {
                if (policy == OF_CLOCK)
                {
                    while (referenced & (1u << hand))
                    {
                        referenced &= ~(1u << hand);
                        hand = (hand + 1) % frames;
                    }
                    slot = hand;
                }
                else
                {
                    slot = 0;
                    for (unsigned j = 1; j < frames; j++)
                    {
                        if (policy == OF_FIFO  ? born[j] < born[slot]
                            : policy == OF_LRU ? last[j] < last[slot]
                                               : 0)
                            slot = j;
                    }
                    if (policy == OF_OPT)
                    {
                        size_t far = 0;
                        for (unsigned j = 0; j < frames; j++)
                        {
                            size_t next = i + 1;
                            while (next < n && refs[next].page != pages[j])
                                next++;
                            if (j == 0 || next > far)
                            {
                                far = next;
                                slot = j;
                            }
                        }
                    }
                }
                s->victim = pages[slot];
                if (dirty & (1u << slot))
                    r.writebacks++;
            }
            pages[slot] = refs[i].page;
            valid |= 1u << slot;
            dirty &= ~(1u << slot);
            born[slot] = (unsigned)i;
            hand = (slot + 1) % frames;
        }
        if (refs[i].write)
            dirty |= 1u << slot;
        referenced |= 1u << slot;
        last[slot] = (unsigned)i;
        s->live_mask = valid;
        s->dirty_mask = dirty;
        s->hand = hand;
        for (unsigned j = 0; j < frames; j++)
            s->resident[j] = pages[j];
    }
    *out = r;
    return OF_OK;
}

enum of_status of_progress(struct of_transfer *t, ptrdiff_t raw,
                           unsigned record)
{
    if (!t || t->done > t->wanted || record > 1 || raw < -4095)
        return OF_INPUT;
    if (t->terminal)
        return OF_ERROR;
    if (raw > 0)
    {
        if ((size_t)raw > t->wanted - t->done)
            return OF_INPUT;
        t->done += (size_t)raw;
        if (record && t->done != t->wanted)
        {
            t->terminal = 1;
            t->error = 71;
            return OF_ERROR;
        }
        if (t->done == t->wanted)
        {
            t->terminal = 1;
            return OF_DONE;
        }
        return OF_OK;
    }
    if (raw == -4 || raw == -11 || raw == 0)
        return OF_WAIT;
    if (raw < 0)
    {
        t->terminal = 1;
        t->error = (int)-raw;
        return OF_ERROR;
    }
    return OF_INPUT;
}
