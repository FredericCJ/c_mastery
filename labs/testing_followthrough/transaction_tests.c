#include "transaction.h"
#include <stdio.h>
#include <string.h>
#define CHECK(x)                                               \
    do                                                         \
    {                                                          \
        if (!(x))                                              \
        {                                                      \
            fprintf(stderr, "TX_CHECK:%d:%s\n", __LINE__, #x); \
            return 1;                                          \
        }                                                      \
    } while (0)

struct cv_context
{
    int fail;
    unsigned acquired[4], released[4], live[31];
    size_t na, nr, calls, bytes;
    long script[8];
    size_t sn;
    int cancel, reenter;
    struct cv_tx *tx;
    unsigned reentry_status;
    unsigned char sent[16];
    int invalid;
};

static int acquire(void *v, unsigned id)
{
    struct cv_context *c = v;
    if ((int)c->na == c->fail)
    {
        return 0;
    }
    c->acquired[c->na++] = id;
    c->live[id]++;
    return 1;
}

static void release(void *v, unsigned id)
{
    struct cv_context *c = v;
    if (id > 30 || !c->live[id] || c->nr == 4)
    {
        c->invalid = 1;
        return;
    }
    c->live[id]--;
    c->released[c->nr++] = id;
}

static long writer(void *v, const unsigned char *p, size_t n)
{
    struct cv_context *c = v;
    if (c->reenter)
    {
        struct tx_owner dummy = {{88}, 1};
        c->reentry_status = tx_run(c->tx, 0, p, n, &dummy).status;
        c->reenter = 0;
    }
    long k = c->calls < c->sn ? c->script[c->calls] : (long)n;
    c->calls++;
    if (k > 0 && (unsigned long)k <= n)
    {
        memcpy(c->sent + c->bytes, p, (size_t)k);
        c->bytes += (size_t)k;
    }
    return k;
}

static int cancelled(void *v)
{
    struct cv_context *c = v;
    return c->cancel && c->bytes >= 2;
}

static struct cv_tx setup(struct cv_context *c)
{
    struct cv_tx t = {c, acquire, release, writer, cancelled, 0};
    return t;
}

int main(void)
{
    const unsigned char input[] = {1, 2, 3, 4, 5};
    for (unsigned fourth = 0; fourth <= 1; fourth++)
    {
        for (int fail = 0; fail < (fourth ? 5 : 4); fail++)
        {
            size_t count = fourth ? 4 : 3;
            struct cv_context c = {.fail = fail};
            struct cv_tx t = setup(&c);
            c.tx = &t;
            struct tx_owner out = {{77, 66, 55, 44}, 9}, old = out;
            struct tx_result r = tx_run(&t, fourth, input, 5, &out);
            size_t acquired = (size_t)fail < count ? (size_t)fail : count;
            CHECK(c.na == acquired && c.invalid == 0 && t.busy == 0);
            if (acquired < count)
            {
                CHECK(r.status == TX_ACQUIRE && r.committed == 0 &&
                      memcmp(&out, &old, sizeof out) == 0 && c.nr == acquired);
            }
            else
            {
                CHECK(r.status == TX_OK && r.committed == 5 &&
                      out.count == count && c.nr == 0);
                tx_destroy(&t, &out);
                CHECK(out.count == 0 && c.nr == count);
            }
            for (size_t i = 0; i < c.nr; i++)
            {
                CHECK(c.released[i] == c.acquired[c.na - 1 - i]);
            }
            for (unsigned id = 0; id < 31; id++)
            {
                CHECK(c.live[id] == 0);
            }
        }
    }
    const long scripts[][3] = {{2, 3, 0}, {2, 0, 0},  {2, -1, 0},
                               {2, 4, 0}, {-2, 0, 0}, {2, 3, 0}};
    const unsigned status[] = {TX_OK,      TX_ZERO,    TX_TIMEOUT,
                               TX_INVALID, TX_INVALID, TX_CANCEL};
    const size_t committed[] = {5, 2, 2, 2, 0, 2}, calls[] = {2, 2, 2, 2, 1, 1};
    for (size_t i = 0; i < 6; i++)
    {
        struct cv_context c = {
            .fail = -1, .sn = 3, .cancel = i == 5, .reenter = 1};
        memcpy(c.script, scripts[i], sizeof scripts[i]);
        struct cv_tx t = setup(&c);
        c.tx = &t;
        struct tx_owner out = {{91}, 7}, old = out;
        struct tx_result r = tx_run(&t, 0, input, 5, &out);
        CHECK(r.status == status[i] && r.committed == committed[i] &&
              c.bytes == committed[i] && c.calls == calls[i]);
        CHECK(c.reentry_status == TX_BUSY && c.invalid == 0 &&
              memcmp(c.sent, input, c.bytes) == 0);
        if (r.status != TX_OK)
        {
            CHECK(memcmp(&out, &old, sizeof out) == 0 && c.nr == 3);
        }
        else
        {
            tx_destroy(&t, &out);
        }
        for (unsigned id = 0; id < 31; id++)
        {
            CHECK(c.live[id] == 0);
        }
    }
    puts(
        "TX_PASS:three-and-four-prefix-identities,partial,zero,timeout,oversize,cancel,reentry");
    return 0;
}
