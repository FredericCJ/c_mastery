#include "barrier.h"
#include <stdio.h>
#include <string.h>
#define CHECK(c)                                                \
    do                                                          \
    {                                                           \
        if (!(c))                                               \
        {                                                       \
            fprintf(stderr, "CDB_CHECK:%d:%s\n", __LINE__, #c); \
            return 1;                                           \
        }                                                       \
    } while (0)

static int setup(cmx_kernel *k, struct cd_barrier *b, struct cdb_member h[4],
                 unsigned mask, unsigned limit)
{
    cmx_init(k, CMX_FIXED);
    for (unsigned i = 0; i < 4; i++)
    {
        if (cmx_task_add(k, 5 + i, 0, 0x1000 + 256 * i, 256) != (int)i)
        {
            return 0;
        }
    }
    if (cdb_init(b, k, mask, limit) != CMX_OK)
    {
        return 0;
    }
    for (unsigned i = 0; i < 4; i++)
    {
        if ((mask >> i) & 1u)
        {
            if (cdb_register(b, i, &h[i]) != CMX_OK)
            {
                return 0;
            }
        }
    }
    return 1;
}

static int rounds(void)
{
    static const unsigned order[6][3] = {{0, 1, 2}, {0, 2, 1}, {1, 0, 2},
                                         {1, 2, 0}, {2, 0, 1}, {2, 1, 0}};
    unsigned histories = 0;
    for (unsigned a = 0; a < 6; a++)
    {
        for (unsigned z = 0; z < 6; z++)
        {
            cmx_kernel k;
            struct cd_barrier b;
            struct cdb_member h[4];
            struct cdb_ticket old[3], next[3], duplicate;
            unsigned out = 999;
            CHECK(setup(&k, &b, h, 7, 3));
            for (unsigned j = 0; j < 3; j++)
            {
                unsigned t = order[a][j];
                CHECK(cdb_arrive(h[t], cmx_forever(), &old[t]) ==
                      (j == 2 ? CMX_OK : CMX_BLOCKED));
                CHECK(cdb_arrive(h[t], cmx_forever(), &duplicate) ==
                      CMX_INVALID);
                if (j < 2)
                {
                    CHECK(cdb_result(old[t], &out) == CMX_BLOCKED &&
                          out == 999 && b.raw.generation == 0);
                }
            }
            CHECK(b.raw.generation == 1);
            /* The fastest participant enters round1 before its peers collect round0. */
            unsigned fast = order[z][0];
            CHECK(cdb_result(old[fast], &out) == CMX_OK && out == 0);
            CHECK(cdb_arrive(h[fast], cmx_forever(), &next[fast]) ==
                  CMX_BLOCKED);
            out = 999;
            CHECK(cdb_result(old[fast], &out) == CDB_STALE && out == 999);
            for (unsigned j = 1; j < 3; j++)
            {
                unsigned t = order[z][j];
                CHECK(cdb_result(old[t], &out) == CMX_OK && out == 0);
                CHECK(cdb_arrive(h[t], cmx_forever(), &next[t]) ==
                      (j == 2 ? CMX_OK : CMX_BLOCKED));
                if (j == 1)
                {
                    out = 999;
                    CHECK(cdb_result(next[t], &out) == CMX_BLOCKED &&
                          out == 999);
                }
            }
            for (unsigned t = 0; t < 3; t++)
            {
                CHECK(cdb_result(next[t], &out) == CMX_OK && out == 1);
            }
            CHECK(b.raw.generation == 2 && b.raw.arrived == 0);
            histories++;
        }
    }
    printf(
        "CDB_ROUNDS:histories=%u;two-rounds;fast-reentry;old-ticket-refusal;all-arrival-orders\n",
        histories);
    return 0;
}

static int terminal(void)
{
    cmx_kernel k;
    struct cd_barrier b;
    struct cdb_member h[4];
    struct cdb_ticket t[3];
    unsigned out = 999;
    CHECK(setup(&k, &b, h, 7, 5));
    CHECK(cdb_arrive(h[0], cmx_at(5), &t[0]) == CMX_BLOCKED);
    CHECK(cdb_arrive(h[1], cmx_forever(), &t[1]) == CMX_BLOCKED);
    CHECK(cdb_stage(&b, 7) == CMX_BLOCKED && cdb_close(&b) == CMX_BLOCKED);
    cmx_tick(&k, 5);
    CHECK(b.raw.broken && b.raw.generation == 1 && b.raw.arrived == 0);
    CHECK(cdb_arrive(h[2], cmx_forever(), &t[2]) == CMX_BROKEN);
    CHECK(cdb_stage(&b, 7) ==
          CMX_BLOCKED); /* Results still hold old references. */
    CHECK(cdb_result(t[0], &out) == CMX_TIMEOUT && out == 999);
    CHECK(cdb_result(t[1], &out) == CMX_BROKEN && out == 999);
    CHECK(cdb_stage(&b, 7) == CMX_OK && cdb_apply(&b) == CMX_BLOCKED);
    struct cdb_member copied = h[0];
    for (unsigned i = 0; i < 3; i++)
    {
        CHECK(cdb_release(&h[i]) == CMX_OK);
    }
    CHECK(cdb_apply(&b) == CMX_OK && !b.raw.broken && b.raw.generation == 1);
    for (unsigned i = 0; i < 3; i++)
    {
        CHECK(cdb_register(&b, i, &h[i]) == CMX_OK);
    }
    CHECK(cdb_arrive(copied, cmx_forever(), &t[2]) == CDB_STALE);
    CHECK(cdb_arrive(h[0], cmx_forever(), &t[0]) == CMX_BLOCKED);
    CHECK(cdb_arrive(h[1], cmx_forever(), &t[1]) == CMX_BLOCKED);
    CHECK(cdb_cancel(t[0]) == CMX_CLOSED);
    CHECK(cdb_result(t[0], &out) == CMX_CLOSED && out == 999);
    CHECK(cdb_result(t[1], &out) == CMX_BROKEN && out == 999);
    CHECK(cdb_close(&b) == CMX_OK);
    for (unsigned i = 0; i < 3; i++)
    {
        CHECK(cdb_release(&h[i]) == CMX_OK);
    }
    CHECK(cdb_destroy(&b) == CMX_OK);
    puts(
        "CDB_TERMINAL:timeout-versus-broken;cancel-versus-broken;result-drain-before-reset;old-epoch-refusal");
    return 0;
}

static int membership(void)
{
    cmx_kernel k;
    struct cd_barrier b;
    struct cdb_member h[4], stale;
    struct cdb_ticket t[4];
    unsigned out;
    CHECK(setup(&k, &b, h, 7, 8));
    for (unsigned i = 0; i < 3; i++)
    {
        CHECK(cdb_arrive(h[i], cmx_forever(), &t[i]) ==
              (i == 2 ? CMX_OK : CMX_BLOCKED));
    }
    CHECK(cdb_stage(&b, 5) == CMX_BLOCKED);
    for (unsigned i = 0; i < 3; i++)
    {
        CHECK(cdb_result(t[i], &out) == CMX_OK && out == 0);
    }
    CHECK(cdb_stage(&b, 0) == CMX_INVALID);
    CHECK(cdb_stage(&b, 5) == CMX_OK);
    stale = h[1];
    CHECK(cdb_arrive(h[0], cmx_forever(), &t[0]) == CMX_INVALID &&
          cdb_apply(&b) == CMX_BLOCKED);
    for (unsigned i = 0; i < 3; i++)
    {
        CHECK(cdb_release(&h[i]) == CMX_OK);
    }
    CHECK(cdb_apply(&b) == CMX_OK && b.raw.parties == 2);
    CHECK(cdb_register(&b, 0, &h[0]) == CMX_OK &&
          cdb_register(&b, 2, &h[2]) == CMX_OK);
    CHECK(cdb_register(&b, 1, &h[1]) == CMX_INVALID &&
          cdb_arrive(stale, cmx_forever(), &t[1]) == CDB_STALE);
    CHECK(cdb_arrive(h[2], cmx_forever(), &t[2]) == CMX_BLOCKED &&
          cdb_arrive(h[0], cmx_forever(), &t[0]) == CMX_OK);
    CHECK(cdb_result(t[0], &out) == CMX_OK && out == 1 &&
          cdb_result(t[2], &out) == CMX_OK && out == 1);
    CHECK(cdb_stage(&b, 13) == CMX_OK && cdb_release(&h[0]) == CMX_OK &&
          cdb_release(&h[2]) == CMX_OK && cdb_apply(&b) == CMX_OK);
    for (unsigned i = 0; i < 4; i++)
    {
        if (i != 1)
        {
            CHECK(cdb_register(&b, i, &h[i]) == CMX_OK);
        }
    }
    CHECK(cdb_arrive(h[3], cmx_forever(), &t[3]) == CMX_BLOCKED &&
          cdb_arrive(h[0], cmx_forever(), &t[0]) == CMX_BLOCKED &&
          cdb_arrive(h[2], cmx_forever(), &t[2]) == CMX_OK);
    for (unsigned i = 0; i < 4; i++)
    {
        if (i != 1)
        {
            CHECK(cdb_result(t[i], &out) == CMX_OK && out == 2);
        }
    }
    puts(
        "CDB_MEMBERSHIP:three-to-two-withdrawal;quiescent-registration-epoch;two-to-three-join;no-midround-change");
    return 0;
}

static int generation(void)
{
    cmx_kernel k;
    struct cd_barrier b;
    struct cdb_member h[4];
    struct cdb_ticket t;
    unsigned out = 999;
    CHECK(setup(&k, &b, h, 1, 3));
    CHECK(cdb_init(&b, &k, 0, 3) == CMX_INVALID);
    for (unsigned i = 0; i < 3; i++)
    {
        CHECK(cdb_arrive(h[0], cmx_forever(), &t) == CMX_OK &&
              cdb_result(t, &out) == CMX_OK && out == i);
    }
    CHECK(b.raw.generation == 3 &&
          cdb_arrive(h[0], cmx_forever(), &t) == CDB_RETIRED);
    CHECK(cdb_stage(&b, 1) == CMX_OK && cdb_release(&h[0]) == CMX_OK &&
          cdb_apply(&b) == CDB_RETIRED);
    CHECK(cdb_close(&b) == CMX_OK && cdb_destroy(&b) == CMX_OK);
    CHECK(setup(&k, &b, h, 3, 3));
    b.epoch = UINT32_MAX;
    h[0].epoch = h[1].epoch = UINT32_MAX;
    CHECK(cdb_stage(&b, 1) == CMX_OK && cdb_release(&h[0]) == CMX_OK &&
          cdb_release(&h[1]) == CMX_OK && cdb_apply(&b) == CDB_RETIRED);
    CHECK(cdb_close(&b) == CMX_OK && cdb_destroy(&b) == CMX_OK);
    puts(
        "CDB_GENERATION:zero-refused;one-party;three-round-saturation;config-epoch-retirement;cleanup-at-limits");
    return 0;
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        return 2;
    }
    if (!strcmp(argv[1], "rounds"))
    {
        return rounds();
    }
    if (!strcmp(argv[1], "terminal"))
    {
        return terminal();
    }
    if (!strcmp(argv[1], "membership"))
    {
        return membership();
    }
    if (!strcmp(argv[1], "generation"))
    {
        return generation();
    }
    return 2;
}
