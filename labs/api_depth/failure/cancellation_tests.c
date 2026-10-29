#include "cancellation.h"
#include <stdio.h>
#include <stdlib.h>
#define CHECK(x)                                                   \
    do                                                             \
    {                                                              \
        if (!(x))                                                  \
        {                                                          \
            fprintf(stderr, "CANCEL-CHECK:%d:%s\n", __LINE__, #x); \
            return 1;                                              \
        }                                                          \
    } while (0)

struct ac_receiver
{
    unsigned reports, generation, bad;
    enum ae_code code;
    struct ae_operation *release;
};

static void receive(void *v, enum ae_code code, unsigned generation,
                    const unsigned char *p, unsigned n)
{
    struct ac_receiver *r = v;
    ++r->reports;
    r->generation = generation;
    r->code = code;
    if (n != 4 || p[0] != 1 || p[3] != 4)
    {
        ++r->bad;
    }
    if (r->release)
    {
        free(r->release);
        r->release = 0;
    }
}

static int history(unsigned word, unsigned length)
{
    struct ae_operation o = {0};
    struct ac_receiver r = {0};
    unsigned generation;
    const unsigned char p[4] = {1, 2, 3, 4};
    CHECK(ae_operation_start(&o, p, 4, receive, &r, &generation) == AE_OK);
    unsigned timeout = 0, done = 0;
    for (unsigned i = 0; i < length; ++i)
    {
        unsigned e = word % 4;
        word /= 4;
        unsigned old_reports = r.reports;
        enum ae_code result = ae_operation_event(&o, 1, (enum ae_event)e);
        if (done)
        {
            CHECK(result == AE_CONFLICT);
        }
        else if (e == AE_DEADLINE)
        {
            timeout = 1;
            CHECK(result == AE_AGAIN);
        }
        else if (e == AE_COMPLETE || timeout)
        {
            done = 1;
            CHECK(result == AE_OK);
        }
        else
        {
            CHECK(result == AE_AGAIN);
        }
        CHECK(r.reports == done && o.backend.releases == done &&
              r.reports >= old_reports);
        if (done)
        {
            CHECK(r.code == (timeout ? AE_TIMEOUT : AE_OK));
        }
        CHECK(!r.bad);
    }
    if (!done)
    {
        CHECK(ae_operation_event(&o, 1, AE_COMPLETE) == AE_OK);
    }
    CHECK(o.backend.releases == 1 && r.reports == 1);
    return 0;
}

int main(void)
{
    unsigned cases = 0, power = 1;
    for (unsigned n = 0; n <= 6; ++n)
    {
        for (unsigned w = 0; w < power; ++w)
        {
            CHECK(!history(w, n));
            ++cases;
        }
        power *= 4;
    }
    const unsigned char p[4] = {1, 2, 3, 4};
    struct ae_operation o = {0};
    struct ac_receiver r = {0};
    unsigned generation = 99;
    for (unsigned g = 1; g <= 3; ++g)
    {
        CHECK(ae_operation_start(&o, p, 4, receive, &r, &generation) == AE_OK &&
              generation == g);
        if (g > 1)
        {
            CHECK(ae_operation_event(&o, g - 1, AE_COMPLETE) == AE_CONFLICT &&
                  o.backend.releases == g - 1);
        }
        CHECK(ae_operation_event(&o, g, AE_DEADLINE) == AE_AGAIN);
        CHECK(ae_operation_start(&o, p, 4, receive, &r, &generation) ==
              AE_CONFLICT);
        CHECK(ae_operation_event(&o, g, AE_CANCEL_ACK) == AE_OK);
        CHECK(ae_operation_event(&o, g, AE_COMPLETE) == AE_CONFLICT &&
              o.backend.releases == g && r.reports == g);
    }
    CHECK(ae_operation_start(&o, p, 4, receive, &r, &generation) == AE_FULL &&
          generation == 3);
    o = (struct ae_operation){0};
    r = (struct ac_receiver){0};
    CHECK(ae_operation_start(&o, p, 4, receive, &r, &generation) == AE_OK);
    for (unsigned i = 0; i < 20; ++i)
    {
        CHECK(ae_operation_event(&o, 1, AE_DEADLINE) == AE_AGAIN &&
              !r.reports && !o.backend.releases);
    }
    CHECK(ae_operation_event(&o, 1, AE_JOINED) == AE_OK && r.reports == 1);
    struct ae_operation *heap = calloc(1, sizeof *heap);
    CHECK(heap);
    r = (struct ac_receiver){.release = heap};
    CHECK(ae_operation_start(heap, p, 4, receive, &r, &generation) == AE_OK);
    CHECK(ae_operation_event(heap, 1, AE_COMPLETE) == AE_OK && !r.release &&
          r.reports == 1 && !r.bad);
    printf("CANCEL-PASS:histories=%u;max_events=6;generation_limit=3;\n",
           cases);
    return 0;
}
