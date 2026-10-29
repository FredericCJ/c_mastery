#include "oslab.h"
#include <limits.h>

int os_simulate(const os_job *jobs, size_t n, os_policy policy,
                unsigned quantum, os_schedule *out)
{
    unsigned remaining[OS_MAX_JOBS] = {0}, ticket[OS_MAX_JOBS] = {0};
    unsigned level[OS_MAX_JOBS] = {0}, used[OS_MAX_JOBS] = {0};
    unsigned pass[OS_MAX_JOBS] = {0}, next = 1, elapsed = 0;
    int current = -1, previous = -1;
    if (!out || (!jobs && n) || n > OS_MAX_JOBS || policy > OS_STRIDE ||
        policy < OS_FCFS || !quantum || quantum > OS_MAX_TICKS)
    {
        return -1;
    }
    *out = (os_schedule){0};
    for (size_t i = 0; i < OS_MAX_TICKS; ++i)
    {
        out->timeline[i] = -1;
    }
    for (size_t i = 0; i < n; ++i)
    {
        if (!jobs[i].service || jobs[i].service > OS_MAX_TICKS ||
            jobs[i].arrival >= OS_MAX_TICKS || !jobs[i].weight ||
            jobs[i].weight > 16)
        {
            return -1;
        }
        remaining[i] = jobs[i].service;
        out->first[i] = UINT_MAX;
    }
    while (out->completed < n)
    {
        if (elapsed == OS_MAX_TICKS)
        {
            return -1;
        }
        /* Simultaneous arrival order is input order; arrival precedes requeue. */
        for (size_t i = 0; i < n; ++i)
        {
            if (jobs[i].arrival == elapsed)
            {
                ticket[i] = next++;
                if (policy == OS_STRIDE)
                {
                    unsigned minimum = UINT_MAX;
                    for (size_t j = 0; j < n; ++j)
                    {
                        if (j != i && remaining[j] &&
                            jobs[j].arrival < elapsed && pass[j] < minimum)
                        {
                            minimum = pass[j];
                        }
                    }
                    pass[i] = minimum == UINT_MAX ? 0 : minimum;
                }
            }
        }
        if (current >= 0 && remaining[current] &&
            ((policy == OS_RR && used[current] >= quantum) ||
             (policy == OS_MLFQ && used[current] >= (1u << level[current]))))
        {
            ticket[current] = next++;
            used[current] = 0;
            if (policy == OS_MLFQ && level[current] < 2)
            {
                ++level[current];
            }
            current = -1;
        }
        if (policy == OS_MLFQ && elapsed && elapsed % 16u == 0)
        {
            for (size_t i = 0; i < n; ++i)
            {
                level[i] = 0;
                used[i] = 0;
            }
            current = -1;
        }
        int best = -1;
        if (current >= 0 && remaining[current] &&
            (policy == OS_FCFS || policy == OS_SJF || policy == OS_RR))
        {
            best = current;
        }
        else
        {
            for (size_t i = 0; i < n; ++i)
            {
                if (remaining[i] && jobs[i].arrival <= elapsed)
                {
                    if (best < 0)
                    {
                        best = (int)i;
                        continue;
                    }
                    unsigned a = ticket[i], b = ticket[best];
                    if (policy == OS_SJF)
                    {
                        a = jobs[i].service;
                        b = jobs[best].service;
                    }
                    if (policy == OS_SRTF)
                    {
                        a = remaining[i];
                        b = remaining[best];
                    }
                    if (policy == OS_MLFQ)
                    {
                        a = level[i];
                        b = level[best];
                    }
                    if (policy == OS_STRIDE)
                    {
                        a = pass[i];
                        b = pass[best];
                    }
                    if (a < b || (a == b && ticket[i] < ticket[best]))
                    {
                        best = (int)i;
                    }
                }
            }
        }
        if (policy == OS_MLFQ && current >= 0 && remaining[current] &&
            best >= 0 && level[current] == level[best])
        {
            best = current;
        }
        out->timeline[elapsed] = best;
        if (best >= 0)
        {
            if (best != previous)
            {
                ++out->dispatches;
            }
            if (out->first[best] == UINT_MAX)
            {
                out->first[best] = elapsed;
            }
            --remaining[best];
            ++used[best];
            if (policy == OS_STRIDE)
            {
                pass[best] += 1024u / jobs[best].weight;
            }
            if (!remaining[best])
            {
                out->finish[best] = elapsed + 1;
                out->waiting[best] =
                    out->finish[best] - jobs[best].arrival - jobs[best].service;
                ++out->completed;
            }
        }
        current = best;
        previous = best;
        ++elapsed;
    }
    out->ticks = elapsed;
    return 0;
}

size_t os_clock_faults(const uint8_t *refs, size_t n, size_t frames)
{
    uint8_t pages[16] = {0};
    unsigned char live[16] = {0}, referenced[16] = {0};
    size_t hand = 0, faults = 0;
    if ((!refs && n) || !frames || frames > 16)
    {
        return SIZE_MAX;
    }
    for (size_t i = 0; i < n; ++i)
    {
        size_t found = frames;
        for (size_t j = 0; j < frames; ++j)
        {
            if (live[j] && pages[j] == refs[i])
            {
                found = j;
                break;
            }
        }
        if (found < frames)
        {
            referenced[found] = 1;
            continue;
        }
        ++faults;
        while (live[hand] && referenced[hand])
        {
            referenced[hand] = 0;
            hand = (hand + 1) % frames;
        }
        pages[hand] = refs[i];
        live[hand] = referenced[hand] = 1;
        hand = (hand + 1) % frames;
    }
    return faults;
}
