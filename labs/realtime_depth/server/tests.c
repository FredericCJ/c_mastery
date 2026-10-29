#include "server.h"
#ifdef SS_TARGET
#include <stddef.h>

static int ss_target_compare(const void *a, const void *b, size_t n)
{
    const unsigned char *x = a, *y = b;
    for (size_t i = 0; i < n; i++)
    {
        if (x[i] != y[i])
        {
            return 1;
        }
    }
    return 0;
}

#define memcmp ss_target_compare
#define puts ss_target_puts
#define CHECK(x)               \
    do                         \
    {                          \
        if (!(x))              \
            ss_target_stop(1); \
    } while (0)
#else
#include <stdio.h>
#include <string.h>
#define CHECK(x)                                               \
    do                                                         \
    {                                                          \
        if (!(x))                                              \
        {                                                      \
            fprintf(stderr, "SS_CHECK:%d:%s\n", __LINE__, #x); \
            return 1;                                          \
        }                                                      \
    } while (0)
#endif
static int ledger(void)
{
    struct ss_budget s;
    CHECK(ss_init(&s, 3, 10, 0, 16, SS_REJECT) == CMX_OK);
    CHECK(ss_charge(&s, 2, 1) == CMX_OK && ss_charge(&s, 5, 2) == CMX_OK);
    CHECK(ss_invariant(&s) && s.core.remaining == 0 && s.core.count == 2);
    CHECK(s.core.at[0] == 12 && s.core.amount[0] == 1 && s.core.at[1] == 15 &&
          s.core.amount[1] == 2);
    CHECK(ss_update(&s, 11) == CMX_OK && s.core.remaining == 0);
    CHECK(ss_update(&s, 12) == CMX_OK && s.core.remaining == 1);
    CHECK(ss_update(&s, 15) == CMX_OK && s.core.remaining == 3 &&
          !s.core.count);
    CHECK(ss_init(&s, 20, 100, 0, 16, SS_REJECT) == CMX_OK);
    for (unsigned i = 0; i < 16; ++i)
    {
        CHECK(ss_charge(&s, i, 1) == CMX_OK);
    }
    struct ss_budget saved = s;
    CHECK(ss_charge(&s, 16, 1) == CMX_FULL && !memcmp(&s, &saved, sizeof s));
    CHECK(ss_charge(&s, 15, 1) == CMX_OK && s.core.count == 16 &&
          s.core.amount[15] == 2);
    CHECK(ss_init(&s, 4, 10, UINT32_MAX - 3, 2, SS_REJECT) == CMX_OK);
    CHECK(ss_charge(&s, UINT32_MAX - 2, 2) == CMX_OK && s.core.at[0] == 7);
    CHECK(ss_update(&s, 6) == CMX_OK && s.core.remaining == 2);
    CHECK(ss_update(&s, 7) == CMX_OK && s.core.remaining == 4);
    saved = s;
    CHECK(ss_update(&s, 6) == CMX_INVALID && !memcmp(&s, &saved, sizeof s));
    CHECK(ss_update(&s, 7 + 0x80000000u) == CMX_INVALID &&
          !memcmp(&s, &saved, sizeof s));
    puts("SS_LEDGER:pass");
    return 0;
}

static int transaction(void)
{
    struct ss_budget s;
    CHECK(ss_init(&s, 4, 10, 0, 2, SS_DELAY_LATEST) == CMX_OK);
    CHECK(ss_charge(&s, 0, 1) == CMX_OK && ss_charge(&s, 1, 1) == CMX_OK);
    CHECK(ss_charge(&s, 2, 1) == CMX_OK && ss_invariant(&s));
    CHECK(s.core.count == 2 && s.core.at[0] == 10 && s.core.amount[0] == 1 &&
          s.core.at[1] == 12 && s.core.amount[1] == 2);
    CHECK(ss_update(&s, 11) == CMX_OK && s.core.remaining == 2);
    CHECK(ss_update(&s, 12) == CMX_OK && s.core.remaining == 4);
    CHECK(ss_init(&s, 3, 10, 0, 16, SS_REJECT) == CMX_OK &&
          ss_charge(&s, 0, 3) == CMX_OK);
    struct ss_budget saved = s;
    cmx_server raw = s.core;
    CHECK(cmx_server_charge(&raw, 10, 4) == CMX_FULL && raw.remaining == 3 &&
          raw.last == 10 && !raw.count);
    CHECK(ss_charge(&s, 10, 4) == CMX_FULL && !memcmp(&s, &saved, sizeof s));
    CHECK(ss_charge(&s, 10, 0) == CMX_INVALID && !memcmp(&s, &saved, sizeof s));
    s.core.amount[0] = 4;
    saved = s;
    CHECK(ss_update(&s, 10) == CMX_BROKEN && !memcmp(&s, &saved, sizeof s));
    CHECK(ss_init(&s, 4, 3, 0, 16, SS_REJECT) == CMX_INVALID &&
          !memcmp(&s, &saved, sizeof s));
    puts("SS_TRANSACTION:pass");
    return 0;
}

static int grouped(void)
{
    struct ss_comparison c;
    uint32_t service[] = {4, 6};
    CHECK(ss_compare(4, 10, 4, 7, service, 2, &c) == CMX_OK);
    CHECK(c.dispatch.core.count == 2 && c.dispatch.core.at[0] == 14 &&
          c.dispatch.core.at[1] == 16);
    CHECK(c.grouped.core.count == 1 && c.grouped.core.at[0] == 14 &&
          c.grouped.core.amount[0] == 2);
    CHECK(ss_update(&c.dispatch, 14) == CMX_OK &&
          ss_update(&c.grouped, 14) == CMX_OK);
    CHECK(c.dispatch.core.remaining == 3 && c.grouped.core.remaining == 4);
    struct ss_comparison saved = c;
    CHECK(ss_compare(4, 10, 4, 14, service, 2, &c) == CMX_INVALID &&
          !memcmp(&c, &saved, sizeof c));
    service[1] = 4;
    CHECK(ss_compare(4, 10, 4, 7, service, 2, &c) == CMX_INVALID &&
          !memcmp(&c, &saved, sizeof c));
    puts("SS_GROUPED:pass");
    return 0;
}

static int workloads(void)
{
    struct ss_workload w = {0};
    w.count = 1;
    w.capacity = 2;
    w.horizon = 30;
    w.q = 3;
    w.period = 10;
    w.request[0] = (struct ss_request){2, 2};
    struct ss_trace background, polling, dispatch;
    CHECK(ss_workload(&w, SS_BACKGROUND, &background) == CMX_OK);
    CHECK(ss_workload(&w, SS_POLLING, &polling) == CMX_OK);
    CHECK(ss_workload(&w, SS_DISPATCH, &dispatch) == CMX_OK);
    CHECK(background.finish[0] == 5 && polling.finish[0] == 13 &&
          dispatch.finish[0] == 4);
    CHECK(polling.supervisor == 6 && polling.periodic == 6 &&
          !polling.periodic_misses);
    w.count = 32;
    w.horizon = 40;
    for (unsigned i = 0; i < w.count; ++i)
    {
        w.request[i] = (struct ss_request){i, 4};
    }
    for (unsigned policy = 0; policy < 3; ++policy)
    {
        struct ss_trace t;
        CHECK(ss_workload(&w, (enum ss_policy)policy, &t) == CMX_OK &&
              t.supervisor == 8);
        unsigned rejected = 0, admitted = 0, work = 0, left = 0;
        for (unsigned i = 0; i < w.count; ++i)
        {
            CHECK(t.admitted[i] + t.rejected[i] == 1);
            CHECK(!t.admitted[i] || t.executed[i] + t.remaining[i] == 4);
            CHECK(!t.rejected[i] || (!t.executed[i] && !t.remaining[i] &&
                                     t.finish[i] == SS_NONE));
            rejected += t.rejected[i];
            admitted += t.admitted[i];
            work += t.executed[i];
            left += t.remaining[i];
        }
        CHECK(rejected && admitted && work + left == 4 * admitted);
        CHECK(t.periodic + t.periodic_dropped + t.periodic_remaining == 8);
        for (unsigned i = 0; i < w.horizon; ++i)
        {
            CHECK(t.backlog[i] <= 2);
        }
        if (policy == SS_DISPATCH)
        {
            for (unsigned i = 0; i + 10 <= w.horizon; ++i)
            {
                unsigned sum = 0;
                for (unsigned j = i; j < i + 10; ++j)
                {
                    sum += t.service[j] < SS_REQUESTS;
                }
                CHECK(sum <= 3);
            }
        }
    }
    struct ss_trace saved = dispatch;
    w.capacity = 0;
    CHECK(ss_workload(&w, SS_DISPATCH, &dispatch) == CMX_INVALID &&
          !memcmp(&dispatch, &saved, sizeof dispatch));
    puts("SS_WORKLOADS:pass");
    return 0;
}
#ifndef SS_TARGET
static int stream(void)
{
    unsigned kind;
    while (scanf("%u", &kind) == 1)
    {
        if (kind == 0)
        {
            unsigned q, p, start, cap, merge, n;
            if (scanf("%u%u%u%u%u%u", &q, &p, &start, &cap, &merge, &n) != 6)
            {
                return 2;
            }
            struct ss_budget s;
            CHECK(ss_init(&s, q, p, start, cap, (enum ss_merge)merge) ==
                  CMX_OK);
            for (unsigned i = 0; i < n; ++i)
            {
                unsigned now, amount;
                if (scanf("%u%u", &now, &amount) != 2)
                {
                    return 2;
                }
                enum cmx_result r =
                    amount ? ss_charge(&s, now, amount) : ss_update(&s, now);
                printf("B %u %u %u %u", (unsigned)r, s.core.remaining,
                       s.core.last, s.core.count);
                for (unsigned j = 0; j < s.core.count; ++j)
                {
                    printf(" %u %u", s.core.at[j], s.core.amount[j]);
                }
                puts("");
            }
        }
        else if (kind == 1)
        {
            struct ss_workload w = {0};
            unsigned policy;
            if (scanf("%u%u%u%u%u%u", &policy, &w.q, &w.period, &w.capacity,
                      &w.horizon, &w.count) != 6)
            {
                return 2;
            }
            if (w.count > SS_REQUESTS)
            {
                return 2;
            }
            for (unsigned i = 0; i < w.count; ++i)
            {
                if (scanf("%u%u", &w.request[i].release, &w.request[i].cost) !=
                    2)
                {
                    return 2;
                }
            }
            struct ss_trace t;
            CHECK(ss_workload(&w, (enum ss_policy)policy, &t) == CMX_OK);
            printf("W %u %u %u %u %u", t.supervisor, t.periodic,
                   t.periodic_misses, t.periodic_dropped, t.periodic_remaining);
            for (unsigned i = 0; i < w.count; ++i)
            {
                printf(" %u %u %u %u %u", t.admitted[i], t.rejected[i],
                       t.remaining[i], t.finish[i], t.executed[i]);
            }
            for (unsigned i = 0; i < w.horizon; ++i)
            {
                printf(" %u %u %u %u %u", t.service[i], t.budget[i],
                       t.backlog[i], t.replenished[i], t.records[i]);
            }
            puts("");
        }
        else
        {
            return 2;
        }
    }
    return 0;
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        return 2;
    }
    if (!strcmp(argv[1], "ledger"))
    {
        return ledger();
    }
    if (!strcmp(argv[1], "transaction"))
    {
        return transaction();
    }
    if (!strcmp(argv[1], "grouped"))
    {
        return grouped();
    }
    if (!strcmp(argv[1], "workloads"))
    {
        return workloads();
    }
    if (!strcmp(argv[1], "stream"))
    {
        return stream();
    }
    return 2;
}
#endif
