#include "test_support.h"
#include "unicode_stream.h"
#include <time.h>

static int equal(struct adu_stream a, struct adu_stream b)
{
    return a.used == b.used && a.bytes == b.bytes && a.scalars == b.scalars &&
           a.phase == b.phase && a.pending[0] == b.pending[0] &&
           a.pending[1] == b.pending[1] && a.pending[2] == b.pending[2];
}

static int chunks(void)
{
    unsigned char input[] = {0x41, 0xe2, 0x82, 0xac, 0xf4, 0x8f, 0xbf, 0xbf};
    for (unsigned split = 0; split <= sizeof input; ++split)
    {
        struct adu_stream s;
        adu_init(&s);
        struct adu_summary out = {99, 98};
        CHECK(adu_feed(&s, input, split) == ADU_OK);
        CHECK(s.used <= 3);
        CHECK(adu_feed(&s, input + split, sizeof input - split) == ADU_OK);
        CHECK(adu_finish(&s, &out) == ADU_OK && out.bytes == 8 &&
              out.scalars == 3);
    }
    struct adu_stream s;
    adu_init(&s);
    unsigned char first[] = {0xe2, 0x82};
    CHECK(adu_feed(&s, first, 2) == ADU_OK && s.used == 2 && s.bytes == 2 &&
          s.scalars == 0);
    first[0] = first[1] =
        0xff; /* retained bytes are copies, not borrowed chunks */
    unsigned char last = 0xac;
    CHECK(adu_feed(&s, &last, 1) == ADU_OK && s.scalars == 1 && s.used == 0);
    struct adu_stream old = s;
    unsigned char bad[] = {0x41, 0xff};
    CHECK(adu_feed(&s, bad, 2) == ADU_UTF8 && equal(s, old));
    unsigned char corrected[] = {0x41, 0x42};
    CHECK(adu_feed(&s, corrected, 2) == ADU_OK && s.bytes == 5 &&
          s.scalars == 3);
    old = s;
    CHECK(adu_feed(&s, NULL, 1) == ADU_INVALID && equal(s, old));
    CHECK(adu_feed(&s, NULL, 0) == ADU_OK && equal(s, old));
    puts("ADU_CHUNKS copied-pending whole-chunk-rollback");
    return 0;
}

static int finalization(void)
{
    struct adu_stream s;
    adu_init(&s);
    struct adu_summary out = {99, 98};
    unsigned char head[] = {0xf0, 0x90, 0x80};
    CHECK(adu_feed(&s, head, 3) == ADU_OK && s.used == 3);
    struct adu_stream old = s;
    CHECK(adu_finish(&s, &out) == ADU_UTF8 && out.bytes == 99 &&
          out.scalars == 98 && equal(s, old));
    CHECK(adu_cancel(&s) == ADU_OK && s.phase == ADU_CANCELLED && s.used == 0 &&
          s.bytes == 0 && s.scalars == 0 && s.pending[0] == 0 &&
          s.pending[1] == 0 && s.pending[2] == 0);
    CHECK(adu_finish(&s, &out) == ADU_STATE && out.bytes == 99);
    CHECK(adu_feed(&s, NULL, 0) == ADU_STATE);
    adu_init(&s);
    CHECK(adu_finish(&s, &out) == ADU_OK && out.bytes == 0 && out.scalars == 0);
    CHECK(adu_finish(&s, &out) == ADU_OK);
    CHECK(adu_feed(&s, NULL, 0) == ADU_STATE);
    adu_init(&s);
    unsigned char ascii[65535];
    for (size_t i = 0; i < sizeof ascii; ++i)
    {
        ascii[i] = 'a';
    }
    CHECK(adu_feed(&s, ascii, sizeof ascii) == ADU_OK && s.bytes == 65535 &&
          s.scalars == 65535);
    old = s;
    CHECK(adu_feed(&s, ascii, 1) == ADU_RANGE && equal(s, old));
    CHECK(adu_finish(&s, &out) == ADU_OK && out.bytes == 65535);
    CHECK(adu_finish(NULL, &out) == ADU_INVALID &&
          adu_cancel(NULL) == ADU_INVALID);
    for (unsigned pending = 1; pending <= 3; ++pending)
    {
        adu_init(&s);
        CHECK(adu_feed(&s, head, pending) == ADU_OK && s.used == pending);
        CHECK(adu_cancel(&s) == ADU_OK && s.used == 0 && s.bytes == 0 &&
              s.scalars == 0);
        old = s;
        CHECK(adu_cancel(&s) == ADU_OK && equal(s, old));
        adu_init(&s);
        unsigned char euro[] = {0xe2, 0x82, 0xac};
        for (unsigned i = 0; i < 3; ++i)
        {
            CHECK(adu_feed(&s, euro + i, 1) == ADU_OK);
        }
        CHECK(adu_finish(&s, &out) == ADU_OK && out.bytes == 3 &&
              out.scalars == 1);
        CHECK(adu_cancel(&s) == ADU_OK && s.used == 0 && s.bytes == 0);
    }
    adu_init(&s);
    CHECK(adu_feed(&s, ascii, 65534) == ADU_OK);
    CHECK(adu_feed(&s, head, 1) == ADU_OK && s.bytes == 65535 && s.used == 1);
    old = s;
    CHECK(adu_feed(&s, head + 1, 1) == ADU_RANGE && equal(s, old));
    out = (struct adu_summary){99, 98};
    CHECK(adu_finish(&s, &out) == ADU_UTF8 && out.bytes == 99 && equal(s, old));
    CHECK(adu_cancel(&s) == ADU_OK && s.bytes == 0);
    puts("ADU_FINALIZE incomplete-refusal cancel-discard length-boundary");
    return 0;
}

