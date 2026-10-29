#include "owning_option.h"
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <time.h>
#define CHECK(x)                                                         \
    do                                                                   \
    {                                                                    \
        if (!(x))                                                        \
        {                                                                \
            fprintf(stderr, "ER_CHECK EOP_CHECK:%d:%s\n", __LINE__, #x); \
            return 1;                                                    \
        }                                                                \
    } while (0)

struct drop_observer
{
    struct v4_allocator base;
    struct eop_owner *owner;
    unsigned empty_seen;
};

static int32_t *observed_take(void *p, size_t n)
{
    struct drop_observer *o = p;
    return o->base.take(o->base.context, n);
}

static void observed_release(void *p, int32_t *cell)
{
    struct drop_observer *o = p;
    o->empty_seen += o->owner->tag == EOP_NONE;
    o->base.release(o->base.context, cell);
}

static int scalar(void)
{
    struct eop_scalar o = {1, 91};
    uint32_t value = 99;
    size_t offset = 88;
    CHECK(eop_scalar_make(0, 1, &o) == EOP_OK && o.tag == EOP_SOME &&
          eop_scalar_get(&o, &value) == EOP_OK && value == 0);
    CHECK(eop_scalar_make(UINT32_MAX, 0, &o) == EOP_OK && o.tag == EOP_NONE);
    value = 99;
    CHECK(eop_scalar_get(&o, &value) == EOP_ABSENT && value == 99);
    struct eop_scalar old = o;
    CHECK(eop_scalar_make(1, 2, &o) == EOP_INVALID &&
          !memcmp(&o, &old, sizeof o));
    o.tag = 9;
    CHECK(eop_scalar_get(&o, &value) == EOP_INVALID && value == 99);
    er_result r = er_ok(UINT32_MAX);
    CHECK(eop_result_get(r, &value, &offset) == EOP_OK && value == UINT32_MAX &&
          offset == 88);
    r = er_error(ER_IO, 7);
    CHECK(eop_result_get(r, &value, &offset) == EOP_ABSENT &&
          value == UINT32_MAX && offset == 7);
    r.tag = (er_code)99;
    CHECK(eop_result_get(r, &value, &offset) == EOP_INVALID &&
          value == UINT32_MAX && offset == 7);
    CHECK(eop_unit(EOP_UNIT) == EOP_OK &&
          eop_unit((enum eop_unit)1) == EOP_INVALID);
    puts(
        "EOP_SCALAR present-zero tagged-access unknown-tag no-inactive-member");
    return 0;
}

static int owning(void)
{
    struct ow_allocator pool = {0};
    struct v4_allocator a = ow_allocator(&pool);
    struct eop_owner source = {0}, clone = {0}, moved = {0};
    uint32_t id = 99;
    int32_t v = 88;
    CHECK(eop_create(&source, a, 0, -7) == EOP_OK && pool.attempts == 1 &&
          pool.live == 1);
    CHECK(eop_read(&source, &id, &v) == EOP_OK && id == 0 && v == -7);
    CHECK(eop_create(&source, a, 9, 1) == EOP_BUSY && pool.attempts == 1);
    CHECK(eop_clone(&source, &clone) == EOP_OK && pool.live == 2 &&
          clone.as.some.cell != source.as.some.cell);
    *clone.as.some.cell = 20;
    CHECK(eop_read(&source, &id, &v) == EOP_OK && v == -7);
    CHECK(eop_move(&moved, &clone) == EOP_OK && clone.tag == EOP_NONE &&
          pool.live == 2);
    CHECK(eop_move(&moved, &source) == EOP_BUSY && source.tag == EOP_SOME);
    CHECK(eop_move(&source, &source) == EOP_INVALID);
    CHECK(eop_drop(&source) == EOP_OK && pool.live == 1 && pool.releases == 1 &&
          source.tag == EOP_NONE);
    CHECK(eop_drop(&source) == EOP_OK && pool.releases == 1);
    CHECK(eop_drop(&moved) == EOP_OK && pool.live == 0 && pool.releases == 2 &&
          pool.invalid_releases == 0);
    struct drop_observer observer = {a, &source, 0};
    struct v4_allocator watched = {&observer, observed_take, observed_release};
    CHECK(eop_create(&source, watched, 0, 1) == EOP_OK &&
          eop_drop(&source) == EOP_OK && observer.empty_seen == 1 &&
          pool.live == 0 && pool.releases == 3);
    puts(
        "EOP_OWNER actual-T66-allocator zero-ID clone-independent move-consumes drop-once");
    return 0;
}

static int failure(void)
{
    struct ow_allocator pool = {0};
    struct v4_allocator a = ow_allocator(&pool);
    struct eop_owner source = {0}, out = {0};
    struct eop_owner old = out;
    pool.fail_at = 1;
    CHECK(eop_create(&out, a, 0, 5) == EOP_NOMEM &&
          !memcmp(&out, &old, sizeof out) && pool.live == 0 &&
          pool.attempts == 1);
    pool.fail_at = 0;
    CHECK(eop_create(&source, a, 0, 7) == EOP_OK);
    pool.fail_at = 3;
    CHECK(eop_clone(&source, &out) == EOP_NOMEM &&
          !memcmp(&out, &old, sizeof out) && pool.live == 1 &&
          source.as.some.id == 0 && *source.as.some.cell == 7);
    CHECK(eop_drop(&source) == EOP_OK && pool.live == 0 && pool.releases == 1);
    CHECK(eop_clone(&source, &out) == EOP_OK && out.tag == 0 &&
          pool.attempts == 3);
    pool.attempts = SIZE_MAX;
    CHECK(eop_create(&out, a, 0, 9) == EOP_NOMEM && pool.exhausted_counters &&
          pool.attempts == SIZE_MAX && pool.live == 0);
    puts(
        "EOP_FAILURE allocation-refusal unchanged-owners empty-no-allocation counter-exhaustion");
    return 0;
}

static int base(void)
{
    uint32_t out = 91;
    CHECK(er_optional_pack(0, 1, &out) == ER_RANGE && out == 91);
    CHECK(er_optional_pack(17, 0, &out) == ER_OK && out == 0);
    CHECK(er_optional_pack(UINT32_MAX, 1, &out) == ER_OK && out == UINT32_MAX);
    CHECK(er_optional_unpack(0).tag == ER_ABSENT);
    CHECK(er_error(ER_OK, 0).tag == ER_INVALID);
    puts("EOP_BASE actual-niche lost-zero complete-tag-distinction");
    return 0;
}

static int oracle(void)
{
    uint32_t value;
    unsigned present;
    while (scanf("%" SCNu32 " %u", &value, &present) == 2)
    {
        uint32_t niche = 91, read = 92;
        struct eop_scalar scalar = {1, 93};
        int ns = er_optional_pack(value, (int)present, &niche),
            ss = eop_scalar_make(value, present, &scalar),
            gs = eop_scalar_get(&scalar, &read);
        printf("%d %" PRIu32 " %d %u %" PRIu32 " %d %" PRIu32 "\n", ns, niche,
               ss, scalar.tag, scalar.value, gs, read);
    }
    return 0;
}

static int histories(void)
{
    unsigned n, fail;
    while (scanf("%u %u", &n, &fail) == 2)
    {
        struct ow_allocator pool = {0};
        pool.fail_at = fail;
        struct eop_owner owners[3] = {{0}};
        struct v4_allocator a = ow_allocator(&pool);
        CHECK(n <= 16);
        for (unsigned j = 0; j < n; ++j)
        {
            unsigned op, dst, src;
            CHECK(scanf("%u %u %u", &op, &dst, &src) == 3 && dst < 3 &&
                  src < 3);
            int status;
            uint32_t id = 99;
            int32_t value = 88;
            if (op == 0)
                status = eop_create(&owners[dst], a, src, (int32_t)src - 7);
            else if (op == 1)
                status = eop_clone(&owners[src], &owners[dst]);
            else if (op == 2)
                status = eop_move(&owners[dst], &owners[src]);
            else if (op == 3)
                status = eop_drop(&owners[dst]);
            else if (op == 4)
                status = eop_read(&owners[dst], &id, &value);
            else
                return 2;
            printf("%s%d %" PRIu32 " %" PRId32 " %zu %zu %zu", j ? " " : "",
                   status, id, value, pool.attempts, pool.live, pool.releases);
            for (unsigned k = 0; k < 3; ++k)
            {
                unsigned block = 8;
                if (owners[k].tag == EOP_SOME)
                    for (unsigned z = 0; z < 8; ++z)
                        if (owners[k].as.some.cell == pool.storage[z])
                            block = z;
                printf(" %u %" PRIu32 " %" PRId32 " %u", owners[k].tag,
                       owners[k].tag ? owners[k].as.some.id : 0,
                       owners[k].tag ? *owners[k].as.some.cell : 0, block);
            }
        }
        for (unsigned k = 0; k < 3; ++k)
            CHECK(eop_drop(&owners[k]) == EOP_OK);
        printf(" %zu %zu %u\n", pool.live, pool.releases,
               pool.invalid_releases);
    }
    return 0;
}

static int metrics(void)
{
    double times[2][7];
    uint64_t checksum = 0;
    size_t attempts = 0, releases = 0;
    for (unsigned mode = 0; mode < 2; ++mode)
        for (unsigned s = 0; s < 7; ++s)
        {
            struct ow_allocator pool = {0};
            struct v4_allocator a = ow_allocator(&pool);
            clock_t start = clock();
            CHECK(start != (clock_t)-1);
            for (unsigned i = 0; i < 10000; ++i)
            {
                if (mode)
                {
                    struct eop_owner owner = {0};
                    CHECK(eop_create(&owner, a, 0, 7) == EOP_OK);
                    checksum += (unsigned)*owner.as.some.cell;
                    CHECK(eop_drop(&owner) == EOP_OK);
                }
                else
                {
                    struct eop_scalar value;
                    CHECK(eop_scalar_make(0, 1, &value) == EOP_OK);
                    checksum += value.tag;
                }
            }
            times[mode][s] = (double)(clock() - start) / CLOCKS_PER_SEC;
            CHECK(pool.live == 0 && pool.invalid_releases == 0);
            attempts += pool.attempts;
            releases += pool.releases;
        }
    printf(
        "{\"iterations\":10000,\"checksum\":%" PRIu64
        ",\"allocation_attempts\":%zu,\"releases\":%zu,\"scalar_size\":%zu,\"scalar_align\":%zu,\"owner_size\":%zu,\"owner_align\":%zu,\"result_size\":%zu,\"result_align\":%zu,\"pointer_size\":%zu",
        checksum, attempts, releases, sizeof(struct eop_scalar),
        _Alignof(struct eop_scalar), sizeof(struct eop_owner),
        _Alignof(struct eop_owner), sizeof(er_result), _Alignof(er_result),
        sizeof(void *));
    for (unsigned m = 0; m < 2; ++m)
    {
        printf(",\"%s_seconds\":[", m ? "owner" : "scalar");
        for (unsigned s = 0; s < 7; ++s)
            printf("%s%.9f", s ? "," : "", times[m][s]);
        putchar(']');
    }
    puts("}");
    return 0;
}

int main(int argc, char **argv)
{
    if (argc != 2)
        return 2;
    if (!strcmp(argv[1], "option_scalar"))
        return scalar();
    if (!strcmp(argv[1], "option_owner"))
        return owning();
    if (!strcmp(argv[1], "option_failure"))
        return failure();
    if (!strcmp(argv[1], "option_base"))
        return base();
    if (!strcmp(argv[1], "oracle"))
        return oracle();
    if (!strcmp(argv[1], "histories"))
        return histories();
    if (!strcmp(argv[1], "metrics"))
        return metrics();
    return 2;
}
