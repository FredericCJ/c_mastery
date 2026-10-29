#include "cooperative.h"
#include "superloop.h"
#ifndef CR_COOP_MUTANT
#define CR_COOP_MUTANT 0
#endif
int cr_cooperative(unsigned stop_after, unsigned max_steps, unsigned features,
                   struct cr_coop_report *out)
{
    if (!out || features > 3)
    {
        return CR_INVALID;
    }
    if (features)
    {
        return CR_UNKNOWN;
    }
    struct cr_pool pool;
    struct sc_schedule scheduler;
    struct cr_coop_report report = {0};
    unsigned state[2] = {1, 0}, produced = 0, head = 0, count = 0;
    struct cr_token queue[3];
    if (cr_pool_init(&pool, 1) != CR_OK ||
        sc_schedule_init(&scheduler, SC_ROUND_ROBIN) != SC_OK)
    {
        return CR_INVALID;
    }
    for (unsigned tick = 0; tick < max_steps; tick++)
    {
        if (tick == 1)
        {
            state[0] = 0;
            report.wakes++;
        } /* Explicit producer start gate. */
        unsigned ready = (state[0] == 0 ? 1u : 0u) | (state[1] == 0 ? 2u : 0u),
                 which;
        if (!ready)
        {
            return CR_BUSY;
        }
        if (sc_schedule_next(&scheduler, tick, ready, &which) != SC_OK)
        {
            return CR_BUSY;
        }
        report.steps++;
        if (which == 0)
        {
            if (produced == 100 || produced == stop_after)
            {
                state[0] = 2;
                if (state[1] == 1)
                {
                    state[1] = 0;
                    report.wakes++;
                }
            }
            else
            {
                unsigned char bytes[16];
                unsigned length = produced % 16 + 1;
                for (unsigned i = 0; i < length; i++)
                {
                    bytes[i] = (unsigned char)produced;
                }
                struct cr_token token;
                int status = cr_allocate(&pool, 0, bytes, length, &token);
                if (status == CR_FULL)
                {
                    state[0] = 1;
                    report.blocked++;
                }
                else
                {
                    if (status != CR_OK || count == 3 ||
                        cr_offer(&pool, token) != CR_OK)
                    {
                        return CR_BUSY;
                    }
                    queue[(head + count) % 3] = token;
                    count++;
                    produced++;
                    if (state[1] == 1 && CR_COOP_MUTANT != 1)
                    {
                        state[1] = 0;
                        report.wakes++;
                    }
                }
            }
        }
        else
        {
            if (!count)
            {
                if (state[0] == 2)
                {
                    state[1] = 2;
                }
                else
                {
                    state[1] = 1;
                    report.blocked++;
                }
            }
            else
            {
                struct cr_token token = queue[head];
                head = (head + 1) % 3;
                count--;
                unsigned char bytes[16];
                size_t length = 0;
                if (cr_claim(&pool, token) != CR_OK ||
                    cr_read(&pool, token, bytes, 16, &length) != CR_OK)
                {
                    return CR_BUSY;
                }
                for (size_t i = 0; i < length; i++)
                {
                    if (bytes[i] != (unsigned char)report.jobs)
                    {
                        return CR_BUSY;
                    }
                    report.sum += bytes[i];
                }
                report.jobs++;
                if (cr_release(&pool, token) != CR_OK)
                {
                    return CR_BUSY;
                }
                if (state[0] == 1)
                {
                    state[0] = 0;
                    report.wakes++;
                }
            }
        }
        if (state[0] == 2 && state[1] == 2)
        {
            if (count || cr_close(&pool) != CR_OK || cr_drained(&pool) != CR_OK)
            {
                return CR_BUSY;
            }
            *out = report;
            return CR_OK;
        }
    }
    return CR_BUSY;
}
