#include "authority_boundary.h"
#include <limits.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
_Static_assert(UINT_MAX == UINT32_MAX,
               "original defect comparison uses 32-bit unsigned");
#define CHECK(x)                                                            \
    do                                                                      \
    {                                                                       \
        if (!(x))                                                           \
        {                                                                   \
            fprintf(stderr, "OW_CHECK %s:%d %s\n", __FILE__, __LINE__, #x); \
            exit(1);                                                        \
        }                                                                   \
    } while (0)

static struct ow_tenant_arena arena(void)
{
    struct ow_tenant_arena a;
    for (unsigned i = 0; i < 16; i++)
    {
        a.data[i] = (int32_t)(i + 90);
    }
    return a;
}

static int equal(struct ow_tenant_arena a, struct ow_tenant_arena b)
{
    for (unsigned i = 0; i < 16; i++)
    {
        if (a.data[i] != b.data[i])
        {
            return 0;
        }
    }
    return 1;
}

static struct oaf_view setup(struct oaf_root *root, struct ow_tenant_arena *a)
{
    struct oaf_view v;
    CHECK(oaf_init(root, a, 0, 3) == OAF_OK);
    CHECK(oaf_view(root, &v) == OAF_OK);
    return v;
}

static void defect(void)
{
    struct ow_tenant_arena a = arena(), old = a;
    CHECK(ow_flawed_write(&a, 0, UINT_MAX - 3, 8, 7) == OW_OK);
    for (unsigned i = 0; i < 16; i++)
    {
        CHECK(a.data[i] == ((i < 4 || i >= 12) ? 7 : old.data[i]));
    }
    a = old;
    CHECK(ow_checked_write(&a, 0, UINT_MAX - 3, 8, 7) == OW_INVALID);
    CHECK(equal(a, old));
    CHECK(ow_checked_write(&a, 0, 8, 1, 7) == OW_INVALID);
    CHECK(equal(a, old));
    CHECK(ow_checked_write(&a, 0, 8, 0, 7) == OW_OK);
    struct oaf_root root;
    struct oaf_view v = setup(&root, &a);
    CHECK(oaf_write(&v, (struct oaf_request){1, UINT32_MAX - 3, 8, 0, 2}, 7) ==
          OAF_INVALID);
    CHECK(equal(a, old));
    puts("OAF_DEFECT original-defined-logical-violation repaired-unchanged");
}

static void bounds(void)
{
    for (unsigned tenant = 0; tenant < 3; tenant++)
    {
        for (unsigned offset = 0; offset < 11; offset++)
        {
            for (unsigned count = 0; count < 11; count++)
            {
                struct ow_tenant_arena a = arena(), old = a;
                enum ow_status s =
                    ow_checked_write(&a, tenant, offset, count, 7);
                int ok = tenant < 2 && offset <= 8 && count <= 8 - offset;
                CHECK((s == OW_OK) == ok);
                for (unsigned i = 0; i < 16; i++)
                {
                    CHECK(a.data[i] == ((ok && i >= tenant * 8 + offset &&
                                         i < tenant * 8 + offset + count)
                                            ? 7
                                            : old.data[i]));
                }
            }
        }
    }
    struct ow_tenant_arena a = arena();
    struct oaf_root r;
    struct oaf_view v = setup(&r, &a);
    struct oaf_view child;
    CHECK(oaf_subview(&v, 8, 0, 1, &child) == OAF_OK);
    int32_t out[8] = {9};
    CHECK(oaf_read(&child, (struct oaf_request){1, 0, 0, 0, 1}, out) == OAF_OK);
    CHECK(out[0] == 9);
    CHECK(oaf_subview(&v, UINT32_MAX, 1, 1, &child) == OAF_INVALID);
    puts("OAF_BOUNDS 363-original-combinations empty-one-past");
}

static void wire(void)
{
    struct oaf_request req = {UINT32_C(0x01020304), UINT32_C(0xaabbccdd),
                              UINT32_MAX, 1, 3},
                       out = {9, 9, 9, 9, 9};
    unsigned char b[16];
    CHECK(oaf_encode(req, b) == OAF_OK);
    const unsigned char expected[16] = {
        1, 1, 3, 0, 1, 2, 3, 4, 0xaa, 0xbb, 0xcc, 0xdd, 0xff, 0xff, 0xff, 0xff};
    CHECK(!memcmp(b, expected, 16));
    CHECK(oaf_decode(b, 16, &out) == OAF_OK);
    CHECK(out.epoch == req.epoch && out.offset == req.offset &&
          out.count == req.count && out.tenant == 1 && out.rights == 3);
    for (unsigned n = 0; n < 18; n++)
    {
        if (n != 16)
        {
            out = (struct oaf_request){9, 9, 9, 9, 9};
            CHECK(oaf_decode(b, n, &out) == OAF_INVALID);
            CHECK(out.epoch == 9 && out.tenant == 9);
        }
    }
    for (unsigned field = 0; field < 4; field++)
    {
        unsigned char save = b[field];
        b[field] = 255;
        out = (struct oaf_request){9, 9, 9, 9, 9};
        CHECK(oaf_decode(b, 16, &out) == OAF_INVALID);
        CHECK(out.epoch == 9 && out.offset == 9 && out.count == 9 &&
              out.tenant == 9 && out.rights == 9);
        b[field] = save;
    }

    /* Decoding all input before publication supports overlap with the output's
       character representation; array object has sufficient storage/alignment. */
    union
    {
        struct oaf_request r;
        unsigned char bytes[sizeof(struct oaf_request) + 16];
    } u;

    memcpy(u.bytes, b, 16);
    CHECK(oaf_decode(u.bytes, 16, &u.r) == OAF_OK);
    CHECK(u.r.offset == req.offset);
    puts(
        "OAF_WIRE exact16-big-endian version-reserved length-refusal staged-overlap");
}

static void rights(void)
{
    struct ow_tenant_arena a = arena(), old = a;
    struct oaf_root r;
    struct oaf_view v = setup(&r, &a), ro, nested, unchanged = {0};
    CHECK(oaf_subview(&v, 2, 4, 1, &ro) == OAF_OK);
    CHECK(oaf_subview(&ro, 1, 2, 1, &nested) == OAF_OK);
    CHECK(nested.offset == 3 && nested.count == 2);
    CHECK(oaf_subview(&ro, 0, 1, 3, &unchanged) == OAF_DENIED);
    CHECK(unchanged.root == NULL);
    CHECK(oaf_write(&ro, (struct oaf_request){1, 0, 1, 0, 2}, 7) == OAF_DENIED);
    CHECK(equal(a, old));
    int32_t out[8] = {-9, -9, -9, -9, -9, -9, -9, -9};
    CHECK(oaf_read(&nested, (struct oaf_request){1, 0, 2, 0, 1}, out) ==
          OAF_OK);
    CHECK(out[0] == 93 && out[1] == 94 && out[2] == -9);
    CHECK(oaf_read(&v, (struct oaf_request){1, 0, 0, 0, 2}, out) == OAF_DENIED);
    CHECK(oaf_write(&v, (struct oaf_request){1, 0, 0, 0, 1}, 7) == OAF_DENIED);
    CHECK(oaf_write(&v, (struct oaf_request){1, 0, 1, 1, 2}, 7) == OAF_DENIED);
    CHECK(equal(a, old));
    puts("OAF_RIGHTS nested-read-only no-widening foreign-tenant-denied");
}

static void revoke(void)
{
    struct ow_tenant_arena a = arena(), old = a;
    struct oaf_root r;
    struct oaf_view v = setup(&r, &a), fresh;
    CHECK(oaf_revoke(&r) == OAF_OK);
    CHECK(oaf_write(&v, (struct oaf_request){1, 0, 1, 0, 2}, 7) == OAF_STALE);
    CHECK(equal(a, old));
    CHECK(oaf_view(&r, &fresh) == OAF_OK);
    CHECK(oaf_write(&fresh, (struct oaf_request){1, 0, 1, 0, 2}, 7) ==
          OAF_STALE);
    CHECK(oaf_write(&fresh, (struct oaf_request){2, 0, 1, 0, 2}, 7) == OAF_OK);
    r.epoch = UINT32_MAX;
    CHECK(oaf_view(&r, &fresh) == OAF_OK);
    CHECK(oaf_revoke(&r) == OAF_OK && r.retired == 1 && r.epoch == UINT32_MAX);
    CHECK(oaf_write(&fresh, (struct oaf_request){UINT32_MAX, 0, 1, 0, 2}, 8) ==
          OAF_STALE);
    CHECK(oaf_view(&r, &v) == OAF_STALE);
    CHECK(oaf_revoke(&r) == OAF_FULL && r.epoch == UINT32_MAX);
    puts("OAF_REVOCATION stale-root stale-wire MAX-retirement");
}

static void atomicity(void)
{
    struct ow_tenant_arena a = arena(), old = a;
    struct oaf_root r;
    struct oaf_view v = setup(&r, &a);
    unsigned char b[16];
    int32_t out[8];
    for (unsigned i = 0; i < 8; i++)
    {
        out[i] = -9;
    }
    CHECK(oaf_encode((struct oaf_request){1, 7, 2, 0, 3}, b) == OAF_OK);
    CHECK(oaf_wire_write(&v, b, 16, 7) == OAF_INVALID);
    CHECK(oaf_wire_read(&v, b, 16, out) == OAF_INVALID);
    CHECK(equal(a, old));
    for (unsigned i = 0; i < 8; i++)
    {
        CHECK(out[i] == -9);
    }
    CHECK(oaf_encode((struct oaf_request){1, 2, 3, 0, 3}, b) == OAF_OK);
    CHECK(oaf_wire_read(&v, b, 16, out) == OAF_OK);
    CHECK(out[0] == 92 && out[2] == 94 && out[3] == -9);
    CHECK(oaf_wire_write(&v, b, 16, -7) == OAF_OK);
    for (unsigned i = 0; i < 16; i++)
    {
        CHECK(a.data[i] == ((i >= 2 && i < 5) ? -7 : old.data[i]));
    }
    CHECK(
        oaf_decode(NULL, 16, (struct oaf_request *)&(struct oaf_request){0}) ==
        OAF_INVALID);
    CHECK(oaf_wire_read(&v, b, 16, NULL) == OAF_INVALID);
    puts("OAF_ATOMIC foreign-revalidation unchanged-refusal one-publication");
}

static void oracle(void)
{
    unsigned tenant, parent_t, rights, parent_rights, op;
    uint32_t epoch, offset, count, begin, length;
    while (scanf("%u %u %u %u %u %" SCNu32 " %" SCNu32 " %" SCNu32 " %" SCNu32
                 " %" SCNu32,
                 &parent_t, &tenant, &parent_rights, &rights, &op, &epoch,
                 &offset, &count, &begin, &length) == 10)
    {
        struct ow_tenant_arena a = arena();
        struct oaf_root r;
        CHECK(oaf_init(&r, &a, parent_t, 3) == OAF_OK);
        struct oaf_view full, parent;
        CHECK(oaf_view(&r, &full) == OAF_OK);
        CHECK(oaf_subview(&full, begin, length, parent_rights, &parent) ==
              OAF_OK);
        struct oaf_request req = {epoch, offset, count, tenant, rights};
        int32_t out[8];
        for (unsigned i = 0; i < 8; i++)
        {
            out[i] = -9;
        }
        enum oaf_status s =
            op ? oaf_write(&parent, req, 7) : oaf_read(&parent, req, out);
        printf("%u", (unsigned)s);
        for (unsigned i = 0; i < 16; i++)
        {
            printf(",%" PRId32, a.data[i]);
        }
        for (unsigned i = 0; i < 8; i++)
        {
            printf(",%" PRId32, out[i]);
        }
        putchar('\n');
    }
}

static void metrics(void)
{
    double times[7];
    uint64_t sum = 0;
    for (unsigned sample = 0; sample < 7; sample++)
    {
        clock_t start = clock();
        struct ow_tenant_arena a = arena();
        struct oaf_root r;
        struct oaf_view v = setup(&r, &a);
        unsigned char b[16];
        CHECK(oaf_encode((struct oaf_request){1, 0, 8, 0, 3}, b) == OAF_OK);
        for (unsigned i = 0; i < 10000; i++)
        {
            CHECK(oaf_wire_write(&v, b, 16, 7) == OAF_OK);
            int32_t out[8];
            CHECK(oaf_wire_read(&v, b, 16, out) == OAF_OK);
            sum += (uint64_t)out[7];
        }
        times[sample] = (double)(clock() - start) / CLOCKS_PER_SEC;
    }
    printf(
        "{\"iterations\":10000,\"checksum\":%" PRIu64
        ",\"wire_bytes\":16,\"root_size\":%zu,\"view_size\":%zu,\"seconds\":[",
        sum, sizeof(struct oaf_root), sizeof(struct oaf_view));
    for (unsigned i = 0; i < 7; i++)
    {
        printf("%s%.9f", i ? "," : "", times[i]);
    }
    puts("]}");
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        return 2;
    }
    if (!strcmp(argv[1], "authority_defect"))
    {
        defect();
    }
    else if (!strcmp(argv[1], "authority_bounds"))
    {
        bounds();
    }
    else if (!strcmp(argv[1], "authority_wire"))
    {
        wire();
    }
    else if (!strcmp(argv[1], "authority_rights"))
    {
        rights();
    }
    else if (!strcmp(argv[1], "authority_revoke"))
    {
        revoke();
    }
    else if (!strcmp(argv[1], "authority_atomic"))
    {
        atomicity();
    }
    else if (!strcmp(argv[1], "oracle"))
    {
        oracle();
    }
    else if (!strcmp(argv[1], "metrics"))
    {
        metrics();
    }
    else
    {
        return 2;
    }
    return 0;
}