static int owner(void)
{
    struct adp_counter c = {0};
    struct adu_owner o;
    adu_owner_init(&o);
    struct ad_text view;
    unsigned char input[] = {0xe2, 0x82, 0xac};
    CHECK(adu_owner_set(&o, input, 3, operations(&c)) == ADU_OK && c.live == 1);
    CHECK(adu_owner_view(&o, &view) == ADU_OK && view.length == 3 &&
          view.scalars == 1 && view.bytes != input);
    input[0] = 0xff;
    struct ad_text check;
    CHECK(!ad_text(input, 3, &check) &&
          ad_text(view.bytes, view.length, &check) && check.scalars == 1);
    const unsigned char *saved = view.bytes;
    size_t calls = c.calls;
    CHECK(adu_owner_set(&o, input, 3, operations(&c)) == ADU_UTF8 &&
          c.calls == calls);
    CHECK(adu_owner_view(&o, &view) == ADU_OK && view.bytes == saved &&
          view.scalars == 1);
    CHECK(adu_owner_set(&o, NULL, 1, operations(&c)) == ADU_INVALID &&
          c.calls == calls);
    CHECK(adu_owner_set(&o, input, 65536, operations(&c)) == ADU_RANGE &&
          c.calls == calls);
    CHECK(adu_owner_set(&o, input, 3, (struct ado_ops){NULL, drop_bytes, &c}) ==
              ADU_INVALID &&
          c.calls == calls);
    CHECK(adu_owner_view(&o, &view) == ADU_OK && view.bytes == saved &&
          view.scalars == 1);
    unsigned char next[] = {0x41, 0x42};
    c.fail = c.calls + 1;
    CHECK(adu_owner_set(&o, next, 2, operations(&c)) == ADU_NOMEM &&
          c.live == 1 && c.drops == 0);
    CHECK(adu_owner_view(&o, &view) == ADU_OK && view.bytes == saved &&
          view.scalars == 1);
    c.fail = c.calls + 1;
    c.partial = 1;
    CHECK(adu_owner_set(&o, next, 2, operations(&c)) == ADU_NOMEM &&
          c.partial_cleanups == 1 && c.live == 1);
    c.fail = 0;
    CHECK(adu_owner_set(&o, view.bytes, view.length, operations(&c)) ==
              ADU_OK &&
          c.live == 1 && c.drops == 1);
    CHECK(adu_owner_view(&o, &view) == ADU_OK && view.scalars == 1 &&
          view.bytes[0] == 0xe2);
    CHECK(adu_owner_set(&o, NULL, 0, operations(&c)) == ADU_OK && c.live == 1 &&
          c.drops == 2);
    CHECK(adu_owner_view(&o, &view) == ADU_OK && view.bytes == NULL &&
          view.length == 0 && view.scalars == 0);
    adu_owner_drop(&o);
    CHECK(c.live == 0 && c.drops == 3);
    adu_owner_drop(&o);
    CHECK(c.drops == 3);
    puts("ADU_OWNER actual-owning-sum failure-preservation "
         "mutable-source-isolation");
    return 0;
}

