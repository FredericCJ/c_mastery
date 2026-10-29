#include "h745_flash.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define C(x)                                                          \
    do                                                                \
    {                                                                 \
        if (!(x))                                                     \
        {                                                             \
            fprintf(stderr, "B06_FLASH_CHECK:%s:%d\n", #x, __LINE__); \
            exit(1);                                                  \
        }                                                             \
    } while (0)

struct f4_bus
{
    uint32_t cr, sr, words[32];
    unsigned key, writes, polls, mode, unlocks, barriers, programs, used, unit;
};

static uint32_t read32(void *p, uint32_t a)
{
    struct f4_bus *b = p;
    if (a == 0x5200200Cu)
        return b->cr;
    if (a == 0x52002010u)
    {
        if (b->writes == 8 && b->mode != 1 && ++b->polls == 3)
            b->sr =
                (b->mode == 2 || (b->mode >= 3 && b->programs == b->mode - 2))
                    ? 0x20000u
                    : 0x10000u;
        return b->sr;
    }
    C(a >= 0x08020000u && a < 0x08020080u);
    return b->words[(a - 0x08020000u) / 4];
}

static void write32(void *p, uint32_t a, uint32_t v)
{
    struct f4_bus *b = p;
    if (a == 0x52002004u)
    {
        C(b->cr & 1u);
        if (!b->key)
        {
            C(v == 0x45670123u);
            b->key = 1;
        }
        else
        {
            C(v == 0xCDEF89ABu);
            b->cr &= ~1u;
            b->key = 0;
            ++b->unlocks;
        }
        return;
    }
    if (a == 0x5200200Cu)
    {
        C(!(b->sr & 7u));
        if ((v & 2u) && !(b->cr & 2u))
        {
            b->writes = 0;
            b->polls = 0;
        }
        b->cr = v;
        return;
    }
    if (a == 0x52002014u)
    {
        b->sr &= ~v;
        return;
    }
    C((b->cr & 3u) == 2u);
    C(a >= 0x08020000u && a < 0x08020080u);
    if (!b->writes)
    {
        b->unit = (a - 0x08020000u) / 32;
        C(!(b->used & (1u << b->unit)));
        b->used |= 1u << b->unit;
    }
    C(a == 0x08020000u + 32 * b->unit + b->writes * 4);
    C(b->writes < 8);
    b->words[b->unit * 8 + b->writes++] = v;
    if (b->writes == 8)
    {
        ++b->programs;
        b->sr = 4u | 0x10000u;
    } /* Busy wins even if EOP is also set. */
}

static void sync32(void *p)
{
    ++((struct f4_bus *)p)->barriers;
}

int main(int argc, char **argv)
{
    const uint32_t words[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    for (unsigned locked = 0; locked < 2; ++locked)
        for (unsigned mode = 0; mode < 3; ++mode)
        {
            struct f4_bus b = {.cr = 0x30u | locked, .mode = mode};
            struct fh_io io = {&b, read32, write32, sync32};
            struct fh_session s = {0};
            C(fh_start(&s, io, 0x08020000u, words) == FH_PENDING);
            C(b.writes == 8 && b.unlocks == locked);
            C(fh_start(&s, io, 0x08020000u, words) == FH_REJECT);
            C(fh_poll(&s) == FH_PENDING);
            C(s.active);
            if (mode == 1)
            {
                for (unsigned i = 0; i < 1000; ++i)
                    C(fh_poll(&s) == FH_PENDING);
                C((b.cr & 2u) && s.active);
            }
            else
            {
                C(fh_poll(&s) == FH_PENDING);
                C(fh_poll(&s) == (mode == 2 ? FH_ERROR : FH_DONE));
                C(!s.active && b.cr == (0x30u | locked));
                C(fh_poll(&s) == FH_REJECT);
            }
        }
    struct f4_bus b = {.cr = 0x31, .sr = 1};
    struct fh_io io = {&b, read32, write32, sync32};
    struct fh_session s = {0};
    C(fh_start(&s, io, 0x08020000u, words) == FH_REJECT);
    C(!b.writes);
    b.sr = 0;
    C(fh_start(&s, io, 0x08020004u, words) == FH_REJECT);
    struct fw_peer peer = {.generation = 7, .health = 3, .confirmed = 1};
    unsigned char hashes[64], record[128], cut[128];
    uint32_t vectors[2] = {0x08040000u, 0x08140000u}, versions[2] = {9, 12};
    for (unsigned i = 0; i < 64; ++i)
        hashes[i] = (unsigned char)(i * 13);
    C(fh_record(&peer, 19, hashes, vectors, versions, record));
    C(fh_record_valid(record));
    if (argc == 2 && !strcmp(argv[1], "--dump"))
    {
        for (unsigned i = 0; i < 128; ++i)
            printf("%02x", record[i]);
        puts("");
        return 0;
    }
    unsigned checked = 0;
    for (unsigned prefix = 0; prefix <= 1024; ++prefix)
    {
        memset(cut, 255, sizeof cut);
        for (unsigned bit = 0; bit < prefix; ++bit)
            if (!(record[bit / 8] & (1u << (bit % 8))))
                cut[bit / 8] &= (unsigned char)~(1u << (bit % 8));
        C(fh_record_valid(cut) == (prefix == 1024));
        ++checked;
    }
    for (unsigned i = 0; i < 128; ++i)
    {
        memcpy(cut, record, 128);
        cut[i] ^= 1;
        C(!fh_record_valid(cut));
    }
    for (unsigned fail = 0; fail <= 4; ++fail)
    {
        struct f4_bus bus = {.cr = 0x31, .mode = fail ? fail + 2 : 0};
        struct fh_io port = {&bus, read32, write32, sync32};
        struct fh_append append = {0};
        C(fh_append_begin(&append, port, 0x08020000u, &peer, 19, hashes,
                          vectors, versions) == FH_PENDING);
        C(fh_append_begin(&append, port, 0x08020000u, &peer, 19, hashes,
                          vectors, versions) == FH_REJECT);
        enum fh_result result;
        unsigned polls = 0;
        do
        {
            result = fh_append_poll(&append);
            C(++polls <= 12);
        } while (result == FH_PENDING);
        C(result == (fail ? FH_ERROR : FH_DONE));
        C(bus.programs == (fail ? fail : 4));
        C(fh_append_poll(&append) == FH_REJECT);
        if (!fail)
            C(!memcmp(bus.words, record, 128));
    }
    peer.outstanding[1] = 1;
    memset(cut, 0xA7, 128);
    C(!fh_record(&peer, 19, hashes, vectors, versions, cut));
    for (unsigned i = 0; i < 128; ++i)
        C(cut[i] == 0xA7);
    printf(
        "B06_FLASH_PASS:%u commit prefixes,128 corruptions, locked/unlocked/busy/error controls\n",
        checked);
}
