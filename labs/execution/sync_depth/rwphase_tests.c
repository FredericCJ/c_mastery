#include "rwphase.h"
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
            fprintf(stderr, "CPF_CHECK:%d:%s\n", __LINE__, #c); \
            return 1;                                           \
        }                                                       \
    } while (0)
#endif
static int pf_setup(cmx_kernel *k, struct cmx_cpf *o, struct cpf_handle h[6])
{
    cmx_init(k, CMX_FIXED);
    for (unsigned i = 0; i < 6; i++)
        if (cmx_task_add(k, 5 + i, 0, 0x1000 + 256 * i, 256) != (int)i)
            return 0;
    if (cpf_init(o, k) != CMX_OK)
        return 0;
    for (unsigned i = 0; i < 6; i++)
        if (cpf_register(o, i, &h[i]) != CMX_OK)
            return 0;
    return 1;
}

static int pf_close_all(struct cmx_cpf *o, struct cpf_handle h[6])
{
    if (cpf_close(o) != CMX_OK)
        return 0;
    for (unsigned i = 0; i < 6; i++)
        if (cpf_release(&h[i]) != CMX_OK)
            return 0;
    return cpf_destroy(o) == CMX_OK;
}

static int pf_phases(void)
{
    cmx_kernel k;
    struct cmx_cpf o;
    struct cpf_handle h[6];
    struct cpf_ticket t[6], old;
    CHECK(pf_setup(&k, &o, h));
    CHECK(cpf_begin(h[0], CPF_READ, cmx_forever(), &t[0]) == CMX_OK &&
          cpf_result(t[0]) == CMX_OK);
    CHECK(cpf_begin(h[1], CPF_WRITE, cmx_forever(), &t[1]) == CMX_BLOCKED);
    CHECK(cpf_begin(h[2], CPF_READ, cmx_forever(), &t[2]) == CMX_BLOCKED);
    CHECK(cpf_begin(h[3], CPF_WRITE, cmx_forever(), &t[3]) == CMX_BLOCKED);
    CHECK(cpf_begin(h[4], CPF_READ, cmx_forever(), &t[4]) == CMX_BLOCKED);
    CHECK(cpf_unlock(h[0]) == CMX_OK && o.writer == 1 &&
          cpf_result(t[1]) == CMX_OK);
    CHECK(cpf_unlock(h[1]) == CMX_OK && o.readers == 20 &&
          o.writer == CMX_NONE);
    CHECK(cpf_result(t[2]) == CMX_OK && cpf_result(t[4]) == CMX_OK &&
          cpf_result(t[3]) == CMX_BLOCKED);
    /* New reader5 cannot extend a phase across already pending writer3. */
    CHECK(cpf_begin(h[5], CPF_READ, cmx_forever(), &t[5]) == CMX_BLOCKED);
    CHECK(cpf_unlock(h[2]) == CMX_OK && cpf_unlock(h[4]) == CMX_OK &&
          o.writer == 3);
    CHECK(cpf_result(t[3]) == CMX_OK && cpf_unlock(h[3]) == CMX_OK &&
          o.readers == 32);
    CHECK(cpf_result(t[5]) == CMX_OK && cpf_unlock(h[5]) == CMX_OK &&
          o.phases == 5);
    old = t[5];
    CHECK(cpf_begin(h[5], CPF_READ, cmx_forever(), &t[5]) == CMX_OK);
    CHECK(cpf_result(old) == CPF_STALE && cpf_result(t[5]) == CMX_OK &&
          cpf_unlock(h[5]) == CMX_OK);
    CHECK(pf_close_all(&o, h));
    puts(
        "CPF_PHASES:R-W-Rbatch-W-R;FIFO-writers;all-current-readers;no-late-reader-barge;ticket-identity");
    return 0;
}

static int pf_upgrade(void)
{
    cmx_kernel k;
    struct cmx_cpf o;
    struct cpf_handle h[6];
    struct cpf_ticket a, b, u, w, unused;
    CHECK(pf_setup(&k, &o, h));
    CHECK(cpf_begin(h[0], CPF_READ, cmx_forever(), &a) == CMX_OK &&
          cpf_result(a) == CMX_OK);
    CHECK(cpf_begin(h[1], CPF_READ, cmx_forever(), &b) == CMX_OK &&
          cpf_result(b) == CMX_OK);
    CHECK(cpf_upgrade(h[0], cmx_forever(), &u) == CMX_BLOCKED &&
          o.upgrader == 0 && o.readers == 3);
    CHECK(cpf_upgrade(h[1], cmx_forever(), &unused) == CMX_DEADLOCK &&
          o.upgrader == 0);
    CHECK(cpf_begin(h[2], CPF_WRITE, cmx_forever(), &w) == CMX_BLOCKED);
    CHECK(cpf_unlock(h[1]) == CMX_OK && o.writer == 0 && o.readers == 0 &&
          k.task[0].owned_sync == 1);
    CHECK(cpf_result(u) == CMX_OK && cpf_unlock(h[0]) == CMX_OK &&
          o.writer == 2);
    CHECK(cpf_result(w) == CMX_OK && cpf_unlock(h[2]) == CMX_OK);
    CHECK(cpf_begin(h[0], CPF_READ, cmx_forever(), &a) == CMX_OK &&
          cpf_result(a) == CMX_OK);
    CHECK(cpf_begin(h[1], CPF_WRITE, cmx_forever(), &w) == CMX_BLOCKED);
    CHECK(cpf_upgrade(h[0], cmx_forever(), &unused) == CMX_DEADLOCK &&
          o.readers == 1);
    CHECK(cpf_unlock(h[0]) == CMX_OK && cpf_result(w) == CMX_OK &&
          cpf_unlock(h[1]) == CMX_OK);
    CHECK(pf_close_all(&o, h));
    puts(
        "CPF_UPGRADE:single-token;retained-read;no-earlier-writer;atomic-conversion;owned-count1");
    return 0;
}

