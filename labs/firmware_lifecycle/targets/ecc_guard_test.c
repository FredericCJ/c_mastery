#include "ecc_guard.h"
#include "flash_maintenance.h"
#include <stdio.h>
#include <string.h>
#define C(x)                                                   \
    do                                                         \
    {                                                          \
        if (!(x))                                              \
        {                                                      \
            fprintf(stderr, "EG_CHECK:%d:%s\n", __LINE__, #x); \
            return 1;                                          \
        }                                                      \
    } while (0)

struct f4_read
{
    unsigned fail, calls;
};

static int read_word(void *v, uint32_t a, uint32_t *out)
{
    struct f4_read *r = v;
    if (r->calls++ == r->fail)
    {
        return 0;
    }
    *out = a ^ 0x5A5A5A5Au;
    return 1;
}

struct f4_pair
{
    unsigned char a[128], b[128];
    unsigned failed_record, failed_word;
};

static int pair_read(void *v, uint32_t a, uint32_t *out)
{
    struct f4_pair *p = v;
    unsigned record = a >= 0x080E0000u;
    unsigned offset = a - (record ? 0x080E0000u : 0x080C0000u);
    if (offset >= 128 || offset % 4)
    {
        return 0;
    }
    if (p->failed_record == record && offset / 4 == p->failed_word)
    {
        return 0;
    }
    const unsigned char *b = (record ? p->b : p->a) + offset;
    *out = (uint32_t)b[0] | (uint32_t)b[1] << 8 | (uint32_t)b[2] << 16 |
           (uint32_t)b[3] << 24;
    return 1;
}

int main(void)
{
    uint32_t f[8] = {7, 8, 9, 10, 11, 12, 0x1020, 0x01000000u}, before[8];
    memcpy(before, f, sizeof f);
    C(eg_frame(f, 0xFFFFFFF9u, 0x8200u, 0x080C0000, 0x1020, 0x1030,
               0x080C0000) == 1 &&
      f[0] == 0 && f[6] == 0x1030);
    for (unsigned i = 1; i < 6; ++i)
    {
        C(f[i] == before[i]);
    }
    for (unsigned b = 0; b < 32; ++b)
    {
        memcpy(f, before, sizeof f);
        C(!eg_frame(f, 0xFFFFFFF9u, 0x8200u ^ (1u << b), 0x080C0000, 0x1020,
                    0x1030, 0x080C0000) &&
          !memcmp(f, before, sizeof f));
    }
    memcpy(f, before, sizeof f);
    C(!eg_frame(f, 0xFFFFFFF9u, 0x8200u, 0x080C0004, 0x1020, 0x1030,
                0x080C0000));
    C(!eg_frame(f, 0xFFFFFFF9u, 0x8200u, 0x080C0000, 0x1022, 0x1030,
                0x080C0000));
    C(!eg_frame(f, 0xFFFFFFFDu, 0x8200u, 0x080C0000, 0x1020, 0x1030,
                0x080C0000));
    C(!eg_frame(f, 0xFFFFFFE9u, 0x8200u, 0x080C0000, 0x1020, 0x1030,
                0x080C0000));
    for (unsigned failure = 0; failure <= 32; ++failure)
    {
        struct f4_read r = {failure, 0};
        unsigned char bytes[128], old[128];
        memset(bytes, 0xA5, 128);
        memset(old, 0xA5, 128);
        int result = eg_snapshot(read_word, &r, 0x080C0000, bytes);
        C(result == (failure == 32));
        if (failure < 32)
        {
            C(!memcmp(bytes, old, 128) && r.calls == failure + 1);
        }
        else
        {
            for (unsigned i = 0; i < 128; ++i)
            {
                C(bytes[i] ==
                  (unsigned char)(((0x080C0000u + 4 * (i / 4)) ^ 0x5A5A5A5Au) >>
                                  (8 * (i % 4))));
            }
        }
    }
    struct f4_pair pair;
    unsigned char hashes[64] = {0}, selected[128];
    uint32_t vectors[2] = {0x08040000, 0x08140000}, versions[2] = {1, 2};
    struct fw_peer peer = {.generation = 1, .health = 3, .confirmed = 1};
    C(fh_record(&peer, 1, hashes, vectors, versions, pair.a));
    C(fh_record(&peer, 2, hashes, vectors, versions, pair.b));
    for (unsigned record = 0; record < 2; ++record)
    {
        for (unsigned word = 0; word < 32; ++word)
        {
            pair.failed_record = record;
            pair.failed_word = word;
            C(eg_scan_pair(pair_read, &pair, 0x080C0000, 0x080E0000,
                           selected) == (record ? 1 : 2));
            C(!memcmp(selected, record ? pair.a : pair.b, 128));
        }
    }
    pair.failed_record = 2;
    C(eg_scan_pair(pair_read, &pair, 0x080C0000, 0x080E0000, selected) == 2);
    memset(pair.a, 0, 128);
    memset(pair.b, 0, 128);
    memset(selected, 0xA5, 128);
    C(!eg_scan_pair(pair_read, &pair, 0x080C0000, 0x080E0000, selected));
    for (unsigned i = 0; i < 128; ++i)
    {
        C(selected[i] == 0xA5);
    }
    puts(
        "EG_PASS:exact fault identity,32 status controls,all32 failed-read positions,64 candidate faults preserve readable pair");
    return 0;
}
