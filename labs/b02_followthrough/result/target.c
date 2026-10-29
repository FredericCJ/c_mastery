#include "result.h"
#include "old_api.h"

static void say(const char *s)
{
    register unsigned r0 __asm__("r0") = 4;
    register const char *r1 __asm__("r1") = s;
    __asm__ volatile("bkpt 0xab" : "+r"(r0) : "r"(r1) : "memory");
}

_Noreturn static void finish(unsigned status)
{
    unsigned args[2];
    args[0] = 0x20026;
    args[1] = status;
    register unsigned r0 __asm__("r0") = 0x20;
    register void *r1 __asm__("r1") = args;
    __asm__ volatile("bkpt 0xab" : "+r"(r0) : "r"(r1) : "memory");
    for (;;)
    {
    }
}

#define TEST(x)                              \
    do                                       \
    {                                        \
        if (!(x))                            \
        {                                    \
            say("BF_TARGET_CHECK " #x "\n"); \
            finish(1);                       \
        }                                    \
    } while (0)

struct bf_heap
{
    unsigned live, closes;
    unsigned char data[8];
};

static void *alloc(void *p, size_t n)
{
    struct bf_heap *h = p;
    TEST(n <= 8 && !h->live);
    h->live = 1;
    return h->data;
}

static void drop(void *p, void *b)
{
    struct bf_heap *h = p;
    TEST(b == h->data && h->live);
    h->live = 0;
    ++h->closes;
}

void bf_target(void)
{
    struct b2_result r;
    TEST(bf_number(-123, 0, &r) == B2_OK && r.tag == 0);
    unsigned char bytes[260];
    for (unsigned i = 0; i < 260; ++i)
    {
        bytes[i] = 165;
    }
    size_t n = 77;
    TEST(b2_render(&r, bytes, 3, &n) == B2_FULL && n == 77 && bytes[0] == 165);
    TEST(b2_render(&r, bytes, 4, &n) == B2_OK && n == 4 && bytes[0] == 45);
    struct bf_heap h = {0};
    struct bf_value a, b;
    bf_value_init(&a);
    bf_value_init(&b);
    unsigned char payload[] = {97, 0, 98};
    TEST(bf_clone(&a, payload, 3, alloc, drop, &h) == B2_OK);
    TEST(bf_move(&b, &a) == B2_OK && a.tag == BF_EMPTY);
    struct bf_bytes owned = {0};
    TEST(bf_take(&b, &owned) == B2_OK && b.tag == BF_EMPTY);
    bf_reset(&a);
    bf_reset(&b);
    TEST(h.closes == 0);
    bf_bytes_drop(&owned);
    bf_bytes_drop(&owned);
    TEST(h.closes == 1 && !h.live);
    struct bf_wire wire = {0}, decoded = {0};
    wire.version = 1;
    wire.tag = 2;
    wire.number = -9999;
    TEST(bf_encode(&wire, bytes, 260, &n) == B2_OK);
    unsigned char out[8];
    size_t written = 0;
    TEST(bf_old_render_wire(bytes, n, out, 8, &written) == B2_OK &&
         written == 5);
    TEST(bf_decode(bytes, n, &decoded) == B2_OK && decoded.number == -9999);
    wire.version = 2;
    wire.tag = 3;
    wire.boolean = 1;
    TEST(bf_encode(&wire, bytes, 260, &n) == B2_OK &&
         bf_decode(bytes, n, &decoded) == B2_OK && decoded.boolean == 1);
    written = 77;
    TEST(bf_old_render_wire(bytes, n, out, 8, &written) == B2_INVALID &&
         written == 77);
    bytes[4] = 2;
    TEST(bf_decode(bytes, n, &decoded) == B2_INVALID);
    say("BF_TARGET_PASS C17 M4 actual-B02 ownership old-object wire-version\n");
    finish(0);
}
