#include "once.h"
#ifdef CDO_TARGET
#define CHECK(c)                \
    do                          \
    {                           \
        if (!(c))               \
            cdo_target_stop(1); \
    } while (0)
#define puts(s) cdo_target_puts(s)
#else
#include <stdio.h>
#include <string.h>
#define CHECK(c)                                                \
    do                                                          \
    {                                                           \
        if (!(c))                                               \
        {                                                       \
            fprintf(stderr, "CDO_CHECK:%d:%s\n", __LINE__, #c); \
            return 1;                                           \
        }                                                       \
    } while (0)
#endif
struct cmx_pool
{
    struct cdo_cell cells[3];
    unsigned used[3], calls, drops, fail, invalid;
};

static struct cdo_cell *allocate(void *ctx)
{
    struct cmx_pool *p = ctx;
    p->calls++;
    if (p->calls == p->fail)
    {
        return 0;
    }
    for (unsigned i = 0; i < 3; i++)
    {
        if (!p->used[i])
        {
            p->used[i] = 1;
            return &p->cells[i];
        }
    }
    return 0;
}

static void drop(void *ctx, struct cdo_cell *cell)
{
    struct cmx_pool *p = ctx;
    for (unsigned i = 0; i < 3; i++)
    {
        if (cell == &p->cells[i])
        {
            if (!p->used[i])
            {
                p->invalid++;
            }
            p->used[i] = 0;
            p->drops++;
            return;
        }
    }
    p->invalid++;
}

static int clean(struct cmx_pool *p)
{
    return !p->used[0] && !p->used[1] && !p->used[2] && !p->invalid;
}

static int setup(cmx_kernel *k, struct cd_once *o, struct cmx_pool *p,
                 struct cdo_handle h[3])
{
    *p = (struct cmx_pool){0};
    cmx_init(k, CMX_FIXED);
    for (unsigned i = 0; i < 3; i++)
    {
        if (cmx_task_add(k, 5 + i, 0, 0x1000 + 256 * i, 256) != (int)i)
        {
            return 0;
        }
    }
    if (cdo_init(o, k, allocate, drop, p) != CMX_OK)
    {
        return 0;
    }
    for (unsigned i = 0; i < 3; i++)
    {
        if (cdo_register(o, i, &h[i]) != CMX_OK)
        {
            return 0;
        }
    }
    return 1;
}

static int build(struct cdo_handle owner)
{
    if (cdo_allocate(owner) != CMX_OK)
    {
        return 0;
    }
    for (unsigned i = 0; i < 8; i++)
    {
        if (cdo_write(owner, i, 17 * i + 1) != CMX_OK)
        {
            return 0;
        }
    }
    return cdo_seal(owner) == CMX_OK && cdo_seal(owner) == CMX_OK;
}

static int publication(void)
{
    for (unsigned owner = 0; owner < 3; owner++)
    {
        cmx_kernel k;
        struct cd_once o = {0};
        struct cmx_pool p;
        struct cdo_handle h[3];
        struct cdo_ticket t[3], copy;
        struct cdo_table out = {.count = 999};
        CHECK(setup(&k, &o, &p, h));
        CHECK(cdo_enter(h[owner], cmx_forever(), &t[owner]) == CDO_INIT);
        CHECK(cdo_enter(h[owner], cmx_forever(), &copy) == CMX_DEADLOCK);
        for (unsigned i = 0; i < 3; i++)
        {
            if (i != owner)
            {
                CHECK(cdo_enter(h[i], cmx_forever(), &t[i]) == CMX_BLOCKED);
            }
        }
        CHECK(cdo_allocate(h[owner]) == CMX_OK);
        for (unsigned i = 0; i < 8; i++)
        {
            CHECK(cdo_write(h[owner], i, 17 * i + 1) == CMX_OK);
            CHECK(cdo_read(h[(owner + 1) % 3], &out) == CMX_BLOCKED &&
                  out.count == 999);
        }
        CHECK(cdo_finish(h[owner], 1) == CMX_INVALID);
        CHECK(cdo_seal(h[owner]) == CMX_OK &&
              cdo_read(h[(owner + 1) % 3], &out) == CMX_BLOCKED);
        CHECK(cdo_seal(h[owner]) == CMX_OK &&
              cdo_finish(h[owner], 1) == CMX_OK);
        for (unsigned i = 0; i < 3; i++)
        {
            CHECK(cdo_result(t[i], &out) == CMX_OK && out.count == 8 &&
                  out.checksum == 484);
            for (unsigned j = 0; j < 8; j++)
            {
                CHECK(out.entries[j] == 17 * j + 1);
            }
        }
        CHECK(o.attempts == 1 && k.task[owner].owned_sync == 0);
        CHECK(cdo_enter(h[owner], cmx_forever(), &copy) == CMX_OK);
        CHECK(cdo_result(t[owner], &out) == CDO_STALE &&
              cdo_result(copy, &out) == CMX_OK);
        CHECK(cdo_close(&o) == CMX_OK);
        for (unsigned i = 0; i < 3; i++)
        {
            CHECK(cdo_release(&h[i]) == CMX_OK);
        }
        CHECK(cdo_destroy(&o) == CMX_OK && clean(&p) && p.drops == 3);
    }
    puts(
        "CDO_PUBLICATION:three-owners;eight-entries+count+checksum484;recursive-refusal;two-waiters;old-ticket-refusal;cleanup");
    return 0;
}

static int failure(void)
{
    unsigned cases = 0;
    for (unsigned cut = 0; cut <= 10; cut++)
    {
        cmx_kernel k;
        struct cd_once o = {0};
        struct cmx_pool p;
        struct cdo_handle h[3];
        struct cdo_ticket t[3];
        struct cdo_table out = {.count = 999};
        CHECK(setup(&k, &o, &p, h) &&
              cdo_enter(h[0], cmx_forever(), &t[0]) == CDO_INIT);
        CHECK(cdo_enter(h[1], cmx_forever(), &t[1]) == CMX_BLOCKED &&
              cdo_enter(h[2], cmx_forever(), &t[2]) == CMX_BLOCKED);
        CHECK(cdo_allocate(h[0]) == CMX_OK);
        for (unsigned i = 0; i < cut && i < 8; i++)
        {
            CHECK(cdo_write(h[0], i, 17 * i + 1) == CMX_OK);
        }
        for (unsigned i = 8; i < cut; i++)
        {
            CHECK(cdo_seal(h[0]) == CMX_OK);
        }
        CHECK(cdo_cancel(t[0]) == CMX_OK && clean(&p) && p.drops == 3);
        for (unsigned i = 0; i < 3; i++)
        {
            CHECK(cdo_result(t[i], &out) == CDO_FAILED && out.count == 999);
        }
        CHECK(cdo_enter(h[2], cmx_forever(), &t[2]) == CDO_FAILED &&
              o.attempts == 1 && o.resources == 0);
        CHECK(cdo_close(&o) == CMX_OK);
        for (unsigned i = 0; i < 3; i++)
        {
            CHECK(cdo_release(&h[i]) == CMX_OK);
        }
        CHECK(cdo_destroy(&o) == CMX_OK && clean(&p));
        cases++;
    }
    CHECK(cases == 11);
    puts(
        "CDO_FAILURE:cuts=11;cleanup-before-failed;all-waiters-failed;permanent-attempt1;no-partial-output");
    return 0;
}

static int retry(void)
{
    for (unsigned fail = 1; fail <= 3; fail++)
    {
        cmx_kernel k;
        struct cd_once o = {0};
        struct cmx_pool p;
        struct cdo_handle h[3];
        struct cdo_ticket old, newticket;
        struct cdo_table out = {.count = 999};
        CHECK(setup(&k, &o, &p, h));
        p.fail = fail;
        CHECK(cdo_enter(h[0], cmx_forever(), &old) == CDO_INIT &&
              cdo_allocate(h[0]) == CDO_FAILED);
        CHECK(clean(&p) && p.drops == fail - 1 &&
              cdo_prepare_reset(&o) == CMX_BLOCKED);
        CHECK(cdo_result(old, &out) == CDO_FAILED && out.count == 999 &&
              cdo_prepare_reset(&o) == CMX_OK);
        CHECK(cdo_reset(&o) == CMX_BLOCKED);
        struct cdo_handle stale = h[0];
        for (unsigned i = 0; i < 3; i++)
        {
            CHECK(cdo_release(&h[i]) == CMX_OK);
        }
        CHECK(cdo_reset(&o) == CMX_OK && o.epoch == 2 && clean(&p));
        p.fail = 0;
        for (unsigned i = 0; i < 3; i++)
        {
            CHECK(cdo_register(&o, i, &h[i]) == CMX_OK);
        }
        CHECK(cdo_enter(stale, cmx_forever(), &newticket) == CDO_STALE);
        CHECK(cdo_enter(h[1], cmx_forever(), &newticket) == CDO_INIT &&
              build(h[1]) && cdo_finish(h[1], 1) == CMX_OK);
        CHECK(cdo_result(old, &out) == CDO_STALE &&
              cdo_result(newticket, &out) == CMX_OK && out.checksum == 484 &&
              o.attempts == 2);
        CHECK(cdo_close(&o) == CMX_OK);
        for (unsigned i = 0; i < 3; i++)
        {
            CHECK(cdo_release(&h[i]) == CMX_OK);
        }
        CHECK(cdo_destroy(&o) == CMX_OK && clean(&p) &&
              p.drops == fail - 1 + 3);
    }
    puts(
        "CDO_RETRY:three-allocation-failures;rollback-complete;drain+unregister-before-reset;new-epoch;attempts2-success1");
    return 0;
}

static int waiting(void)
{
    cmx_kernel k;
    struct cd_once o = {0};
    struct cmx_pool p;
    struct cdo_handle h[3];
    struct cdo_ticket t[3];
    struct cdo_table out = {.count = 999};
    CHECK(setup(&k, &o, &p, h) &&
          cdo_enter(h[0], cmx_forever(), &t[0]) == CDO_INIT);
    CHECK(cdo_enter(h[1], cmx_at(5), &t[1]) == CMX_BLOCKED &&
          cdo_enter(h[2], cmx_forever(), &t[2]) == CMX_BLOCKED);
    cmx_tick(&k, 5);
    CHECK(cdo_cancel(t[2]) == CMX_CLOSED);
    CHECK(build(h[0]) && cdo_finish(h[0], 1) == CMX_OK);
    CHECK(cdo_result(t[1], &out) == CMX_TIMEOUT && out.count == 999 &&
          cdo_result(t[2], &out) == CMX_CLOSED && out.count == 999);
    CHECK(cdo_result(t[0], &out) == CMX_OK && out.checksum == 484 &&
          o.attempts == 1 && k.task[0].owned_sync == 0);
    CHECK(cdo_close(&o) == CMX_OK);
    for (unsigned i = 0; i < 3; i++)
    {
        CHECK(cdo_release(&h[i]) == CMX_OK);
    }
    CHECK(cdo_destroy(&o) == CMX_OK && clean(&p));
    puts(
        "CDO_WAITING:timeout+cancel-selected-before-publication;owner-unaffected;completion-collection");
    return 0;
}

static int policies(void)
{
    cmx_kernel k;
    cmx_once raw;
    cmx_init(&k, CMX_FIXED);
    for (unsigned i = 0; i < 3; i++)
    {
        CHECK(cmx_task_add(&k, 5 + i, 0, 0x1000 + 256 * i, 256) == (int)i);
    }
    cmx_once_init(&raw);
    CHECK(cmx_once_enter(&k, &raw, 0, cmx_forever()) == CMX_RETRY);
    CHECK(cmx_once_enter(&k, &raw, 1, cmx_forever()) == CMX_BLOCKED);
    CHECK(cmx_once_finish(&k, &raw, 0, 0) == CMX_OK &&
          k.task[1].result == CMX_RETRY && raw.state == 0);
    CHECK(cmx_once_enter(&k, &raw, 2, cmx_forever()) == CMX_RETRY &&
          raw.owner == 2);
    CHECK(cmx_once_finish(&k, &raw, 2, 1) == CMX_OK);
    for (unsigned retry = 0; retry < 2; retry++)
    {
        struct mb_once bridge = {0};
        unsigned out[2] = {999, 998};
        mb_once_init(&bridge, retry != 0);
        CHECK(mb_once_enter(&bridge, 0, out) == MB_INIT &&
              mb_once_enter(&bridge, 1, out) == MB_BUSY);
        CHECK(mb_once_finish(&bridge, 0, false, 0, 0) == MB_OK);
        CHECK(mb_once_enter(&bridge, 2, out) == (retry ? MB_INIT : MB_FAILED) &&
              out[0] == 999);
        if (retry)
        {
            CHECK(mb_once_finish(&bridge, 2, true, 41, 42) == MB_OK &&
                  mb_once_enter(&bridge, 1, out) == MB_OK && out[1] == 42);
        }
    }
    puts(
        "CDO_POLICIES:CMX-failure-retry-role;MB-permanent-vs-retry-policy;MB-contention-status-distinct");
    return 0;
}

static int limits(void)
{
    cmx_kernel k;
    struct cd_once o = {0};
    struct cmx_pool p;
    struct cdo_handle h[3];
    struct cdo_ticket t;
    struct cdo_table out = {.count = 999};
    CHECK(setup(&k, &o, &p, h));
    o.attempts = UINT32_MAX;
    CHECK(cdo_enter(h[0], cmx_forever(), &t) == CDO_RETIRED &&
          atomic_load(&o.publication.state) == 0);
    CHECK(cdo_close(&o) == CMX_OK);
    for (unsigned i = 0; i < 3; i++)
    {
        CHECK(cdo_release(&h[i]) == CMX_OK);
    }
    CHECK(cdo_destroy(&o) == CMX_OK && clean(&p));
    CHECK(setup(&k, &o, &p, h));
    o.slot[0].serial = UINT32_MAX;
    CHECK(cdo_enter(h[0], cmx_forever(), &t) == CDO_RETIRED);
    CHECK(cdo_enter(h[1], cmx_forever(), &t) == CDO_INIT &&
          cdo_finish(h[1], 0) == CMX_OK && cdo_result(t, &out) == CDO_FAILED);
    o.epoch = UINT32_MAX;
    for (unsigned i = 0; i < 3; i++)
    {
        h[i].epoch = UINT32_MAX;
    }
    CHECK(cdo_prepare_reset(&o) == CMX_OK);
    for (unsigned i = 0; i < 3; i++)
    {
        CHECK(cdo_release(&h[i]) == CMX_OK);
    }
    CHECK(cdo_reset(&o) == CDO_RETIRED && cdo_close(&o) == CMX_OK &&
          cdo_destroy(&o) == CMX_OK && clean(&p));
    puts(
        "CDO_LIMITS:attempt-and-ticket-retirement;reset-epoch-no-wrap;cleanup-legal-at-limits");
    return 0;
}
#ifndef CDO_TARGET
int main(int argc, char **argv)
{
    if (argc != 2)
    {
        return 2;
    }
    if (!strcmp(argv[1], "publication"))
    {
        return publication();
    }
    if (!strcmp(argv[1], "failure"))
    {
        return failure();
    }
    if (!strcmp(argv[1], "retry"))
    {
        return retry();
    }
    if (!strcmp(argv[1], "waiting"))
    {
        return waiting();
    }
    if (!strcmp(argv[1], "policies"))
    {
        return policies();
    }
    if (!strcmp(argv[1], "limits"))
    {
        return limits();
    }
    return 2;
}
#endif