static int original(void)
{
    unsigned char valid[] = {0x41, 0xe2, 0x82, 0xac, 0xf4, 0x8f, 0xbf, 0xbf};
    struct ad_text view;
    CHECK(ad_text(valid, 8, &view) && view.length == 8 && view.scalars == 3);
    unsigned char overlong[] = {0xc0, 0x80}, surrogate[] = {0xed, 0xa0, 0x80};
    struct ad_text old = view;
    CHECK(!ad_text(overlong, 2, &view) && view.bytes == old.bytes &&
          view.scalars == 3);
    CHECK(!ad_text(surrogate, 3, &view) && view.bytes == old.bytes &&
          view.length == 8);
    valid[0] = 0xff;
    CHECK(!ad_text(
        view.bytes, view.length,
        &old)); /* live alias invalidates encoding, no dangling access */
    CHECK(ad_text(NULL, 0, &view) && view.scalars == 0);
    unsigned char composed[] = {0xc3, 0xa9}, decomposed[] = {0x65, 0xcc, 0x81};
    CHECK(ad_text(composed, 2, &view) && view.scalars == 1);
    CHECK(ad_text(decomposed, 3, &view) && view.scalars == 2);
    puts("ADU_BASE actual-ad-text mutable-alias-counterexample");
    return 0;
}

static int oracle(void)
{
    char hex[33];
    unsigned mask;
    while (scanf("%32s %u", hex, &mask) == 2)
    {
        unsigned char bytes[16];
        size_t n = strcmp(hex, "-") ? strlen(hex) / 2 : 0;
        for (size_t i = 0; i < n; ++i)
        {
            unsigned v;
            if (sscanf(hex + 2 * i, "%2x", &v) != 1)
            {
                return 2;
            }
            bytes[i] = (unsigned char)v;
        }
        struct ad_text borrowed = {NULL, 77, 78};
        int b = ad_text(bytes, n, &borrowed);
        printf("%d %zu %zu ", b, borrowed.length, borrowed.scalars);
        struct adu_stream s;
        adu_init(&s);
        size_t last = 0;
        for (size_t i = 0; i <= n; ++i)
        {
            if (mask & (1u << i))
            {
                printf("%u", (unsigned)adu_feed(&s, bytes + last, i - last));
                last = i;
            }
        }
        printf("%u ", (unsigned)adu_feed(&s, bytes + last, n - last));
        struct adu_summary out = {77, 78};
        enum adu_status result = adu_finish(&s, &out);
        printf("%u %zu %zu %u ", (unsigned)s.phase, s.bytes, s.scalars, s.used);
        if (!s.used)
        {
            printf("-");
        }
        else
        {
            for (unsigned i = 0; i < s.used; ++i)
            {
                printf("%02x", s.pending[i]);
            }
        }
        printf(" %u %zu %zu\n", (unsigned)result, out.bytes, out.scalars);
    }
    return 0;
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        return 2;
    }
    atexit(cleanup_allocations);
    if (!strcmp(argv[1], "utf8_chunks"))
    {
        return chunks();
    }
    if (!strcmp(argv[1], "utf8_finalization"))
    {
        return finalization();
    }
    if (!strcmp(argv[1], "utf8_owner"))
    {
        return owner();
    }
    if (!strcmp(argv[1], "utf8_base"))
    {
        return original();
    }
    if (!strcmp(argv[1], "oracle"))
    {
        return oracle();
    }
    if (!strcmp(argv[1], "metrics"))
    {
        unsigned char input[] = {0x41, 0xe2, 0x82, 0xac,
                                 0xf4, 0x8f, 0xbf, 0xbf};
        double seconds[7];
        for (unsigned run = 0; run < 7; ++run)
        {
            clock_t start = clock();
            size_t sum = 0;
            for (unsigned i = 0; i < 10000; ++i)
            {
                struct adu_stream s;
                adu_init(&s);
                struct adu_summary r;
                for (unsigned j = 0; j < 8; ++j)
                {
                    CHECK(adu_feed(&s, input + j, 1) == ADU_OK);
                }
                CHECK(adu_finish(&s, &r) == ADU_OK);
                sum += r.scalars;
            }
            CHECK(sum == 30000);
            seconds[run] = (double)(clock() - start) / CLOCKS_PER_SEC;
        }
        printf("{\"stream_size\":%zu,\"owner_size\":%zu,\"retained_bytes\":%zu,"
               "\"iterations\":10000,\"checksum\":30000,\"seconds\":[",
               sizeof(struct adu_stream), sizeof(struct adu_owner),
               sizeof(((struct adu_stream *)0)->pending));
        for (unsigned i = 0; i < 7; ++i)
        {
            printf("%s%.9f", i ? "," : "", seconds[i]);
        }
        puts("]}");
        return 0;
    }
    return 2;
}
