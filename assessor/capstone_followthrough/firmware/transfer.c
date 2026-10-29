#include "firmware.h"
#include <stdio.h>
#define C(x)                                     \
    do                                           \
    {                                            \
        if (!(x))                                \
        {                                        \
            puts("CAP_MCU_TRANSFER_REJECT:" #x); \
            return 41;                           \
        }                                        \
    } while (0)

static int ready(void *p)
{
    (void)p;
    return 1;
}

static int feed(struct cs_service *s, const unsigned char *b, size_t n)
{
    size_t position = 0;
    for (unsigned turn = 0; turn < 100; ++turn)
    {
        size_t used = 0;
        enum cs_status st =
            cs_feed(s, 1, b + position, n - position, &used, 1, 0);
        position += used;
        if (st == CS_MORE)
            return position == n;
        if (st != CS_FRAME && st != CS_REJECT)
            return 0;
        if (position == n && !s->input[1].used)
            return 1;
    }
    return 0;
}

int main(void)
{
    struct cf_firmware f;
    C(cf_init(&f) && cf_boot(&f, ready, 0, 1) == CF_OK);
    struct cp_message m = {400, 3, {0xa5, 9, 0}};
    unsigned char wire[13], bad[13];
    size_t n = 0;
    C(cp_encode(&m, wire, 13, &n) == CP_OK && n == 13);
    for (unsigned i = 0; i < 13; ++i)
        bad[i] = wire[i];
    bad[12] ^= 1;
    C(feed(&f.core, bad, 13));
    C(!f.core.accepted);
    C(feed(&f.core, wire, 7) && feed(&f.core, wire + 7, 6) &&
      f.core.accepted == 1);
    C(cs_begin(&f.core) == CS_OK);
    const unsigned char *bytes;
    size_t length;
    unsigned generation;
    C(cs_lease(&f.core, &bytes, &length, &generation) == CS_OK &&
      bytes[6] == 0xa5);
    cf_fault(&f, 71);
    C(cf_recover(&f) == CF_PENDING && f.core.borrowed && bytes[6] == 0xa5);
    C(cs_complete(&f.core, generation + 1, 1, 0, 0, 0) == CS_STALE);
    C(cs_complete(&f.core, generation, 0, 0, 0, 0) == CS_PENDING);
    C(cs_complete(&f.core, generation, 1, 5, 0, 0) == CS_CLOSED);
    C(cf_recover(&f) == CF_OK && f.prior_accepted == 1 &&
      f.prior_delivered == 0 && f.prior_discarded == 1 && f.first_fault == 71 &&
      !f.actuator);
    puts(
        "CAP_MCU_TRANSFER_PASS split7/6 payloadA5 stale-before-quiescent five-byte-prefix recovery-ledger");
    return 0;
}