static int pf_cancelled(void)
{
    for (unsigned timeout = 0; timeout < 2; timeout++)
    {
        cmx_kernel k;
        struct cmx_cpf o;
        struct cpf_handle h[6];
        struct cpf_ticket a, b, u, r;
        CHECK(pf_setup(&k, &o, h));
        CHECK(cpf_begin(h[0], CPF_READ, cmx_forever(), &a) == CMX_OK &&
              cpf_result(a) == CMX_OK);
        CHECK(cpf_begin(h[1], CPF_READ, cmx_forever(), &b) == CMX_OK &&
              cpf_result(b) == CMX_OK);
        CHECK(cpf_upgrade(h[0], timeout ? cmx_at(5) : cmx_forever(), &u) ==
              CMX_BLOCKED);
        CHECK(cpf_begin(h[2], CPF_READ, cmx_forever(), &r) == CMX_BLOCKED);
        if (timeout)
            cmx_tick(&k, 5);
        else
            CHECK(cpf_cancel(u) == CMX_CLOSED);
        CHECK(cpf_result(u) == (timeout ? CMX_TIMEOUT : CMX_CLOSED) &&
              o.upgrader == CMX_NONE && o.readers == 7);
        CHECK(cpf_result(r) == CMX_OK && k.task[0].owned_sync == 1);
        for (unsigned i = 0; i < 3; i++)
            CHECK(cpf_unlock(h[i]) == CMX_OK);
        CHECK(pf_close_all(&o, h));
    }
    puts(
        "CPF_CANCEL:upgrade-timeout+cancel-retain-read;gate-reopens;no-owner-loss");
    return 0;
}

static int pf_boundaries(void)
{
    cmx_kernel k;
    struct cmx_cpf o;
    struct cpf_handle h[6];
    struct cpf_ticket a, b;
    CHECK(pf_setup(&k, &o, h));
    CHECK(cpf_begin(h[0], CPF_READ, cmx_at(0), &a) == CMX_OK &&
          cpf_result(a) == CMX_OK);
    CHECK(cpf_upgrade(h[0], cmx_at(0), &b) == CMX_OK &&
          cpf_result(b) == CMX_OK && o.writer == 0);
    CHECK(cpf_unlock(h[1]) == CMX_NOT_OWNER && cpf_close(&o) == CMX_NOT_OWNER);
    CHECK(cpf_unlock(h[0]) == CMX_OK);
    CHECK(cmx_sleep(&k, 0, cmx_forever()) == CMX_BLOCKED &&
          cpf_begin(h[0], CPF_READ, cmx_forever(), &a) == CMX_INVALID);
    CHECK(cmx_cancel(&k, 0) == CMX_OK);
    k.task[1].owned_sync = UINT_MAX;
    b = (struct cpf_ticket){h[2], 17};
    CHECK(cpf_begin(h[1], CPF_WRITE, cmx_forever(), &b) == CMX_FULL &&
          o.writer == CMX_NONE && o.readers == 0 && b.handle.task == 2 &&
          b.serial == 17 && o.slot[1].phase == CPF_IDLE);
    k.task[1].owned_sync = 0;
    o.slot[1].serial = UINT32_MAX;
    CHECK(cpf_begin(h[1], CPF_READ, cmx_forever(), &b) == CPF_RETIRED &&
          b.serial == 17);
    o.admissions = UINT32_MAX;
    CHECK(cpf_begin(h[0], CPF_READ, cmx_forever(), &a) == CPF_RETIRED &&
          pf_close_all(&o, h));
    puts(
        "CPF_BOUNDARIES:immediate-expired-deadline;single-reader-upgrade;nonowner;unrelated-wait-refusal;retirement-cleanup");
    return 0;
}
#ifndef RWD_TARGET
int main(int argc, char **argv)
{
    if (argc != 2)
        return 2;
    if (!strcmp(argv[1], "phases"))
        return pf_phases();
    if (!strcmp(argv[1], "upgrade"))
        return pf_upgrade();
    if (!strcmp(argv[1], "cancel"))
        return pf_cancelled();
    if (!strcmp(argv[1], "boundaries"))
        return pf_boundaries();
    return 2;
}

#endif
