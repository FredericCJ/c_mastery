#include "cohort.h"
#ifdef SU_TARGET
#include <stddef.h>

static int su_target_compare(const void *a, const void *b, size_t n)
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

#define memcmp su_target_compare
#define puts su_target_puts
#define CHECK(x)               \
    do                         \
    {                          \
        if (!(x))              \
            su_target_stop(1); \
    } while (0)
#else
#include <stdio.h>
#include <string.h>
#define CHECK(x)                                               \
    do                                                         \
    {                                                          \
        if (!(x))                                              \
        {                                                      \
            fprintf(stderr, "SU_CHECK:%d:%s\n", __LINE__, #x); \
            return 1;                                          \
        }                                                      \
    } while (0)
#endif
#include "server.h"
#include "specimens.h"

static int cohort(void)
{
    struct su_state s;
    CHECK(su_init(&s, 20, 50, 0, 0) == CMX_OK && su_active(&s, 0, 1) == CMX_OK);
    CHECK(su_run(&s, 0, 18) == CMX_OK && su_active(&s, 18, 0) == CMX_OK);
    CHECK(su_active(&s, 40, 1) == CMX_OK && su_run(&s, 40, 1) == CMX_OK);
    /* The priority interval stays active while a higher-priority task runs. */
    CHECK(su_active(&s, 51, 1) == CMX_OK && su_invariant(&s));
    unsigned old = 0, newer = 0;
    for (unsigned i = 0; i < s.q; ++i)
    {
        if (s.unit[i].active)
        {
            old += s.unit[i].anchor == 40;
            newer += s.unit[i].anchor == 50;
        }
    }
    CHECK(old == 1 && newer == 18);
    CHECK(su_run(&s, 51, 19) == CMX_OK && su_active(&s, 70, 0) == CMX_OK);
    CHECK(su_active(&s, 90, 1) == CMX_OK && su_available(&s) == 2);
    CHECK(su_run(&s, 90, 2) == CMX_OK && !su_available(&s));
    CHECK(su_active(&s, 99, 1) == CMX_OK && !su_available(&s));
    CHECK(su_active(&s, 100, 1) == CMX_OK && su_available(&s) == 18);
    CHECK(su_init(&s, 2, 10, 0, 0) == CMX_OK && su_active(&s, 0, 1) == CMX_OK);
    CHECK(su_run(&s, 0, 1) == CMX_OK && su_active(&s, 1, 0) == CMX_OK);
    CHECK(su_active(&s, 8, 1) == CMX_OK && su_run(&s, 8, 1) == CMX_OK);
    CHECK(su_active(&s, 10, 1) == CMX_OK && su_available(&s) == 1);
    puts("SU_COHORT:pass");
    return 0;
}

static int overrun(void)
{
    struct su_state s;
    struct su_flawed bad;
    CHECK(su_init(&s, 4, 20, 0, 1) == CMX_OK && su_active(&s, 0, 1) == CMX_OK);
    CHECK(su_run(&s, 0, 2) == CMX_OK && su_active(&s, 2, 0) == CMX_OK);
    CHECK(su_active(&s, 10, 1) == CMX_OK && su_run(&s, 10, 3) == CMX_OK);
    CHECK(s.borrowed == 1 && s.served == 5 && su_invariant(&s));
    unsigned at20 = 0, at30 = 0, at40 = 0;
    for (unsigned i = 0; i < s.q; ++i)
    {
        at20 += s.unit[i].due == 20;
        at30 += s.unit[i].due == 30;
        at40 += s.unit[i].due == 40;
    }
    CHECK(at20 == 1 && at30 == 2 && at40 == 1);
    CHECK(su_active(&s, 20, 1) == CMX_OK && su_available(&s) == 1);
    struct su_state saved = s;
    CHECK(su_run(&s, 20, 3) == CMX_FULL && !memcmp(&s, &saved, sizeof s));
    su_flawed_init(&bad, 4, 20);
    su_flawed_run(&bad, 0, 2);
    su_flawed_active(&bad, 2, 0);
    su_flawed_run(&bad, 10, 3);
    unsigned total = bad.remaining;
    for (unsigned i = 0; i < bad.count; ++i)
    {
        total += bad.amount[i];
    }
    CHECK(total == 5 && total > bad.q);
    puts("SU_OVERRUN:pass");
    return 0;
}

static int guard(void)
{
    struct su_state s;
    CHECK(su_init(&s, 2, 10, 0, 0) == CMX_OK && su_active(&s, 0, 1) == CMX_OK &&
          su_run(&s, 0, 2) == CMX_OK);
    struct su_state saved = s;
    CHECK(su_run(&s, 10, 3) == CMX_FULL && !memcmp(&s, &saved, sizeof s));
    CHECK(su_active(&s, 1, 0) == CMX_INVALID && !memcmp(&s, &saved, sizeof s));
    CHECK(su_init(&s, 33, 40, 0, 0) == CMX_INVALID &&
          !memcmp(&s, &saved, sizeof s));
    CHECK(su_init(&s, 2, 10, UINT32_MAX - 5, 0) == CMX_OK &&
          su_active(&s, UINT32_MAX - 5, 1) == CMX_OK);
    saved = s;
    CHECK(su_run(&s, UINT32_MAX - 5, 1) == CMX_FULL &&
          !memcmp(&s, &saved, sizeof s));
    CHECK(su_init(&s, 2, 10, 0, 0) == CMX_OK && su_active(&s, 0, 1) == CMX_OK);
    CHECK(su_active(&s, 100, 1) == CMX_OK && su_run(&s, 100, 1) == CMX_OK);
    CHECK(su_invariant(&s) && s.last == 101 && s.served == 1);
    s.unit[0].active = 2;
    saved = s;
    CHECK(su_active(&s, 102, 0) == CMX_BROKEN && !memcmp(&s, &saved, sizeof s));
    puts("SU_GUARD:pass");
    return 0;
}

static unsigned completion(unsigned flawed)
{
    struct su_state s;
    struct su_flawed bad;
    if (su_init(&s, 20, 50, 0, 0) != CMX_OK)
    {
        return 0;
    }
    su_flawed_init(&bad, 20, 50);
    unsigned release[3] = {0, 40, 90}, cost[3] = {18, 20, 20}, left[3] = {0},
             hp = 0, low = 49;
    for (unsigned t = 0; t < 150; ++t)
    {
        for (unsigned i = 0; i < 3; ++i)
        {
            if (release[i] == t)
            {
                left[i] = cost[i];
            }
        }
        if (t == 41)
        {
            hp = 10;
        }
        if (flawed)
        {
            su_flawed_update(&bad, t);
        }
        else if (su_active(&s, t, s.active) != CMX_OK)
        {
            return 0;
        }
        unsigned index = 0;
        while (index < 3 && !left[index])
        {
            ++index;
        }
        unsigned available = flawed ? bad.remaining : su_available(&s);
        unsigned active = hp || (index < 3 && available);
        if (flawed)
        {
            su_flawed_active(&bad, t, active);
        }
        else if (su_active(&s, t, active) != CMX_OK)
        {
            return 0;
        }
#ifndef SU_TARGET
        printf("TRACE %u %u %u %u %u\n", flawed, t,
               hp                       ? 3
               : index < 3 && available ? index
                                        : 4,
               available, low);
#endif
        if (hp)
        {
            --hp;
        }
        else if (index < 3 && available)
        {
            if (flawed)
            {
                su_flawed_run(&bad, t, 1);
            }
            else if (su_run(&s, t, 1) != CMX_OK)
            {
                return 0;
            }
            --left[index];
        }
        else if (low && !--low)
        {
            return t + 1;
        }
    }
    return 0;
}

static int comparison(void)
{
    CHECK(completion(0) == 99 && completion(1) == 117);
    struct ss_budget dispatch;
    struct su_state s;
    CHECK(ss_init(&dispatch, 2, 10, 0, 16, SS_REJECT) == CMX_OK);
    CHECK(su_init(&s, 2, 10, 0, 0) == CMX_OK && su_active(&s, 0, 1) == CMX_OK);
    CHECK(ss_charge(&dispatch, 4, 1) == CMX_OK && su_run(&s, 4, 1) == CMX_OK);
    CHECK(su_available(&s) == 1);
    CHECK(su_active(&s, 5, 0) == CMX_OK && su_active(&s, 10, 1) == CMX_OK);
    CHECK(su_available(&s) == 2 && ss_update(&dispatch, 10) == CMX_OK &&
          dispatch.core.remaining == 1);
    CHECK(ss_update(&dispatch, 14) == CMX_OK && dispatch.core.remaining == 2);
    puts("SU_COMPARISON:pass");
    return 0;
}
#ifndef SU_TARGET
static int stream(void)
{
    unsigned q, p, start, overrun, n;
    while (scanf("%u%u%u%u%u", &q, &p, &start, &overrun, &n) == 5)
    {
        struct su_state s;
        CHECK(su_init(&s, q, p, start, overrun) == CMX_OK);
        for (unsigned i = 0; i < n; ++i)
        {
            unsigned op, t, arg;
            if (scanf("%u%u%u", &op, &t, &arg) != 3)
            {
                return 2;
            }
            enum cmx_result r = op ? su_run(&s, t, arg) : su_active(&s, t, arg);
            printf("U %u %u %u %llu %llu %u", (unsigned)r, s.last, s.active,
                   (unsigned long long)s.served, (unsigned long long)s.borrowed,
                   su_available(&s));
            for (unsigned j = 0; j < s.q; ++j)
            {
                printf(" %u %u %u", s.unit[j].due, s.unit[j].anchor,
                       s.unit[j].active);
            }
            puts("");
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
    if (!strcmp(argv[1], "cohort"))
    {
        return cohort();
    }
    if (!strcmp(argv[1], "overrun"))
    {
        return overrun();
    }
    if (!strcmp(argv[1], "guard"))
    {
        return guard();
    }
    if (!strcmp(argv[1], "comparison"))
    {
        return comparison();
    }
    if (!strcmp(argv[1], "stream"))
    {
        return stream();
    }
    return 2;
}
#endif
