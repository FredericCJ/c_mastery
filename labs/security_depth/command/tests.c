#include "command.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(x)                                   \
    do                                             \
    {                                              \
        if (!(x))                                  \
        {                                          \
            fprintf(stderr, "SC_ASSERT:%s\n", #x); \
            exit(41);                              \
        }                                          \
    } while (0)

static void base(void)
{
    unsigned char p[11] = {1, 2, 0, 1, 0, 2};
    struct sc_batch b;
    struct sc_meter m;
    struct sc_authority a = {1, 0};
    unsigned count, n, op, i;
    unsigned tests = 0;
    CHECK(sc_decode(p, 6, &a, 0, &b, &m) == SC_OK);
    CHECK(b.count == 1 && b.commands[0].values[0] == 1 &&
          b.commands[0].values[1] == 2);
    for (op = 0; op < 5; ++op)
    {
        for (count = 0; count < 6; ++count)
        {
            for (n = 0; n <= 11; ++n)
            {
                struct sc_batch old;
                enum sc_result expected;
                p[0] = (unsigned char)op;
                p[1] = (unsigned char)count;
                for (i = 2; i < 11; ++i)
                {
                    p[i] = (unsigned char)i;
                }
                memset(&b, 0x5a, sizeof b);
                old = b;
                expected = n < 2                  ? SC_LENGTH
                           : (op != 1 && op != 2) ? SC_OPCODE
                           : count > 4            ? SC_COUNT
                           : n != 2 + 2 * count   ? SC_LENGTH
                           : op == 2              ? SC_DENIED
                                                  : SC_OK;
                CHECK(sc_decode(p, n, &a, 0, &b, &m) == expected);
                if (expected != SC_OK)
                {
                    CHECK(!memcmp(&b, &old, sizeof b));
                }
                CHECK(m.values <= 4 && m.headers <= 1);
                ++tests;
            }
        }
    }
    printf("OBS base cases=%u exact_grammar=1 failure_preserved=1\n", tests);
}

static struct sc_batch returned_copy(void)
{
    unsigned char p[] = {1, 1, 0, 42};
    struct sc_batch b;
    struct sc_meter m;
    struct sc_authority a = {1, 0};
    CHECK(sc_decode(p, sizeof p, &a, 0, &b, &m) == SC_OK);
    return b;
}

static void authority(void)
{
    unsigned char p[] = {2, 0};
    struct sc_batch b, accepted;
    struct sc_meter m;
    struct sc_authority a = {1, 0};
    uint32_t ordinary = 0, protected = 7;
    b = returned_copy();
    CHECK(sc_apply(&b, &a, &ordinary, &protected) == SC_OK && ordinary == 42);
    CHECK(sc_flawed_authority(p, 2, &protected) == SC_OK && protected == 8);
    CHECK(sc_decode(p, 2, &a, 0, &b, &m) == SC_DENIED);
    a.allowed = 1;
    CHECK(sc_decode(p, 2, &a, 0, &b, &m) == SC_OK);
    accepted = b;
    p[0] = 99;
    CHECK(sc_apply(&accepted, &a, &ordinary, &protected) == SC_OK &&
          protected == 9);
    b = accepted;
    b.commands[0].count = 5;
    CHECK(sc_apply(&b, &a, &ordinary, &protected) == SC_INVALID &&
          protected == 9);
    b = accepted;
    b.work = 0;
    CHECK(sc_apply(&b, &a, &ordinary, &protected) == SC_INVALID &&
          protected == 9);
    CHECK(sc_revoke(&a) == SC_OK);
    CHECK(sc_apply(&accepted, &a, &ordinary, &protected) == SC_STALE &&
          protected == 9);
    a.allowed = 1;
    CHECK(sc_apply(&accepted, &a, &ordinary, &protected) == SC_STALE);
    a.generation = UINT_MAX - 1;
    CHECK(sc_revoke(&a) == SC_OK && a.generation == UINT_MAX);
    a.allowed = 1;
    p[0] = 2;
    CHECK(sc_decode(p, 2, &a, 0, &b, &m) == SC_DENIED);
    CHECK(sc_revoke(&a) == SC_EXHAUSTED && a.generation == UINT_MAX &&
          !a.allowed);
    /* Same-process arbitrary writes can forge ordinary public fields; explicit
     * limit. */
    a = (struct sc_authority){5, 1};
    accepted.commands[0].generation = 5;
    CHECK(sc_apply(&accepted, &a, &ordinary, &protected) == SC_OK &&
          protected == 10);
    puts("OBS authority flawed_sentinel_increment=1 revocation=stale "
         "finite_generation=terminal shared_memory_forgery=residual");
}

static void extensions(void)
{
    struct sc_authority a = {1, 0};
    struct sc_batch b;
    struct sc_meter m;
    unsigned char role[] = {4, 0, 255, 0},
                  option[] = {4, 1, 255, 1, 0, 9, 0, 7},
                  nested[] = {3, 2, 4, 1, 1, 0, 4, 4, 1, 1, 0, 7},
                  deep[] = {3, 1, 6, 3, 1, 3, 3, 1, 0},
                  depth_ok[] = {3, 1, 5, 3, 1, 2, 1, 0},
                  expand[] = {5, 255, 0, 8};
    CHECK(sc_decode(role, sizeof role, &a, 1, &b, &m) == SC_DENIED);
    a.allowed = 1;
    CHECK(sc_decode(option, sizeof option, &a, 1, &b, &m) == SC_OK &&
          b.work == 2 && b.commands[0].values[0] == 9);
    CHECK(sc_decode(option, sizeof option - 1, &a, 1, &b, &m) == SC_LENGTH);
    CHECK(sc_decode(option, sizeof option, &a, 0, &b, &m) == SC_OPCODE);
    CHECK(sc_decode(nested, sizeof nested, &a, 1, &b, &m) == SC_OK &&
          b.count == 2 && b.work == 2 && m.values == 2);
    CHECK(sc_decode(deep, sizeof deep, &a, 1, &b, &m) == SC_BUDGET);
    CHECK(sc_decode(depth_ok, sizeof depth_ok, &a, 1, &b, &m) == SC_OK &&
          b.count == 1);
    CHECK(sc_decode(expand, 4, &a, 1, &b, &m) == SC_BUDGET && m.values == 0);
    expand[1] = 4;
    CHECK(sc_decode(expand, 4, &a, 1, &b, &m) == SC_OK && b.work == 4 &&
          b.commands[0].values[3] == 8);
    puts("OBS extensions role_untrusted=1 optional_exact_length=1 "
         "depth_and_decoded_budget=1");
}

static void arithmetic(void)
{
    static const char *bad[] = {
        "5", "-1", "+1", "01", "1 ", "", "99999999999999999999999999"};
    unsigned v = 99;
    char c[2] = {'X', 0};
    size_t out = 77, i;
    CHECK(sc_decimal((const unsigned char *)"0", 1, 0, &v, c) == SC_OK &&
          v == 0 && !strcmp(c, "0"));
    CHECK(sc_decimal((const unsigned char *)"4", 1, 0, &v, c) == SC_OK &&
          v == 4);
    for (i = 0; i < sizeof bad / sizeof bad[0]; ++i)
    {
        v = 99;
        c[0] = 'X';
        CHECK(sc_decimal((const unsigned char *)bad[i], strlen(bad[i]), 0, &v,
                         c) != SC_OK &&
              v == 99 && c[0] == 'X');
    }
    CHECK(sc_decimal((const unsigned char *)"0004", 4, 1, &v, c) == SC_OK &&
          v == 4 && !strcmp(c, "4"));
    CHECK(sc_extent(1, SIZE_MAX, 2, SIZE_MAX, &out) == SC_COUNT && out == 77);
    CHECK(sc_extent(3, 2, 4, 11, &out) == SC_OK && out == 11);
    CHECK(sc_extent(SIZE_MAX, 0, 2, SIZE_MAX, &out) == SC_OK &&
          out == SIZE_MAX);
    CHECK((uint8_t)(128u * 2u) == 0 && 128u * 2u == 256u);
    puts("OBS arithmetic bounded_wrap=0_vs_256 canonical_decimal=1 "
         "extent_before_product=1");
}

int main(int argc, char **argv)
{
    CHECK(argc == 2);
    if (!strcmp(argv[1], "base"))
    {
        base();
    }
    else if (!strcmp(argv[1], "authority"))
    {
        authority();
    }
    else if (!strcmp(argv[1], "extensions"))
    {
        extensions();
    }
    else if (!strcmp(argv[1], "arithmetic"))
    {
        arithmetic();
    }
    else
    {
        CHECK(0);
    }
    return 0;
}
