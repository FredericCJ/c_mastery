#include "amp.h"
#include <stdio.h>
#include <string.h>
#include <limits.h>
#define C(x)                                                                \
    do                                                                      \
    {                                                                       \
        if (!(x))                                                           \
        {                                                                   \
            fprintf(stderr, "AF_CHECK:%s:%d:%s\n", __func__, __LINE__, #x); \
            return 1;                                                       \
        }                                                                   \
    } while (0)

static int images(void)
{
    struct af_bank banks[] = {{0x20000000, 0x20000, 1, 1},
                              {0x30000000, 0x20000, 7, 2},
                              {0x30020000, 0x20000, 7, 2},
                              {0x30040000, 0x8000, 7, 2},
                              {0x38000000, 0x10000, 3, 3}};
    /* Master bits M7=1,M4=2,DMA2=4: selected tested ledger subset. Actual
     * board bus reachability is a separate primary-source/physical premise. */
    unsigned domain = 99;
    C(af_region(banks, 5, 0x30040000, 32768, 3, &domain) == AF_OK &&
      domain == 2);
    C(af_region(banks, 5, 0x20000000, 32, 3, &domain) == AF_INVALID &&
      domain == 2);
    C(af_region(banks, 5, 0x38000000, 32768, 3, &domain) == AF_OK &&
      domain == 3);
    C(af_region(banks, 5, 0x30047fff, 2, 3, &domain) == AF_INVALID);
    C(af_region(banks, 5, UINT32_MAX - 1, 4, 3, &domain) == AF_INVALID);
    struct af_owners owners = {0};
    for (unsigned r = 0; r < 4; r++)
        C(af_owner_claim(&owners, r, r == 3 ? 4 : 7) == AF_OK);
    C(af_owner_claim(&owners, 2, 4) == AF_CONFLICT &&
      owners.telemetry_owner == 7);
    struct af_owners moved = {7, 7, 4, 7};
    C(af_owner_claim(&moved, 2, 4) == AF_OK &&
      af_owner_claim(&moved, 2, 7) == AF_CONFLICT);
    puts(
        "AF_IMAGE:TCM not peer shared;SRAM3 D2 and SRAM4 D3 distinct premises;duplicate telemetry owner refused");
    return 0;
}

static int boot(void)
{
    struct af_boot b;
    atomic_init(&b.ready, 9);
    atomic_init(&b.version, 1);
    atomic_init(&b.layout, AF_LAYOUT - 1);
    atomic_init(&b.epoch, 9);
    atomic_init(&b.ack, 9);
    atomic_init(&b.parked, 0);
    C(af_boot_ack(&b, 2, AF_LAYOUT, 9) == AF_STALE);
    atomic_store(&b.version, 2);
    C(af_boot_ack(&b, 2, AF_LAYOUT, 9) == AF_STALE);
    struct af_boot_owner s = {0}, before = s;
    C(af_boot_begin(&s, 9, UINT32_MAX - 10, 100, 0, 1) == AF_BUSY &&
      !memcmp(&s, &before, sizeof s));
    C(af_boot_begin(&s, 9, UINT32_MAX - 10, 100, 1, 0) == AF_BUSY);
    C(af_boot_begin(&s, 9, UINT32_MAX - 10, 100, 1, 1) == AF_OK);
    C(af_boot_stage(&s, &b, 1, 2, AF_LAYOUT) == AF_INVALID);
    for (unsigned stage = 0; stage < 4; stage++)
    {
        C(af_boot_stage(&s, &b, stage, 2, AF_LAYOUT) == AF_OK);
        C(af_boot_stage(&s, &b, stage, 2, AF_LAYOUT) == AF_OK);
        C(af_boot_ack(&b, 2, AF_LAYOUT, 10) == (stage == 3 ? AF_OK : AF_BUSY));
    }
    C(af_boot_status(&s, &b, 20) == AF_OK);
    C(af_boot_ack(&b, 2, AF_LAYOUT, 9) == AF_STALE);
    C(af_boot_close(&s, &b, 0, 1) == AF_BUSY && atomic_load(&b.ready) == 0 &&
      s.active);
    C(af_boot_begin(&s, 10, 21, 100, 1, 1) == AF_BUSY);
    C(af_boot_close(&s, &b, 1, 1) == AF_OK);
    C(af_boot_begin(&s, 10, 21, 100, 1, 1) == AF_OK);
    C(af_boot_status(&s, &b, 120) == AF_BUSY &&
      af_boot_status(&s, &b, 121) == AF_STALE);
    C(af_boot_status(&s, &b, 21 + UINT32_C(0x80000000)) == AF_INVALID);
    C(af_boot_close(&s, &b, 1, 1) == AF_OK &&
      af_boot_begin(&s, UINT32_MAX, 0, 1, 1, 1) == AF_EXHAUSTED);
    unsigned expired = 99;
    C(af_elapsed(UINT32_MAX - 3, 2, 6, &expired) == AF_OK && expired == 1);
    C(af_elapsed(0, 0, UINT32_C(0x80000000), &expired) == AF_INVALID);
    puts(
        "AF_BOOT:layout width before payload;4 replayable publication stages;both quiescence domains;wrapped deadline6;MAX epoch retires");
    return 0;
}

static struct af_message message(unsigned id, unsigned length)
{
    struct af_message m = {7, id, length, {0}};
    for (unsigned j = 0; j < length; j++)
        m.bytes[j] = (unsigned char)(id * 17 + j);
    return m;
}

static int ring(void)
{
    struct af_ring q;
    af_ring_init(&q, UINT32_MAX - 1);
    C(atomic_is_lock_free(&q.head) && atomic_is_lock_free(&q.tail));
    for (unsigned batch = 0; batch < 4; batch++)
    {
        struct af_message v[4];
        for (unsigned j = 0; j < 4; j++)
            v[j] = message(batch * 4 + j + 1, (batch * 4 + j) % 33);
        C(af_batch(&q, 7, v, 4) == AF_OK);
        C(af_queued(&q) == (batch + 1) * 4);
    }
    struct af_message v = message(17, 32);
    C(af_batch(&q, 7, &v, 1) == AF_FULL);
    unsigned char bytes[32];
    uint32_t id = 99;
    size_t length = 99;
    for (unsigned n = 1; n <= 16; n++)
    {
        memset(bytes, 0xee, sizeof bytes);
        uint32_t tail = atomic_load(&q.tail);
        unsigned size = (n - 1) % 33;
        if (size)
        {
            C(af_receive(&q, 7, bytes, size - 1, &id, &length) == AF_INVALID &&
              atomic_load(&q.tail) == tail);
            for (unsigned j = 0; j < 32; j++)
                C(bytes[j] == 0xee);
        }
        C(af_receive(&q, 6, bytes, 32, &id, &length) == AF_STALE &&
          atomic_load(&q.tail) == tail);
        C(af_receive(&q, 7, bytes, 32, &id, &length) == AF_OK && id == n &&
          length == size);
        for (unsigned j = 0; j < size; j++)
            C(bytes[j] == (unsigned char)(n * 17 + j));
    }
    C(af_receive(&q, 7, bytes, 32, &id, &length) == AF_EMPTY && id == 16);
    C(af_batch(&q, 7, &v, 1) == AF_OK);
    C(af_receive(&q, 7, bytes, 31, &id, &length) == AF_INVALID &&
      af_queued(&q) == 1);
    C(af_receive(&q, 7, bytes, 32, &id, &length) == AF_OK && id == 17 &&
      length == 32);
    atomic_store(&q.head, 1);
    atomic_store(&q.tail, UINT32_MAX - 1);
    C(af_queued(&q) == 3);
    atomic_store(&q.head, 100);
    C(af_batch(&q, 7, &v, 1) == AF_INVALID);
    puts(
        "AF_RING:4x4 batch publication;full16/wrap/variable0..32;short receive retains slot/output;distance3 at head1 tailFFFFFFFE");
    return 0;
}

static int cache(void)
{
    uint32_t a = 99, b = 99;
    C(af_cache_span(0x3004001f, 2, 32, &a, &b) == AF_OK && a == 0x30040000 &&
      b == 0x30040040);
    C(af_cache_span(0x3004001f, 2, 64, &a, &b) == AF_OK && a == 0x30040000 &&
      b == 0x30040040);
    C(af_cache_span(0x3004003f, 2, 64, &a, &b) == AF_OK && b == 0x30040080);
    C(af_cache_span(UINT32_MAX - 31, 32, 32, &a, &b) == AF_INVALID);
    C(af_cache_span(0, 1, 3, &a, &b) == AF_INVALID);
    puts(
        "AF_CACHE:32/64 line rounding and UINT32 endpoint overflow;actual visibility validated through separate frozen Pd/cache providers");
    return 0;
}

static int receipt(void)
{
    struct af_receipt s = {4, 0, 10, 0, 0};
    int32_t out = 777;
    C(af_effect(&s, 4, 1, 5, &out) == AF_OK && out == 15);
    C(af_effect(&s, 4, 1, 5, &out) == AF_OK && out == 15 && s.total == 15);
    C(af_effect(&s, 4, 1, 6, &out) == AF_CONFLICT && out == 15 &&
      s.total == 15);
    C(af_effect(&s, 3, 2, 1, &out) == AF_STALE);
    C(af_effect(&s, 4, 2, -2, &out) == AF_OK && out == 13);
    C(af_effect(&s, 4, 1, 5, &out) == AF_STALE);
    s.total = INT32_MAX;
    C(af_effect(&s, 4, 3, 1, &out) == AF_INVALID && out == 13 &&
      s.last_id == 2);
    s.last_id = UINT32_MAX;
    C(af_effect(&s, 4, 1, 0, &out) == AF_EXHAUSTED);
    puts(
        "AF_RECEIPT:peer-reset survivor receipt deduplicates latest effect;different payload conflicts;old horizon stale;full-machine persistence not implied");
    return 0;
}

int main(int argc, char **argv)
{
    if (argc != 2)
        return 2;

    struct
    {
        const char *name;
        int (*run)(void);
    } tests[] = {{"images", images},
                 {"boot", boot},
                 {"ring", ring},
                 {"cache", cache},
                 {"receipt", receipt}};

    for (unsigned i = 0; i < sizeof tests / sizeof tests[0]; i++)
        if (!strcmp(argv[1], tests[i].name))
        {
            int r = tests[i].run();
            if (!r)
                printf("AF_PASS:%s\n", tests[i].name);
            return r;
        }
    return 2;
}
