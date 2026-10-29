#include "events.h"
#include <stdio.h>
#include <string.h>
#define CHECK(c)                                                \
    do                                                          \
    {                                                           \
        if (!(c))                                               \
        {                                                       \
            fprintf(stderr, "CDE_CHECK:%d:%s\n", __LINE__, #c); \
            return 1;                                           \
        }                                                       \
    } while (0)

static int setup(cmx_kernel *k, struct cd_events *e)
{
    cmx_init(k, CMX_FIXED);
    for (unsigned t = 0; t < 3; t++)
    {
        if (cmx_task_add(k, 9 - t, 0, 0x1000 + 0x200 * t, 256) != (int)t)
        {
            return 0;
        }
    }
    return cde_init(e, k) == CMX_OK;
}

/* Independent Boolean oracle: inspect each named bit rather than calling the
   implementation's mask predicate or using its result as the expected value. */
static unsigned relation(unsigned bits, unsigned mask, unsigned all,
                         unsigned *observed)
{
    unsigned required = 0, present = 0;
    *observed = 0;
    for (unsigned bit = 0; bit < 3; bit++)
    {
        if (mask / (1u << bit) % 2u)
        {
            required++;
            if (bits / (1u << bit) % 2u)
            {
                present++;
                *observed += 1u << bit;
            }
        }
    }
    return all ? present == required : present != 0;
}

static int truth(void)
{
    unsigned cases = 0;
    for (unsigned bits = 0; bits < 8; bits++)
    {
        for (unsigned mask = 1; mask < 8; mask++)
        {
            for (unsigned all = 0; all < 2; all++)
            {
                for (unsigned consume = 0; consume < 2; consume++)
                {
                    cmx_kernel k;
                    struct cd_events e;
                    unsigned observed;
                    uint32_t out = 999;
                    unsigned matched = relation(bits, mask, all, &observed);
                    CHECK(setup(&k, &e) && cde_set(&e, bits) == CMX_OK);
                    CHECK(cde_wait(&e, 0, mask, all, consume, cmx_at(0)) ==
                          (matched ? CMX_OK : CMX_TIMEOUT));
                    CHECK(cde_result(&e, 0, &out) ==
                          (matched ? CMX_OK : CMX_TIMEOUT));
                    CHECK(out == (matched ? observed : 999));
                    CHECK(e.event.bits ==
                          (matched && consume ? bits - observed : bits));
                    CHECK(cde_result(&e, 0, &out) == CMX_INVALID);
                    cases++;
                }
            }
        }
    }
    cmx_kernel k;
    struct cd_events e;
    uint32_t out = 999;
    CHECK(setup(&k, &e));
    const uint32_t high = UINT32_C(1) << 31;
    CHECK(cde_set(&e, high | 3) == CMX_OK);
    CHECK(cde_wait(&e, 0, high | 1, 1, 1, cmx_forever()) == CMX_OK);
    CHECK(cde_result(&e, 0, &out) == CMX_OK && out == (high | 1) &&
          e.event.bits == 2);
    CHECK(cde_clear(&e, 2) == CMX_OK && e.event.bits == 0);
    CHECK(cde_wait(&e, 0, high, 0, 0, cmx_forever()) == CMX_BLOCKED);
    CHECK(cde_set(&e, high) == CMX_OK);
    CHECK(cde_result(&e, 0, &out) == CMX_OK && out == high &&
          e.event.bits == high);
    printf(
        "CDE_TRUTH:cases=%u;high-bit31;observed-only-clear;output-sentinels\n",
        cases);
    return 0;
}

static int order(void)
{
    unsigned cases = 0;
    for (unsigned a = 1; a < 8; a++)
    {
        for (unsigned b = 1; b < 8; b++)
        {
            for (unsigned modes = 0; modes < 4; modes++)
            {
                for (unsigned clears = 0; clears < 4; clears++)
                {
                    for (unsigned reverse = 0; reverse < 2; reverse++)
                    {
                        for (unsigned posted = 0; posted < 8; posted++)
                        {
                            cmx_kernel k;
                            struct cd_events e;
                            unsigned masks[2] = {a, b}, matched[2] = {0},
                                     values[2] = {0};
                            unsigned remaining = posted;
                            CHECK(setup(&k, &e));
                            for (unsigned step = 0; step < 2; step++)
                            {
                                unsigned t = step ^ reverse;
                                CHECK(cde_wait(&e, t, masks[t],
                                               (modes >> t) & 1u,
                                               (clears >> t) & 1u,
                                               cmx_forever()) == CMX_BLOCKED);
                                matched[t] =
                                    relation(remaining, masks[t],
                                             (modes >> t) & 1u, &values[t]);
                                if (matched[t] && ((clears >> t) & 1u))
                                {
                                    remaining -= values[t];
                                }
                            }
                            CHECK(cde_set(&e, posted) == CMX_OK &&
                                  e.event.bits == remaining);
                            for (unsigned t = 0; t < 2; t++)
                            {
                                uint32_t out = 999;
                                CHECK(cde_result(&e, t, &out) ==
                                      (matched[t] ? CMX_OK : CMX_BLOCKED));
                                CHECK(out == (matched[t] ? values[t] : 999));
                                if (!matched[t])
                                {
                                    CHECK(cmx_cancel(&k, t) == CMX_OK);
                                    CHECK(cde_result(&e, t, &out) ==
                                              CMX_CLOSED &&
                                          out == 999);
                                }
                            }
                            CHECK(cmx_invariant(&k) &&
                                  cde_destroy(&e) == CMX_OK);
                            cases++;
                        }
                    }
                }
            }
        }
    }
    cmx_kernel k;
    struct cd_events e;
    uint32_t out = 999;
    CHECK(setup(&k, &e));
    CHECK(cde_wait(&e, 0, 3, 1, 1, cmx_forever()) == CMX_BLOCKED);
    CHECK(cde_wait(&e, 1, 1, 0, 1, cmx_forever()) == CMX_BLOCKED);
    CHECK(cde_set(&e, 1) == CMX_OK);
    CHECK(cde_result(&e, 1, &out) == CMX_OK && out == 1);
    CHECK(cde_result(&e, 0, &out) == CMX_BLOCKED && out == 1);
    CHECK(cde_set(&e, 3) == CMX_OK);
    CHECK(cde_result(&e, 0, &out) == CMX_OK && out == 3 && e.event.bits == 0);
    printf(
        "CDE_ORDER:cases=%u;both-registration-orders;sequential-consumption;observer-position\n",
        cases);
    return 0;
}

static int terminal(void)
{
    cmx_kernel k;
    struct cd_events e;
    uint32_t out = 999;
    CHECK(setup(&k, &e) && cde_set(&e, 4) == CMX_OK);
    CHECK(cde_wait(&e, 0, 0, 0, 1, cmx_forever()) == CMX_INVALID &&
          e.event.bits == 4);
    CHECK(cde_wait(&e, 0, 1, 2, 1, cmx_forever()) == CMX_INVALID &&
          e.event.bits == 4);
    CHECK(cde_wait(&e, 3, 4, 0, 1, cmx_forever()) == CMX_INVALID &&
          e.event.bits == 4);
    CHECK(cde_wait(&e, 0, 1, 0, 1, cmx_at(5)) == CMX_BLOCKED);
    CHECK(cde_wait(&e, 0, 4, 0, 1, cmx_forever()) == CMX_INVALID &&
          e.event.bits == 4);
    CHECK(cde_destroy(&e) == CMX_BLOCKED);
    cmx_tick(&k, 5);
    CHECK(cde_set(&e, 1) == CMX_OK);
    CHECK(cde_result(&e, 0, &out) == CMX_TIMEOUT && out == 999 &&
          e.event.bits == 5);
    CHECK(cde_wait(&e, 0, 1, 0, 1, cmx_at(5)) == CMX_OK);
    CHECK(cde_result(&e, 0, &out) == CMX_OK && out == 1);
    CHECK(cde_wait(&e, 0, 3, 0, 0, cmx_forever()) == CMX_BLOCKED);
    CHECK(cde_wait(&e, 1, 3, 0, 0, cmx_forever()) == CMX_BLOCKED);
    CHECK(cde_set(&e, 2) == CMX_OK); /* STOP participates in both predicates. */
    for (unsigned t = 0; t < 2; t++)
    {
        CHECK(cde_result(&e, t, &out) == CMX_OK && out == 2);
    }
    CHECK(cde_clear(&e, UINT32_MAX) == CMX_OK);
    CHECK(cde_wait(&e, 0, 1, 0, 1, cmx_forever()) == CMX_BLOCKED);
    CHECK(cde_wait(&e, 1, 1, 0, 1, cmx_forever()) == CMX_BLOCKED);
    CHECK(cde_close(&e) == CMX_OK && cde_set(&e, 1) == CMX_CLOSED);
    CHECK(cde_destroy(&e) == CMX_BLOCKED);
    for (unsigned t = 0; t < 2; t++)
    {
        out = 999;
        CHECK(cde_result(&e, t, &out) == CMX_CLOSED && out == 999);
    }
    CHECK(cde_destroy(&e) == CMX_OK);
    CHECK(setup(&k, &e));
    k.sequence = UINT32_MAX;
    CHECK(cde_wait(&e, 0, 1, 0, 0, cmx_forever()) == CMX_FULL);
    CHECK(cde_result(&e, 0, &out) == CMX_FULL && out == 999);
    puts(
        "CDE_TERMINAL:zero-invalid-unchanged;timeout-before-set;stop-observers;close-drain;enrollment-exhaustion");
    return 0;
}

static int sequence(void)
{
    cmx_kernel k;
    struct cd_events e;
    struct cde_sequence s;
    struct cde_cursor cursor = {0, 0};
    uint32_t out = 999;
    uint64_t delta = 999, missed = 999;
    CHECK(setup(&k, &e));
    cde_sequence_init(&s);
    for (unsigned i = 0; i < 3; i++)
    {
        CHECK(cde_set(&e, 4) == CMX_OK && cde_sequence_publish(&s) == CMX_OK);
        CHECK(cmx_notify(&k, 1, 1) == CMX_OK);
    }
    CHECK(e.event.bits == 4 && k.task[1].notifications == 3);
    CHECK(cde_wait(&e, 0, 4, 0, 1, cmx_at(0)) == CMX_OK);
    CHECK(cde_result(&e, 0, &out) == CMX_OK && out == 4);
    CHECK(cde_wait(&e, 0, 4, 0, 1, cmx_at(0)) == CMX_TIMEOUT);
    CHECK(cde_result(&e, 0, &out) == CMX_TIMEOUT);
    for (unsigned i = 0; i < 3; i++)
    {
        CHECK(cmx_notify_take(&k, 1, cmx_at(0)) == CMX_OK);
    }
    CHECK(cmx_notify_take(&k, 1, cmx_at(0)) == CMX_TIMEOUT);
    CHECK(cde_sequence_read(&s, &cursor, &delta, &missed) == CMX_OK &&
          delta == 3 && missed == 2);
    delta = missed = 999;
    CHECK(cde_sequence_read(&s, &cursor, &delta, &missed) == CMX_BLOCKED &&
          delta == 999 && missed == 999);
    s.current = (struct cde_cursor){7, UINT32_MAX - 1};
    cursor = s.current;
    CHECK(cde_sequence_publish(&s) == CMX_OK &&
          cde_sequence_publish(&s) == CMX_OK);
    CHECK(s.current.epoch == 8 && s.current.serial == 0);
    CHECK(cde_sequence_read(&s, &cursor, &delta, &missed) == CMX_OK &&
          delta == 2 && missed == 1);
    cursor = (struct cde_cursor){9, 0};
    delta = missed = 999;
    CHECK(cde_sequence_read(&s, &cursor, &delta, &missed) == CMX_INVALID &&
          cursor.epoch == 9 && delta == 999 && missed == 999);
    s.current = (struct cde_cursor){UINT32_MAX, UINT32_MAX};
    cursor = (struct cde_cursor){0, 0};
    CHECK(cde_sequence_publish(&s) == CMX_FULL &&
          s.current.epoch == UINT32_MAX && s.current.serial == UINT32_MAX);
    CHECK(cde_sequence_read(&s, &cursor, &delta, &missed) == CMX_OK &&
          delta == UINT64_MAX && missed == UINT64_MAX - 1);
    puts(
        "CDE_SEQUENCE:three-sets-one-bit-three-counts;gap3-missed2;low-word-wrap;finite-exhaustion;future-cursor-refusal");
    return 0;
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        return 2;
    }
    if (!strcmp(argv[1], "truth"))
    {
        return truth();
    }
    if (!strcmp(argv[1], "order"))
    {
        return order();
    }
    if (!strcmp(argv[1], "terminal"))
    {
        return terminal();
    }
    if (!strcmp(argv[1], "sequence"))
    {
        return sequence();
    }
    return 2;
}
