#include "protocol_follow.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define C(x)                                                   \
    do                                                         \
    {                                                          \
        if (!(x))                                              \
        {                                                      \
            fprintf(stderr, "PF_CHECK:%s:%d\n", #x, __LINE__); \
            exit(1);                                           \
        }                                                      \
    } while (0)

struct pf_env
{
    unsigned allocations, frees, fail, busy, stop, feeds;
    void *live;
};

static void *allocate(void *ctx, size_t n)
{
    struct pf_env *e = ctx;
    if (e->fail)
    {
        return 0;
    }
    C(!e->live);
    e->live = malloc(n);
    C(e->live);
    ++e->allocations;
    return e->live;
}

static void release(void *ctx, void *p)
{
    struct pf_env *e = ctx;
    C(e->live == p);
    free(p);
    e->live = 0;
    ++e->frees;
}

static uint32_t read_reg(void *ctx, unsigned reg)
{
    struct pf_env *e = ctx;
    return reg == FW_DMA ? e->busy : FW_TXE;
}

static void write_reg(void *ctx, unsigned reg, uint32_t v)
{
    struct pf_env *e = ctx;
    if (reg == FW_DMA)
    {
        if (v)
        {
            e->busy = 1;
        }
        else
        {
            ++e->stop;
        }
    }
    if (reg == FW_WATCHDOG)
    {
        ++e->feeds;
    }
}

static int driver(void)
{
    for (unsigned fail = 0; fail < 2; ++fail)
    {
        struct pf_env e = {0};
        struct pf_driver d;
        struct fw_io io = {&e, read_reg, write_reg};
        C(pf_driver_init(&d, io, 7, release, &e) == PT_OK);
        unsigned char *p = allocate(&e, 4);
        struct pf_recovery r;
        C(pf_driver_start(&d, &p, 4, fail, &r) == (fail ? PT_RECOVERY : PT_OK));
        C(!p && d.owned == e.live);
        struct pf_recovery stale = r;
        C(pf_driver_reset(&r) == PT_PENDING && d.owned && d.dma.state == 2);
        e.busy =
            0; /* new reset-status says idle, old bus epoch not yet proven */
        C(pf_driver_recover(&r, 8, 1) == PT_STATE && e.frees == 0);
        C(pf_driver_recover(&r, 7, 0) == PT_PENDING && e.frees == 0);
        C(pf_driver_recover(&r, 7, 1) == PT_OK && e.frees == 1 && !r.owner &&
          d.epoch == 8);
        C(pf_driver_recover(&stale, 7, 1) == PT_STATE);
        C(pf_driver_ready(&d) == PT_OK);
        p = allocate(&e, 4);
        C(pf_driver_start(&d, &p, 4, 0, &r) == PT_OK);
        struct pf_recovery duplicate = r;
        C(pf_driver_complete(&r, 8, 0) == PT_PENDING);
        e.busy = 0;
        C(pf_driver_complete(&r, 8, 1) == PT_OK && e.feeds == 1 &&
          e.frees == 2);
        p = allocate(&e, 4);
        C(pf_driver_start(&d, &p, 4, 0, &r) == PT_OK);
        e.busy = 0;
        C(pf_driver_complete(&duplicate, 8, 1) == PT_STATE && e.frees == 2);
        C(pf_driver_complete(&r, 8, 1) == PT_OK && e.frees == 3 &&
          e.feeds == 2);
    }
    struct pf_env e = {0};
    struct pf_driver d;
    struct fw_io io = {&e, read_reg, write_reg};
    C(pf_driver_init(&d, io, UINT32_MAX, release, &e) == PT_OK);
    unsigned char *p = allocate(&e, 1);
    struct pf_recovery r;
    C(pf_driver_start(&d, &p, 1, 0, &r) == PT_OK);
    C(pf_driver_reset(&r) == PT_PENDING);
    e.busy = 0;
    C(pf_driver_recover(&r, UINT32_MAX, 1) == PT_OK && e.frees == 1 &&
      d.reset_pending == 2);
    C(pf_driver_ready(&d) == PT_STATE);
    return 0;
}

static int builder(void)
{
    for (unsigned mode = 0; mode < 2; ++mode)
    {
        for (unsigned address = 0; address < 256; address += 4)
        {
            for (unsigned length = 1; length <= 64; ++length)
            {
                struct pf_builder b = {0};
                struct pf_config out = {.base = {91, 92, 93}};
                C(pt_builder_set(&b.base, 0, address) == PT_OK &&
                  pt_builder_set(&b.base, 1, length) == PT_OK &&
                  pt_builder_set(&b.base, 2, 10) == PT_OK);
                C(pf_builder_mode(&b, mode, mode) == PT_OK);
                int valid = address + length <= 256 &&
                            (!mode || (!(address & 15) && !(length & 15)));
                C(pf_builder_finish(&b, &out) == (valid ? PT_OK : PT_RANGE));
                if (!valid)
                {
                    C(out.base.address == 91);
                }
            }
        }
    }
    struct pf_builder b = {0};
    C(pf_builder_mode(&b, PF_POLL, 1) == PT_RANGE && !b.mode_set);
    C(pf_builder_mode(&b, PF_DMA, 1) == PT_OK);
    C(pt_builder_set(&b.base, 0, 16) == PT_OK &&
      pt_builder_set(&b.base, 1, 16) == PT_OK &&
      pt_builder_set(&b.base, 2, 10) == PT_OK);
    struct pf_builder old = b;
    C(pf_builder_replace(&b, 0, 20) == PT_RANGE && !memcmp(&b, &old, sizeof b));
    C(pf_builder_replace(&b, 0, 32) == PT_OK && b.base.address == 32);
    C(pf_builder_replace(&b, 2, 0) == PT_RANGE && b.base.timeout == 10);
    return 0;
}

static int session(void)
{
    for (unsigned retries = 0; retries < 5; ++retries)
    {
        struct pf_session s = {.next = 1};
        C(pf_session(&s, PF_SEND, 1, 7) == PT_OK);
        C(pf_session(&s, PF_APPLY, 1, 7) == PT_OK && s.total == 7);
        C(pf_session(&s, PF_CRASH, 0, 0) == PT_OK);
        C(pf_session(&s, PF_ACK, 1, 0) == PT_PENDING);
        C(pf_session(&s, PF_RECONNECT, 0, 0) == PT_OK);
        for (unsigned i = 0; i <= retries; ++i)
        {
            C(pf_session(&s, PF_RETRY, 1, 0) == PT_OK);
            C(pf_session(&s, PF_APPLY, 1, 7) == PT_OK && s.total == 7);
        }
        struct pf_session old = s;
        C(pf_session(&s, PF_ACK, 2, 0) == PT_STATE &&
          !memcmp(&s, &old, sizeof s));
        C(pf_session(&s, PF_ACK, 1, 0) == PT_OK && s.next == 2);
        C(pf_session(&s, PF_SEND, 2, -3) == PT_OK);
        C(pf_session(&s, PF_APPLY, 2, -3) == PT_OK && s.total == 4);
        C(pf_session(&s, PF_CANCEL, 0, 0) == PT_OK);
        C(pf_session(&s, PF_ACK, 2, 0) == PT_STATE);
    }
    struct pf_session s = {.next = 1, .total = INT32_MAX};
    C(pf_session(&s, PF_SEND, 1, 1) == PT_OK);
    struct pf_session old = s;
    C(pf_session(&s, PF_APPLY, 1, 1) == PT_RANGE &&
      !memcmp(&s, &old, sizeof s));
    C(pf_session(&s, PF_CANCEL, 0, 0) == PT_OK);
    return 0;
}

static int decoder(void)
{
    for (unsigned n = 0; n <= 8; ++n)
    {
        for (unsigned cut = 0; cut <= n + 2; ++cut)
        {
            struct pf_env e = {0};
            struct pf_decoder d;
            C(pf_decoder_init(&d, 100, allocate, release, &e) == PT_OK);
            unsigned char *out = (unsigned char *)(void *)&d;
            size_t length = 99;
            unsigned char sum = 0;
            for (unsigned i = 0; i < n; ++i)
            {
                sum = (unsigned char)(sum + i + 1);
            }
            for (unsigned i = 0; i < cut; ++i)
            {
                C(pf_decoder_step(&d, 0, 99, i, &out, &length) == PT_PENDING);
                unsigned char v = i == 0 ? (unsigned char)n
                                         : (i <= n ? (unsigned char)i : sum);
                int want = i == n + 1 ? PT_DONE : PT_PENDING;
                C(pf_decoder_step(&d, 1, v, i, &out, &length) == want);
            }
            if (cut == n + 2)
            {
                C(length == n && out == e.live);
                C(pf_decoder_cancel(&d) == PT_DONE && e.frees == 0);
                release(&e, out);
            }
            else
            {
                C(pf_decoder_cancel(&d) == PT_DONE);
                C(e.frees == e.allocations);
                C(pf_decoder_cancel(&d) == PT_DONE && e.frees == e.allocations);
            }
        }
    }
    struct pf_env e = {0};
    struct pf_decoder d;
    C(pf_decoder_init(&d, 3, allocate, release, &e) == PT_OK);
    unsigned char *out = 0;
    size_t length = 99;
    C(pf_decoder_step(&d, 1, 1, 0, &out, &length) == PT_PENDING);
    C(pf_decoder_step(&d, 1, 7, 1, &out, &length) == PT_PENDING);
    C(pf_decoder_step(&d, 1, 7, 3, &out, &length) == PT_IO && !out &&
      length == 99 && e.frees == 1);
    e = (struct pf_env){.fail = 1};
    C(pf_decoder_init(&d, 9, allocate, release, &e) == PT_OK);
    C(pf_decoder_step(&d, 1, 3, 0, &out, &length) == PT_RANGE && !d.header_pc);
    e.fail = 0;
    C(pf_decoder_step(&d, 1, 0, 0, &out, &length) == PT_PENDING);
    C(pf_decoder_step(&d, 1, 1, 1, &out, &length) == PT_IO && e.frees == 1);
    return 0;
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        return 2;
    }
#define RUN(name)                   \
    if (!strcmp(argv[1], #name))    \
    {                               \
        int r = name();             \
        if (!r)                     \
            puts("PF_PASS:" #name); \
        return r;                   \
    }
    RUN(driver);
    RUN(builder);
    RUN(session);
    RUN(decoder);
    return 2;
}
