#include "contracts.h"
#include <stdio.h>
#include <string.h>
#define CHECK(x)                                               \
    do                                                         \
    {                                                          \
        if (!(x))                                              \
        {                                                      \
            fprintf(stderr, "KF_CHECK:%u:%s\n", __LINE__, #x); \
            return 1;                                          \
        }                                                      \
    } while (0)

static void w32(unsigned char *p, uint32_t x)
{
    for (unsigned i = 0; i < 4; ++i)
        p[i] = (unsigned char)(x >> (8 * i));
}

static void w64(unsigned char *p, uint64_t x)
{
    for (unsigned i = 0; i < 8; ++i)
        p[i] = (unsigned char)(x >> (8 * i));
}

static size_t base(unsigned char *p, unsigned stride)
{
    memset(p, 0, 512);
    w32(p + 8, 3);
    w32(p + 12, 21);
    w32(p + 16, 0x210000);
    w32(p + 20, 0x220000);
    memcpy(p + 24, "init", 5);
    w32(p + 32, 6);
    w32(p + 36, 16 + stride);
    w32(p + 40, stride);
    w64(p + 48, 0);
    w64(p + 56, 0x400000);
    w32(p + 64, 1);
    size_t n = 56 + stride;
    w32(p + n - 4, 8);
    w32(p, (uint32_t)n);
    return n;
}

static int refused(const unsigned char *p, size_t n)
{
    struct kf_boot out, before;
    memset(&out, 0xa5, sizeof out);
    memcpy(&before, &out, sizeof out);
    return !kf_boot(p, n, &out) && memcmp(&before, &out, sizeof out) == 0;
}

static int tags(void)
{
    unsigned char p[512], q[512];
    struct kf_boot out;
    size_t n = base(p, 40);
    CHECK(kf_boot(p, n, &out) && out.next == n && out.visited == 3 &&
          out.core.entry_size == 40);
    CHECK(out.core.map == p + 48 && out.core.module_start == 0x210000);
    for (size_t i = 0; i < n; ++i)
        CHECK(refused(p, i));
    for (unsigned size = 7; size <= 8; ++size)
    {
        base(p, 40);
        w32(p + 12, size);
        CHECK(refused(p, n));
    }
    base(p, 40);
    w32(p + 12, UINT32_MAX);
    CHECK(refused(p, n));
    base(p, 40);
    w32(p + 40, 25);
    CHECK(refused(p, n));
    base(p, 40);
    w64(p + 48, UINT64_MAX - 3);
    w64(p + 56, 8);
    CHECK(refused(p, n));
    /* Unknown size8 tag and raw size21 tag: cursor is32 then40 then80/96. */
    n = base(q, 40);
    memcpy(p, q, 32);
    w32(p + 32, 77);
    w32(p + 36, 8);
    memcpy(p + 40, q + 32, n - 32);
    w32(p, (uint32_t)(n + 8));
    CHECK(kf_boot(p, n + 8, &out) && out.visited == 4 && out.next == n + 8);
    /* A valid total ends exactly after the end tag, even with a larger supplied
     * span. */
    CHECK(kf_boot(p, sizeof p, &out) && out.next == n + 8);
    /* Late duplicate required tags and missing exact end: output stays untouched.
     */
    n = base(q, 40);
    memcpy(p, q, n - 8);
    memcpy(p + n - 8, q + 8, 24);
    memcpy(p + n + 16, q + n - 8, 8);
    w32(p, (uint32_t)(n + 24));
    CHECK(refused(p, n + 24));
    memcpy(p, q, n - 8);
    memcpy(p + n - 8, q + 32, 56);
    memcpy(p + n + 48, q + n - 8, 8);
    w32(p, (uint32_t)(n + 56));
    CHECK(refused(p, n + 56));
    base(p, 40);
    w32(p, (uint32_t)(n - 8));
    CHECK(refused(p, n - 8));
    /* Optional cmdline: staged validation does not waive module/map/end. */
    memcpy(p, q, n - 8);
    w32(p + n - 8, 1);
    w32(p + n - 4, 13);
    memcpy(p + n, "mode", 5);
    memcpy(p + n + 8, q + n - 8, 8);
    w32(p, (uint32_t)(n + 16));
    CHECK(kf_boot(p, n + 16, &out) && out.command_length == 4 &&
          !memcmp(out.command, "mode", 4));
    p[n + 4] = 'x';
    CHECK(refused(p, n + 16));
    p[n + 4] = 0;
    w32(p + 8, 77);
    CHECK(refused(p, n + 16));
    puts("KF_PASS:tags:truncation-size7-size8-size21-stride40-unknown-exact-"
         "boundary-late-duplicate-overflow-optional");
    return 0;
}

static int reservations(void)
{
    unsigned char records[4][24] = {{0}}, permuted[4][24];
    cmk_boot b = {0};
    cmk_frames got, oracle, snapshot;
    const uint64_t lo[] = {0x300000, 0x320800, 0x330000, 0x2ff800},
                   len[] = {0x100000, 0x1000, 0x2000, 0x1800};
    const unsigned types[] = {1, 2, 1, 2};
    for (unsigned i = 0; i < 4; ++i)
    {
        w64(records[i], lo[i]);
        w64(records[i] + 8, len[i]);
        w32(records[i] + 16, types[i]);
    }
    struct kf_span reserve[] = {
        {0x340001, 0x342000}, {0x3ff000, 0x400000}, {0x300000, 0x301000}};
    b.map = (const unsigned char *)records;
    b.map_bytes = sizeof records;
    b.entry_size = 24;
    memset(&oracle, 0, sizeof oracle);
    /* Independent page oracle enumerates byte endpoints over this finite4MiB
     * domain. */
    for (unsigned page = 1; page < 1024; ++page)
    {
        uint64_t a = (uint64_t)page * 4096, z = a + 4095;
        int usable = 0, bad = 0;
        for (unsigned i = 0; i < 4; ++i)
        {
            if (types[i] == 1 && a >= lo[i] && z < lo[i] + len[i])
                usable = 1;
            if (types[i] != 1 && a < lo[i] + len[i] && z >= lo[i])
                bad = 1;
        }
        for (unsigned i = 0; i < 3; ++i)
            if (a < reserve[i].end && z >= reserve[i].start)
                bad = 1;
        oracle.state[page] = (unsigned char)(usable && !bad);
    }
    CHECK(kf_frames(&got, &b, reserve, 3) &&
          !memcmp(&got, &oracle, sizeof got));
    unsigned permutations = 0;
    for (unsigned a = 0; a < 4; ++a)
        for (unsigned c = 0; c < 4; ++c)
            for (unsigned d = 0; d < 4; ++d)
                for (unsigned e = 0; e < 4; ++e)
                    if (a != c && a != d && a != e && c != d && c != e &&
                        d != e)
                    {
                        unsigned order[] = {a, c, d, e};
                        for (unsigned i = 0; i < 4; ++i)
                            memcpy(permuted[i], records[order[i]], 24);
                        b.map = (unsigned char *)permuted;
                        CHECK(kf_frames(&got, &b, reserve, 3) &&
                              !memcmp(&got, &oracle, sizeof got));
                        ++permutations;
                    }
    CHECK(permutations == 24);
    b.map = (unsigned char *)records;
    reserve[1] = (struct kf_span){0x350000, 0x352000};
    CHECK(kf_frames(&got, &b, reserve, 3));
    CHECK(got.state[0x350] == 0 && got.state[0x351] == 0 &&
          got.state[0x3ff] == 1);
    snapshot = got;
    reserve[1] = (struct kf_span){10, 9};
    CHECK(!kf_frames(&got, &b, reserve, 3) &&
          !memcmp(&got, &snapshot, sizeof got));
    reserve[1] = (struct kf_span){0, 0};
    w64(records[3], UINT64_MAX);
    w64(records[3] + 8, 2);
    CHECK(!kf_frames(&got, &b, reserve, 3) &&
          !memcmp(&got, &snapshot, sizeof got));
    puts("KF_PASS:reservations:24-permutations-partial-pages-moving-module-"
         "transactional");
    return 0;
}
static struct kf_pool pool;

static void initpool(void)
{
    memset(&pool, 0, sizeof pool);
    for (unsigned i = 768; i < 1024; ++i)
        pool.frames.state[i] = 1;
    memset(pool.bytes, 0xa5, sizeof pool.bytes);
}

static int frames(void)
{
    uint32_t addresses[256];
    cmk_frames before;
    initpool();
    for (unsigned i = 0; i < 256; ++i)
    {
        addresses[i] = cmk_frame_take(&pool.frames);
        CHECK(addresses[i] == 0x300000 + 4096 * i);
        CHECK(kf_free(&pool) == 255 - i);
    }
    CHECK(!cmk_frame_take(&pool.frames));
    uint32_t invalid[] = {0, 0x300001, 0x2ff000, 0x400000, UINT32_MAX};
    for (unsigned i = 0; i < 5; ++i)
    {
        before = pool.frames;
        CHECK(cmk_frame_put(&pool.frames, invalid[i]) != 0 &&
              !memcmp(&before, &pool.frames, sizeof before));
    }
    for (unsigned i = 0; i < 256; ++i)
    {
        unsigned j = (i * 73) % 256;
        CHECK(cmk_frame_put(&pool.frames, addresses[j]) == 0);
        before = pool.frames;
        CHECK(cmk_frame_put(&pool.frames, addresses[j]) != 0 &&
              !memcmp(&before, &pool.frames, sizeof before));
        CHECK(kf_free(&pool) == i + 1);
    }
    initpool();
    for (unsigned i = 800; i < 809; ++i)
        pool.frames.state[i] = 0;
    unsigned seen = 0;
    for (uint32_t p; (p = cmk_frame_take(&pool.frames)) != 0;)
    {
        CHECK(p >= 0x300000 && p < 0x400000 &&
              (p < 800 * 4096u || p >= 809 * 4096u));
        ++seen;
    }
    CHECK(seen == 247);
    puts("KF_PASS:frames:256-exact-permuted-release-refusals-247-hole");
    return 0;
}

static int allocation(void)
{
    for (unsigned count = 1; count <= 16; ++count)
        for (unsigned fail = 0; fail <= count; ++fail)
        {
            initpool();
            struct kf_space s = {0}, before;
            s.live = 1;
            s.generation = 1;
            CHECK(kf_allocate(&pool, &s, 10, 1, 1));
            memset(pool.bytes[0], 0x73, 4096);
            before = s;
            size_t free = kf_free(&pool);
            cmk_frames state = pool.frames;
            int ok = kf_allocate(&pool, &s, 256, count, fail);
            CHECK(ok == (fail == count));
            if (!ok)
                CHECK(kf_free(&pool) == free &&
                      !memcmp(&before, &s, sizeof s) &&
                      !memcmp(&state, &pool.frames, sizeof state));
            else
            {
                CHECK(kf_free(&pool) == free - count);
                for (unsigned i = 0; i < count; ++i)
                {
                    CHECK(s.pages[256 + i].permissions == 7);
                    unsigned index = (s.pages[256 + i].frame - 0x300000) / 4096;
                    for (unsigned j = 0; j < 4096; ++j)
                        CHECK(pool.bytes[index][j] == 0);
                }
            }
            for (unsigned i = 0; i < 4096; ++i)
                CHECK(pool.bytes[0][i] == 0x73);
        }
    initpool();
    struct kf_space s = {0};
    s.live = 1;
    CHECK(kf_allocate(&pool, &s, 258, 1, 1));
    struct kf_space old = s;
    cmk_frames state = pool.frames;
    CHECK(!kf_allocate(&pool, &s, 256, 4, 4) && !memcmp(&s, &old, sizeof s) &&
          !memcmp(&state, &pool.frames, sizeof state));
    puts("KF_PASS:allocation:152-prefixes-zero-exposure-existing-mapping-"
         "preserved");
    return 0;
}

static int lifecycle(void)
{
    initpool();
    struct kf_space s = {0};
    s.live = 1;
    s.generation = 1;
    struct kf_cpu c = {0, 0, 0, 0};
    CHECK(kf_allocate(&pool, &s, 10, 3, 3));
    size_t free = kf_free(&pool);
    CHECK(!kf_reap_user(&pool, &s, 0, &c));
    s.live = 0;
    CHECK(!kf_reap_user(&pool, &s, 0, &c));
    c.current = 1;
    CHECK(!kf_reap_user(&pool, &s, 0, &c));
    c.tss_owner = 1;
    CHECK(!kf_reap_user(&pool, &s, 0, &c));
    CHECK(kf_free(&pool) == free);
    c.tss_owner = 0;
    c.cr3_owner = 1;
    CHECK(!kf_reap_user(&pool, &s, 0, &c));
    c.tss_owner = 1;
    CHECK(kf_reap_user(&pool, &s, 0, &c) && kf_free(&pool) == free + 3);
    CHECK(!kf_retire_slot(&s, 0, &c));
    c.stack_owner = 1;
    CHECK(kf_retire_slot(&s, 0, &c) && s.generation == 2);
    s.generation = UINT32_MAX;
    CHECK(kf_retire_slot(&s, 0, &c) && s.retired && s.generation == UINT32_MAX);
    puts(
        "KF_PASS:lifecycle:dead-peer-CR3-TSS-user-reap-stack-handoff-generation-"
        "retirement");
    return 0;
}

int main(int argc, char **argv)
{
    if (argc != 2)
        return 2;
    if (!strcmp(argv[1], "tags"))
        return tags();
    if (!strcmp(argv[1], "reservations"))
        return reservations();
    if (!strcmp(argv[1], "frames"))
        return frames();
    if (!strcmp(argv[1], "allocation"))
        return allocation();
    if (!strcmp(argv[1], "lifecycle"))
        return lifecycle();
    return 2;
}
