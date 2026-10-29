#include "boot_storage.h"
#include "monocypher-ed25519.h"
#include <stdio.h>
#include <string.h>
#define C(x)                                                   \
    do                                                         \
    {                                                          \
        if (!(x))                                              \
        {                                                      \
            fprintf(stderr, "BS_CHECK:%d:%s\n", __LINE__, #x); \
            return 1;                                          \
        }                                                      \
    } while (0)

struct f4_store
{
    unsigned char slot[2][2][2144], record[2][128];
    struct bs_layouts layouts;
    uint32_t failure;
    unsigned reads, bad;
};
static struct f4_store storage;

static uint32_t get(const unsigned char *p)
{
    return (uint32_t)p[0] | (uint32_t)p[1] << 8 | (uint32_t)p[2] << 16 |
           (uint32_t)p[3] << 24;
}

static void put(unsigned char *p, uint32_t v)
{
    for (unsigned i = 0; i < 4; ++i)
        p[i] = (unsigned char)(v >> (8 * i));
}

static int read_word(void *v, uint32_t a, uint32_t *out)
{
    struct f4_store *s = v;
    s->reads++;
    if (a == s->failure)
        return 0;
    for (unsigned r = 0; r < 2; ++r)
    {
        uint32_t base = 0x080C0000u + r * 0x20000u;
        if (a >= base && a - base < 128)
        {
            *out = get(s->record[r] + a - base);
            return 1;
        }
    }
    for (unsigned c = 0; c < 2; ++c)
        for (unsigned slot = 0; slot < 2; ++slot)
        {
            uint32_t base = s->layouts.core[c][slot].base - 32;
            if (a >= base && a - base <= 2140)
            {
                *out = get(s->slot[c][slot] + a - base);
                return 1;
            }
        }
    s->bad++;
    return 0;
}

int main(void)
{
    struct fb_policy policy = {0};
    unsigned char secret[2][64];
    for (unsigned key = 0; key < 2; ++key)
    {
        unsigned char seed[32] = {0};
        seed[0] = (unsigned char)(key + 1);
        crypto_ed25519_key_pair(secret[key], policy.keys[key], seed);
    }
    policy.present = 6;
    policy.floor[0] = policy.floor[1] = 3;
    policy.generation_floor = 5;
    for (unsigned generation = 0; generation < 2; ++generation)
    {
        struct fb_image pair[2];
        for (unsigned core = 0; core < 2; ++core)
        {
            uint32_t base =
                0x08040000u + core * 0x100000u + generation * 0x40000u;
            storage.layouts.core[core][generation] = (struct fb_layout){
                base, base + 0x20000u, 0x20000000u, 0x20020000u};
            unsigned char *b = storage.slot[core][generation];
            put(b, 0x31423646u);
            b[4] = (unsigned char)(generation + 1);
            b[5] = (unsigned char)core;
            put(b + 8, 3 + generation);
            put(b + 12, 5 + generation);
            put(b + 16, base);
            put(b + 20, 576);
            put(b + 32, 0x20010000u);
            put(b + 36, base + 513);
            crypto_ed25519_sign(b + 2080, secret[generation], b, 608);
            C(fb_admit(&policy, storage.layouts.core[core][generation], core, b,
                       608, b + 2080, &pair[core]));
        }
        struct fw_peer peer = {
            .generation = 5 + generation, .health = 3, .confirmed = 1};
        C(fb_joint_record(pair, &peer, 9 + generation,
                          storage.record[generation]));
    }
    const uint32_t records[2] = {0x080C0000u, 0x080E0000u};
    struct bs_selection chosen, before;
    memset(&chosen, 0xA5, sizeof chosen);
    C(bs_select(read_word, &storage, &policy, &storage.layouts, records,
                &chosen) == 1 &&
      chosen.record == 1 && chosen.sequence == 10 && !storage.bad);
    /* Fault every header/payload/signature word of each newest component. */
    for (unsigned core = 0; core < 2; ++core)
        for (unsigned word = 0; word < 168; ++word)
        {
            storage.failure = storage.layouts.core[core][1].base - 32 +
                              (word < 152 ? 4 * word : 2080 + 4 * (word - 152));
            C(bs_select(read_word, &storage, &policy, &storage.layouts, records,
                        &chosen) == 1 &&
              chosen.record == 0 && chosen.sequence == 9 && !storage.bad);
        }
    for (unsigned record = 0; record < 2; ++record)
        for (unsigned word = 0; word < 32; ++word)
        {
            storage.failure = records[record] + 4 * word;
            C(bs_select(read_word, &storage, &policy, &storage.layouts, records,
                        &chosen) == 1 &&
              chosen.record == (1 - record));
        }
    storage.failure = 0;
    /* A structurally valid journal with an authenticated-image digest mismatch. */
    unsigned char saved[128];
    memcpy(saved, storage.record[1], 128);
    struct fw_peer peer = {.generation = 6, .health = 3, .confirmed = 1};
    unsigned char false_hashes[64] = {0};
    uint32_t vectors[2] = {storage.layouts.core[0][1].base,
                           storage.layouts.core[1][1].base},
             versions[2] = {4, 4};
    C(fh_record(&peer, 10, false_hashes, vectors, versions, storage.record[1]));
    C(bs_select(read_word, &storage, &policy, &storage.layouts, records,
                &chosen) == 1 &&
      chosen.record == 0);
    memcpy(storage.record[1], saved, 128);
    policy.revoked = 4;
    C(bs_select(read_word, &storage, &policy, &storage.layouts, records,
                &chosen) == 1 &&
      chosen.record == 0);
    policy.floor[0] = policy.floor[1] = 4;
    before = chosen;
    C(bs_select(read_word, &storage, &policy, &storage.layouts, records,
                &chosen) == 0 &&
      !memcmp(&before, &chosen, sizeof before));
    policy.revoked = 0;
    C(bs_select(read_word, &storage, &policy, &storage.layouts, records,
                &chosen) == 1 &&
      chosen.record == 1);
    puts(
        "BS_PASS:actual guarded-read callback/admission/journal composition,336 image-word faults,64 record-word faults,policy-bounded fallback");
    return 0;
}
