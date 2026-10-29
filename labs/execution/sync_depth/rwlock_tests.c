#include "rwlock.h"
#include <limits.h>
#ifdef RWD_TARGET
#define CHECK(c)                \
    do                          \
    {                           \
        if (!(c))               \
            rwd_target_stop(1); \
    } while (0)
#define puts(s) rwd_target_puts(s)
#else
#include <stdio.h>
#include <string.h>
#define CHECK(c)                                                \
    do                                                          \
    {                                                           \
        if (!(c))                                               \
        {                                                       \
            fprintf(stderr, "CDR_CHECK:%d:%s\n", __LINE__, #c); \
            return 1;                                           \
        }                                                       \
    } while (0)
#endif
static int rw_setup(cmx_kernel *k, struct cd_rw *r, struct cdr_handle h[4],
                    unsigned limit)
{
    cmx_init(k, CMX_FIXED);
    for (unsigned i = 0; i < 4; i++)
    {
        if (cmx_task_add(k, 5 + i, 0, 0x1000 + 256 * i, 256) != (int)i)
        {
            return 0;
        }
    }
    if (cdr_init(r, k, limit) != CMX_OK)
    {
        return 0;
    }
    for (unsigned i = 0; i < 4; i++)
    {
        if (cdr_register(r, i, &h[i]) != CMX_OK)
        {
            return 0;
        }
    }
    return 1;
}

static int rw_close_all(struct cd_rw *r, struct cdr_handle h[4])
{
    if (cdr_close(r) != CMX_OK)
    {
        return 0;
    }
    for (unsigned i = 0; i < 4; i++)
    {
        if (cdr_release(&h[i]) != CMX_OK)
        {
            return 0;
        }
    }
    return cdr_destroy(r) == CMX_OK;
}

static int rw_policy(void)
{
    cmx_kernel k;
    struct cd_rw r;
    struct cdr_handle h[4];
    struct cdr_ticket a, b, c;
    CHECK(rw_setup(&k, &r, h, 4));
    CHECK(cdr_begin(h[0], CDR_READ, cmx_forever(), &a) == CMX_OK &&
          cdr_result(a) == CMX_OK);
    CHECK(cdr_begin(h[0], CDR_WRITE, cmx_forever(), &b) == CMX_DEADLOCK);
    CHECK(cdr_begin(h[1], CDR_WRITE, cmx_forever(), &b) == CMX_BLOCKED);
    CHECK(cdr_begin(h[2], CDR_READ, cmx_forever(), &c) == CMX_BLOCKED);
    CHECK(cdr_unlock(h[0]) == CMX_OK && r.raw.writer == 1 &&
          r.raw.readers == 0);
    CHECK(cdr_result(b) == CMX_OK && cdr_result(c) == CMX_BLOCKED);
    CHECK(cdr_unlock(h[1]) == CMX_OK && cdr_result(c) == CMX_OK &&
          r.raw.readers == (1u << 2));
    CHECK(cdr_unlock(h[2]) == CMX_OK && rw_close_all(&r, h));
    puts(
        "CDR_POLICY:retained-A-B-C-trace;writer-before-later-reader;upgrade-refusal");
    return 0;
}

static int rw_cancelled(void)
{
    for (unsigned timeout = 0; timeout < 2; timeout++)
    {
        cmx_kernel k;
        struct cd_rw r;
        struct cdr_handle h[4];
        struct cdr_ticket a, b, c, d;
        CHECK(rw_setup(&k, &r, h, 4));
        CHECK(cdr_begin(h[0], CDR_READ, cmx_forever(), &a) == CMX_OK &&
              cdr_result(a) == CMX_OK);
        CHECK(cdr_begin(h[1], CDR_WRITE, timeout ? cmx_at(5) : cmx_forever(),
                        &b) == CMX_BLOCKED);
        CHECK(cdr_begin(h[2], CDR_READ, cmx_forever(), &c) == CMX_BLOCKED);
        CHECK(cdr_begin(h[3], CDR_WRITE, cmx_forever(), &d) == CMX_BLOCKED);
        if (timeout)
        {
            cmx_tick(&k, 5);
        }
        else
        {
            CHECK(cdr_cancel(b) == CMX_CLOSED);
        }
        CHECK(cdr_result(b) == (timeout ? CMX_TIMEOUT : CMX_CLOSED) &&
              cdr_result(c) == CMX_BLOCKED);
        CHECK(cdr_cancel(d) == CMX_CLOSED && cdr_result(d) == CMX_CLOSED);
        CHECK(cdr_result(c) == CMX_OK && r.raw.readers == 5);
        CHECK(cdr_unlock(h[0]) == CMX_OK && cdr_unlock(h[2]) == CMX_OK &&
              rw_close_all(&r, h));
    }
    puts(
        "CDR_CANCEL:timeout+cancel;remaining-writer-gate;last-writer-removal-read-admission");
    return 0;
}

static int rw_bounds(void)
{
    cmx_kernel k;
    struct cd_rw r;
    struct cdr_handle h[4];
    struct cdr_ticket a, b, c;
    CHECK(rw_setup(&k, &r, h, 2));
    CHECK(cdr_begin(h[0], CDR_READ, cmx_forever(), &a) == CMX_OK &&
          cdr_result(a) == CMX_OK);
    CHECK(cdr_begin(h[1], CDR_READ, cmx_forever(), &b) == CMX_OK &&
          cdr_result(b) == CMX_OK);
    CHECK(cdr_begin(h[2], CDR_READ, cmx_forever(), &c) == CMX_FULL &&
          r.raw.readers == 3 && k.task[2].owned_sync == 0);
    CHECK(cdr_unlock(h[3]) == CMX_NOT_OWNER && r.raw.readers == 3);
    CHECK(cdr_unlock(h[0]) == CMX_OK &&
          cdr_begin(h[2], CDR_READ, cmx_forever(), &c) == CMX_OK &&
          cdr_result(c) == CMX_OK);
    CHECK(cdr_unlock(h[1]) == CMX_OK && cdr_unlock(h[2]) == CMX_OK);
    k.task[3].owned_sync = UINT_MAX;
    c = (struct cdr_ticket){h[2], 17};
    CHECK(cdr_begin(h[3], CDR_READ, cmx_forever(), &c) == CMX_FULL &&
          r.raw.readers == 0 && r.raw.writer == CMX_NONE &&
          c.handle.task == 2 && c.serial == 17 &&
          k.task[3].owned_sync == UINT_MAX && r.slot[3].phase == CDR_IDLE);
    k.task[3].owned_sync = 0;
    r.slot[0].serial = UINT32_MAX;
    CHECK(cdr_begin(h[0], CDR_WRITE, cmx_forever(), &a) == CDR_RETIRED &&
          rw_close_all(&r, h));
    puts(
        "CDR_BOUNDS:two-reader-reservation-limit;overflow-no-mutation;nonowner-refusal;operation-retirement-cleanup");
    return 0;
}

static int rw_starvation(void)
{
    cmx_kernel k;
    struct cd_rw r;
    struct cdr_handle h[4];
    struct cdr_ticket read, writer, next;
    CHECK(rw_setup(&k, &r, h, 4));
    CHECK(cdr_begin(h[0], CDR_READ, cmx_forever(), &read) == CMX_OK &&
          cdr_result(read) == CMX_OK);
    CHECK(cdr_begin(h[1], CDR_WRITE, cmx_forever(), &writer) == CMX_BLOCKED);
    unsigned held = 0, handoffs = 0, progress = 0;
    for (unsigned step = 0; step < 64; step++)
    {
        unsigned incoming = held == 0 ? 2 : 0;
        int admission = cdr_begin(h[incoming], CDR_READ, cmx_forever(), &next);
        if (admission == CMX_BLOCKED)
        {
            CHECK(cdr_unlock(h[held]) == CMX_OK &&
                  cdr_result(writer) == CMX_OK && r.raw.writer == 1);
            CHECK(cdr_unlock(h[1]) == CMX_OK && cdr_result(next) == CMX_OK &&
                  cdr_unlock(h[incoming]) == CMX_OK);
            progress = 1;
            break;
        }
        CHECK(admission == CMX_OK && cdr_result(next) == CMX_OK &&
              cdr_unlock(h[held]) == CMX_OK);
        held = incoming;
        handoffs++;
        CHECK(cdr_result(writer) == CMX_BLOCKED && r.raw.writer == CMX_NONE);
    }
#ifndef RWD_TARGET
    printf("CDR_STARVATION_TRACE:reader-handoffs=%u;writer-progress=%u\n",
           handoffs, progress);
#else
    puts(progress
             ? "CDR_STARVATION_TRACE:reader-handoffs=0;writer-progress=1"
             : "CDR_STARVATION_TRACE:reader-handoffs=64;writer-progress=0");
#endif
    CHECK(progress && handoffs == 0 && rw_close_all(&r, h));
    puts(
        "CDR_STARVATION:pending-writer-closes-reader-gate;first-cohort-drain-admits-writer");
    return 0;
}
#ifndef RWD_TARGET
int main(int argc, char **argv)
{
    if (argc != 2)
    {
        return 2;
    }
    if (!strcmp(argv[1], "policy"))
    {
        return rw_policy();
    }
    if (!strcmp(argv[1], "cancel"))
    {
        return rw_cancelled();
    }
    if (!strcmp(argv[1], "bounds"))
    {
        return rw_bounds();
    }
    if (!strcmp(argv[1], "starvation"))
    {
        return rw_starvation();
    }
    return 2;
}

#endif
