/*
 * Test driver for the clinic laboratory.
 *
 * Usage: tests [containers]   (no argument, or any other one, runs "unique")
 * Exit status: 0 the case passed; 41 a REQUIRE failed (its text goes to
 * stderr after the marker MC_ASSERT:).
 *
 * The allocator below keeps its last block in trace.allocation so that a
 * failing case can free it (clean). With trace.quarantine set, release()
 * only counts the call and keeps the C allocation alive until clean(), which
 * lets a case look at a block whose logical owner has already let go.
 */
#include "clinic.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Allocator state and fault injection (fail: allocate returns NULL). */
struct mc_trace
{
    void *allocation;
    unsigned calls, releases;
    int fail, quarantine;
};
static struct mc_trace trace;

/* Frees the block kept in trace.allocation, if any. */
static void clean(void)
{
    free(trace.allocation);
    trace.allocation = NULL;
}

#define REQUIRE(x)                                 \
    do                                             \
    {                                              \
        if (!(x))                                  \
        {                                          \
            fprintf(stderr, "MC_ASSERT:%s\n", #x); \
            clean();                               \
            return 41;                             \
        }                                          \
    } while (0)

static void *allocate(void *c, size_t size)
{
    struct mc_trace *t = c;
    ++t->calls;
    if (t->fail)
    {
        return NULL;
    }
    t->allocation = malloc(size);
    return t->allocation;
}

static void release(void *c, void *p)
{
    struct mc_trace *t = c;
    ++t->releases;
    if (!t->quarantine)
    {
        free(p);
        t->allocation = NULL;
    }
}

static int unique(void)
{
    struct mc_owner owner = {0};
    int copy = 99;
    int out = 88;
    struct mc_allocator a = {&trace, allocate, release};
    unsigned char bytes[12];
    unsigned char expected[12];
    trace.fail = 1;
    REQUIRE(mc_acquire(&owner, a, 7) == -1 && !owner.value &&
            !trace.allocation);
    trace.fail = 0;
    REQUIRE(mc_acquire(&owner, a, 7) == 0);
    REQUIRE(mc_snapshot(&owner, &copy) == 0 && copy == 7);
    REQUIRE(mc_release(&owner) == 0 && !owner.value && owner.releases == 1 &&
            trace.releases == 1);
    REQUIRE(mc_release(&owner) == -1 && trace.releases == 1);
    REQUIRE(mc_read(&owner, &out) == -1 && out == 88 && copy == 7);
    /* The original abstract trace's last two events are rejected; no invalid C access. */
    puts("MC_TRACE:2=double-release,3=dead-access,actual_releases=1");
    for (size_t i = 0; i < 12; ++i)
    {
        bytes[i] = expected[i] = (unsigned char)i;
    }
    for (size_t i = 0; i < 8; ++i)
    {
        expected[2 + i] = (unsigned char)i;
    }
    REQUIRE(mc_move(bytes, 12, 2, 0, 8) == FD_OK &&
            !memcmp(bytes, expected, 12));
    REQUIRE(mc_move(bytes, 12, 0, 2, 8) == FD_OK);
    for (size_t i = 0; i < 8; ++i)
    {
        REQUIRE(bytes[i] == i);
    }
    trace.quarantine = 1;
    REQUIRE(mc_acquire(&owner, a, 23) == 0);
    {
        int *retained = owner.value;
        REQUIRE(mc_release(&owner) == 0 && mc_read(&owner, &out) == -1 &&
                out == 88);
        REQUIRE(*retained == 23);
    } /* Storage has NOT been freed by this declared allocator. */
    clean();
    puts("MC_UNIQUE_PASS");
    return 0;
}

static int containers(void)
{
    const struct mc_container owner = {1, 17};
    const struct mc_container other = {2, 99};
    const struct mc_first first = {23, 7};
    const struct mc_first foreign = {31, 8};
    int out = 77;
    _Static_assert(offsetof(struct mc_first, first) == 0,
                   "initial member has no preceding padding");
    REQUIRE(mc_member(&owner, &owner.member, &out) == 0 && out == 17);
    REQUIRE(mc_member(&owner, &other.member, &out) == -1 && out == 17);
    REQUIRE(mc_initial(&first, &first.first, &out) == 0 && out == 23);
    REQUIRE(mc_initial(&first, &foreign.first, &out) == -1 && out == 23);
    printf("MC_OFFSETS:ordinary=%zu,initial=%zu,const_preserved=1\n",
           offsetof(struct mc_container, member),
           offsetof(struct mc_first, first));
    puts("MC_CONTAINERS_PASS");
    return 0;
}

int main(int argc, char **argv)
{
    return argc == 2 && !strcmp(argv[1], "containers") ? containers()
                                                       : unique();
}
