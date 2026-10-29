#include "transfer.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#define C(x)                                                   \
    do                                                         \
    {                                                          \
        if (!(x))                                              \
        {                                                      \
            fprintf(stderr, "VT_CHECK:%d:%s\n", __LINE__, #x); \
            exit(1);                                           \
        }                                                      \
    } while (0)

struct vt_context
{
    struct vt_queue *q;
    unsigned prepares, releases, reentries;
    int fail;
};

static int prepare(void *v, const void *p)
{
    struct vt_context *c = v;
    (void)p;
    c->prepares++;
    C(!vt_remove(c->q));
    c->reentries++;
    return !c->fail;
}

static void release(void *v, const void *p)
{
    struct vt_context *c = v;
    C(*(const int *)p == 19);
    c->releases++;
    C(!vt_remove(c->q));
    C(!vt_offer(c->q, p, prepare, release, c));
    c->reentries++;
}

int main(void)
{
    uint32_t out = 19;
    C(vt_mul(0, UINT32_MAX, &out) && out == 0);
    out = 19;
    C(!vt_mul(UINT32_MAX, 2, &out) && out == 19);
    C(vt_mul(65535, 65537, &out) && out == UINT32_MAX);
    uint16_t small = 91;
    C(!vt_narrow(-1, &small) && small == 91);
    C(!vt_narrow(65536, &small) && small == 91);
    C(vt_narrow(65535, &small) && small == 65535);
    struct vt_record a[] = {{9, 0}, {7, 1}, {7, 2}, {-2, 3}};
    for (int k = -4; k <= 11; k++)
    {
        size_t x = vt_desc(a, 4, k);
        for (size_t i = 0; i < x; i++)
        {
            C(a[i].key > k);
        }
        for (size_t i = x; i < 4; i++)
        {
            C(a[i].key <= k);
        }
    }
    struct vt_queue q = {0};
    struct vt_context c = {&q, 0, 0, 0, 0};
    int payload = 19;
    C(vt_offer(&q, &payload, prepare, release, &c));
    c.fail = 1;
    C(!vt_offer(&q, &payload, prepare, release, &c) && q.n == 1 &&
      c.releases == 0);
    c.fail = 0;
    C(vt_offer(&q, &payload, prepare, release, &c));
    C(vt_remove(&q) && vt_remove(&q) && !vt_remove(&q) && c.releases == 2 &&
      c.prepares == 3 && c.reentries == 5);
    C(payload == 19);
    unsigned char bytes[18];
    for (unsigned len = 0; len <= 16; len++)
    {
        bytes[0] = (unsigned char)len;
        unsigned check = 0;
        uint32_t want = 0;
        for (unsigned i = 0; i < len; i++)
        {
            bytes[i + 1] = (unsigned char)(i * 7 + len);
            check ^= bytes[i + 1];
            want = want * 33 + bytes[i + 1];
        }
        bytes[len + 1] = (unsigned char)check;
        for (size_t i = 0; i <= len + 2; i++)
        {
            for (size_t j = i; j <= len + 2; j++)
            {
                struct vt_stream s = {0};
                vt_reset(&s);
                C(vt_feed(&s, bytes, i));
                C(vt_feed(&s, bytes + i, j - i));
                C(vt_feed(&s, bytes + j, len + 2 - j));
                out = 91;
                C(vt_finish(&s, &out) && out == want);
                C(!vt_feed(&s, bytes, 1));
                out = 91;
                C(!vt_finish(&s, &out) && out == 91);
                vt_reset(&s);
                C(vt_feed(&s, bytes, len + 2) && vt_finish(&s, &out) &&
                  out == want);
            }
        }
    }
    struct vt_event ev[] = {{1, 4}, {2, 0}, {3, 1}, {4, 4},
                            {5, 3}, {6, 3}, {7, 2}};
    C(vt_violation(ev, 7) == 5);
    size_t n = vt_shrink(ev, 7);
    C(n == 3 && vt_violation(ev, n) == 5 && ev[0].id == 2 && ev[1].id == 3 &&
      ev[2].id == 5);
    puts("VT_PASS arithmetic descending lifetime splits identity");
    return 0;
}
