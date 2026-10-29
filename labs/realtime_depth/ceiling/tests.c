#include "ceiling.h"
#ifdef SC_TARGET
#include <stddef.h>

static int sc_target_compare(const void *a, const void *b, size_t n)
{
    const unsigned char *x = a, *y = b;
    for (size_t i = 0; i < n; i++)
        if (x[i] != y[i])
            return 1;
    return 0;
}

#define memcmp sc_target_compare
#define puts sc_target_puts
#define CHECK(x)               \
    do                         \
    {                          \
        if (!(x))              \
            sc_target_stop(1); \
    } while (0)
#else
#include <stdio.h>
#include <string.h>
#define CHECK(x)                                               \
    do                                                         \
    {                                                          \
        if (!(x))                                              \
        {                                                      \
            fprintf(stderr, "SC_CHECK:%d:%s\n", __LINE__, #x); \
            return 1;                                          \
        }                                                      \
    } while (0)
#endif
static struct sc_config config(unsigned strict)
{
    struct sc_config c = {0};
    c.tasks = 4;
    c.resources = 2;
    c.priority[0] = 2;
    c.priority[1] = 5;
    c.priority[2] = 8;
    c.priority[3] = 1;
    c.users[0] = 5;
    c.users[1] = 6;
    c.ceiling[0] = 2;
    c.ceiling[1] = 5;
    c.strict_nesting = strict;
    return c;
}

static int table(void)
{
    struct sc_config c = config(0);
    struct sc_world w;
    CHECK(sc_init(&w, &c, SC_IMMEDIATE) == CMX_OK && sc_invariant(&w));
    CHECK(sc_lock(&w, 2, 0, cmx_forever(), SC_TASK_CONTEXT) == CMX_OK &&
          w.kernel.task[2].priority == 2);
    CHECK(sc_lock(&w, 2, 1, cmx_forever(), SC_TASK_CONTEXT) == CMX_OK &&
          w.kernel.task[2].priority == 2);
    CHECK(sc_unlock(&w, 2, 0, SC_TASK_CONTEXT) == CMX_OK &&
          w.kernel.task[2].priority == 5 && sc_invariant(&w));
    CHECK(sc_unlock(&w, 2, 1, SC_TASK_CONTEXT) == CMX_OK &&
          w.kernel.task[2].priority == 8);
    CHECK(sc_lock(&w, 3, 0, cmx_forever(), SC_TASK_CONTEXT) == CMX_INVALID &&
          w.mutex[0].owner == CMX_NONE);
    CHECK(sc_lock(&w, 1, 0, cmx_forever(), SC_TASK_CONTEXT) == CMX_INVALID);
    CHECK(sc_lock(&w, 2, 0, cmx_forever(), SC_INTERRUPT_CONTEXT) ==
          CMX_INVALID);
    struct sc_world before = w;
    c.ceiling[0] = 5;
    CHECK(sc_init(&w, &c, SC_IMMEDIATE) == CMX_INVALID &&
          memcmp(&w, &before, sizeof w) == 0);
    c = config(0);
    c.users[0] |= 8;
    c.ceiling[0] = 1;
    CHECK(sc_init(&w, &c, SC_IMMEDIATE) == CMX_OK &&
          sc_lock(&w, 3, 0, cmx_forever(), SC_TASK_CONTEXT) == CMX_OK &&
          w.kernel.task[3].priority == 1);
    puts("SC_TABLE:PASS");
    return 0;
}

static int nested(void)
{
    for (unsigned mode = 0; mode < 2; mode++)
    {
        struct sc_config c = config(1);
        struct sc_world w;
        CHECK(sc_init(&w, &c, (enum sc_mode)mode) == CMX_OK);
        CHECK(sc_lock(&w, 2, 0, cmx_forever(), SC_TASK_CONTEXT) == CMX_OK);
        CHECK(sc_lock(&w, 2, 1, cmx_forever(), SC_TASK_CONTEXT) == CMX_OK);
        CHECK(sc_lock(&w, 1, 1, cmx_forever(), SC_TASK_CONTEXT) == CMX_BLOCKED);
        CHECK(sc_lock(&w, 0, 0, cmx_forever(), SC_TASK_CONTEXT) ==
                  CMX_BLOCKED &&
              w.kernel.task[2].priority == 2 && sc_invariant(&w));
        CHECK(sc_cancel(&w, 0) == CMX_OK && sc_poll(&w, 0) == CMX_CLOSED &&
              w.kernel.task[2].priority == (mode ? 2u : 5u));
        CHECK(sc_unlock(&w, 2, 0, SC_TASK_CONTEXT) == CMX_INVALID);
        CHECK(sc_unlock(&w, 2, 1, SC_TASK_CONTEXT) == CMX_OK &&
              w.kernel.task[2].priority == (mode ? 2u : 8u) &&
              sc_invariant(&w));
        CHECK(sc_unlock(&w, 1, 1, SC_TASK_CONTEXT) == CMX_BLOCKED);
        CHECK(sc_poll(&w, 1) == CMX_OK &&
              sc_unlock(&w, 1, 1, SC_TASK_CONTEXT) == CMX_OK);
        CHECK(sc_unlock(&w, 2, 0, SC_TASK_CONTEXT) == CMX_OK &&
              w.kernel.task[2].priority == 8 && sc_invariant(&w));
    }
    puts("SC_NESTED:PASS");
    return 0;
}

static int admission(void)
{
    struct sc_config c = config(1);
    struct sc_world w;
    CHECK(sc_init(&w, &c, SC_IMMEDIATE) == CMX_OK);
    CHECK(sc_lock(&w, 2, 1, cmx_forever(), SC_TASK_CONTEXT) == CMX_OK);
    CHECK(sc_lock(&w, 2, 0, cmx_forever(), SC_TASK_CONTEXT) == CMX_DEADLOCK);
    CHECK(sc_set_base(&w, 2, 1) == CMX_INVALID &&
          w.kernel.task[2].base_priority == 8);
    CHECK(sc_set_base(&w, 2, 8) == CMX_OK);
    CHECK(sc_tick(&w, 4) == CMX_OK);
    CHECK(sc_lock(&w, 1, 1, cmx_at(4), SC_TASK_CONTEXT) == CMX_TIMEOUT &&
          !w.pending[1]);
    CHECK(sc_lock(&w, 1, 1, cmx_at(6), SC_TASK_CONTEXT) == CMX_BLOCKED);
    CHECK(sc_tick(&w, 6) == CMX_OK && sc_poll(&w, 1) == CMX_TIMEOUT &&
          w.mutex[1].owner == 2);
    CHECK(sc_tick(&w, 5) == CMX_INVALID && w.kernel.now == 6);
    w.kernel.sequence = UINT32_MAX;
    CHECK(sc_lock(&w, 1, 1, cmx_forever(), SC_TASK_CONTEXT) == CMX_FULL &&
          !w.pending[1]);
    CHECK(sc_unlock(&w, 2, 1, SC_TASK_CONTEXT) == CMX_OK && sc_invariant(&w));
    puts("SC_ADMISSION:PASS");
    return 0;
}

static int workloads(void)
{
    for (unsigned mode = 0; mode < 3; mode++)
    {
        struct sc_config c = config(1);
        struct sc_world w;
        CHECK(sc_init(&w, &c, mode == 2 ? SC_IMMEDIATE : SC_INHERITANCE) ==
              CMX_OK);
        unsigned remaining[] = {1, 3, 4}, release[] = {1, 2, 0},
                 finish[3] = {0}, first[3] = {99, 99, 99}, trace[8],
                 blocked = 0, boosted = 0;
        unsigned suspended = 0;
        int owner = CMX_NONE, current = CMX_NONE;
        for (unsigned now = 0; now < 8; now++)
        {
            int chosen = CMX_NONE;
            for (unsigned attempt = 0; attempt < 3; attempt++)
            {
                unsigned ready = 0;
                for (unsigned t = 0; t < 3; t++)
                    if (release[t] <= now && remaining[t] &&
                        !(suspended & (1u << t)))
                        ready |= 1u << t;
                if (mode)
                    chosen = sc_choose(&w, ready, current);
                else
                {
                    chosen = CMX_NONE;
                    for (unsigned t = 0; t < 3; t++)
                        if ((ready & (1u << t)) &&
                            (chosen < 0 || c.priority[t] < c.priority[chosen]))
                            chosen = (int)t;
                }
                CHECK(chosen >= 0);
                if (chosen != 1)
                {
                    if (mode)
                    {
                        if (w.pending[chosen])
                            CHECK(sc_poll(&w, (unsigned)chosen) == CMX_OK);
                        if (!(w.held[chosen] & 1u))
                        {
                            enum cmx_result r =
                                sc_lock(&w, (unsigned)chosen, 0, cmx_forever(),
                                        SC_TASK_CONTEXT);
                            if (r == CMX_BLOCKED)
                                continue;
                            CHECK(r == CMX_OK);
                        }
                    }
                    else if (owner >= 0 && owner != chosen)
                    {
                        suspended |= 1u << (unsigned)chosen;
                        continue;
                    }
                    else
                        owner = chosen;
                }
                break;
            }
            CHECK(chosen >= 0);
            trace[now] = (unsigned)chosen;
            if (mode ? w.kernel.task[0].state == CMX_WAITING
                     : (suspended & 1u) != 0)
                blocked++;
            if (mode && chosen == 2 &&
                w.kernel.task[2].priority < c.priority[2])
                boosted++;
            if (first[chosen] == 99)
                first[chosen] = now;
            remaining[chosen]--;
            current = chosen;
            if (!remaining[chosen])
            {
                finish[chosen] = now + 1;
                current = CMX_NONE;
                if (chosen != 1)
                {
                    if (mode)
                        CHECK(sc_unlock(&w, (unsigned)chosen, 0,
                                        SC_TASK_CONTEXT) == CMX_OK);
                    else
                    {
                        owner = CMX_NONE;
                        suspended = 0;
                    }
                }
            }
            CHECK(sc_invariant(&w));
#ifndef SC_TARGET
            printf("SC_TRACE %u %u %u %u %u\n", mode, now, trace[now], blocked,
                   boosted);
#endif
        }
        unsigned none[] = {2, 2, 1, 1, 1, 2, 2, 0},
                 protected[] = {2, 2, 2, 2, 0, 1, 1, 1};
        CHECK(memcmp(trace, mode ? protected : none, sizeof trace) == 0);
        CHECK(finish[0] == (mode ? 5u : 8u) && first[1] == (mode ? 5u : 2u));
        CHECK(blocked == (mode == 0   ? 6u
                          : mode == 1 ? 3u
                                      : 0u) &&
              boosted == (mode == 0   ? 0u
                          : mode == 1 ? 3u
                                      : 4u));
    }
    puts("SC_WORKLOADS:PASS");
    return 0;
}
#ifndef SC_TARGET
static int stream(void)
{
    unsigned mode, strict, length;
    while (scanf("%u%u%u", &mode, &strict, &length) == 3)
    {
        if (length > 64)
            return 2;
        struct sc_config c = config(strict);
        struct sc_world w;
        enum cmx_result init = sc_init(&w, &c, (enum sc_mode)mode);
        if (init != CMX_OK)
            return 2;
        for (unsigned at = 0; at < length; at++)
        {
            unsigned op, t, arg;
            if (scanf("%u%u%u", &op, &t, &arg) != 3)
                return 2;
            enum cmx_result r;
            if (op == 0)
                r = sc_lock(&w, t, arg, cmx_forever(), SC_TASK_CONTEXT);
            else if (op == 1)
                r = sc_unlock(&w, t, arg, SC_TASK_CONTEXT);
            else if (op == 2)
                r = sc_cancel(&w, t);
            else if (op == 3)
                r = sc_poll(&w, t);
            else if (op == 4)
                r = sc_set_base(&w, t, arg);
            else
                return 2;
            printf("%u %d", r, sc_invariant(&w));
            for (unsigned i = 0; i < 4; i++)
                printf(" %u %u %u %u", w.kernel.task[i].priority, w.held[i],
                       w.pending[i], w.kernel.task[i].state == CMX_WAITING);
            for (unsigned i = 0; i < 2; i++)
                printf(" %d", w.mutex[i].owner);
            putchar('\n');
        }
    }
    return 0;
}

int main(int argc, char **argv)
{
    if (argc != 2)
        return 2;
    if (!strcmp(argv[1], "table"))
        return table();
    if (!strcmp(argv[1], "nested"))
        return nested();
    if (!strcmp(argv[1], "admission"))
        return admission();
    if (!strcmp(argv[1], "workloads"))
        return workloads();
    if (!strcmp(argv[1], "stream"))
        return stream();
    return 2;
}
#endif
