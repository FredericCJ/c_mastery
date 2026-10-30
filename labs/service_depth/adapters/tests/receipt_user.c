#include "kernel_abi.h"
#include "receipt.h"

/* Original byte helpers close the freestanding store's declared string API. */
void *memset(void *p, int c, size_t n)
{
    unsigned char *a = p;
    while (n--)
    {
        *a++ = (unsigned char)c;
    }
    return p;
}

void *memcpy(void *p, const void *q, size_t n)
{
    unsigned char *a = p;
    const unsigned char *b = q;
    while (n--)
    {
        *a++ = *b++;
    }
    return p;
}

void *memmove(void *p, const void *q, size_t n)
{
    unsigned char *a = p;
    const unsigned char *b = q;
    if ((uintptr_t)a < (uintptr_t)b)
    {
        while (n--)
        {
            *a++ = *b++;
        }
    }
    else
    {
        while (n)
        {
            n--;
            a[n] = b[n];
        }
    }
    return p;
}

int memcmp(const void *p, const void *q, size_t n)
{
    const unsigned char *a = p;
    const unsigned char *b = q;
    while (n--)
    {
        if (*a != *b)
        {
            return *a < *b ? -1 : 1;
        }
        a++;
        b++;
    }
    return 0;
}

static store database;
/* Crash-cut state: `event` counts device boundaries once `active`, and the
 * guest is stopped at boundary number `cut` (0 means never). */
static unsigned active;
static unsigned event;
static unsigned cut;

static void print(const char *s)
{
    size_t n = 0;
    while (s[n])
    {
        n++;
    }
    cmk_write(s, n);
}

static void number(unsigned n)
{
    char text[12];
    unsigned i = 0;
    do
    {
        text[i++] = (char)('0' + n % 10);
        n /= 10;
    } while (n);
    while (i)
    {
        cmk_write(text + --i, 1);
    }
}

static void finish(unsigned status)
{
    cmk_call(CMK_EXIT, status, 0, 0);
    for (;;)
    {
    }
}

#define CHECK(x, s)             \
    do                          \
    {                           \
        if (!(x))               \
        {                       \
            print("SD_CHECK:"); \
            print(s);           \
            print("\n");        \
            finish(91);         \
        }                       \
    } while (0)

static void boundary(void)
{
    if (active && ++event == cut)
    {
        print("PASS sd-crash-cut\n");
        cmk_call(SDK_CRASH, 0, 0, 0);
        for (;;)
        {
        }
    }
}

static int read_page(void *ctx, uint32_t page, void *out)
{
    if (page >= STORE_IMAGE_PAGES)
    {
        return -1;
    }
    return cmk2_block(ctx, 0, page + 128, out);
}

static int write_page(void *ctx, uint32_t page, const void *in)
{
    if (page >= STORE_IMAGE_PAGES)
    {
        return -1;
    }
    boundary();
    int r = cmk2_block(ctx, 1, page + 128, (void *)in);
    if (r)
    {
        print("SD_IO_WRITE event ");
        number(event);
        print(" page ");
        number(page);
        print(" result magnitude ");
        number((unsigned)(r < 0 ? -r : r));
        print("\n");
    }
    boundary();
    return r;
}

static int flush(void *ctx)
{
    boundary();
    int r = cmk2_block(ctx, 4, 0, 0);
    if (r)
    {
        print("SD_IO_FLUSH event ");
        number(event);
        print("\n");
    }
    boundary();
    return r;
}

void _start(void)
{
    store_io io = {(void *)(uintptr_t)CMK_DISK_HANDLE, read_page, write_page,
                   flush, STORE_IMAGE_PAGES};
    int verify = (int)cmk_call(CMK_BOOT_MODE, 0, 0, 0);
    cut = (unsigned)cmk_call(SDK_CUT, 0, 0, 0);
    if (!verify)
    {
        CHECK(store_format(io) == STORE_OK, "disk format");
        CHECK(store_open(&database, io) == STORE_OK, "initial open");
        CHECK(!sdr_initialize(&database), "receipt provision");
        CHECK(store_checkpoint(&database) == STORE_OK, "baseline checkpoint");
        active = 1;
        int32_t out = 99;
        CHECK(!sdr_apply(&database, 1, 1, 5, 7, &out) && out == 7,
              "durable first effect");
        boundary();
        active = 0;
        print("PASS sd-durable-receipt-before-reply\n");
        print("SD_EVENTS ");
        number(event);
        print("\n");
        /* Recreate only the owner object while the VM/device remains alive. */
        CHECK(store_open(&database, io) == STORE_OK, "owner restart open");
        out = 99;
        CHECK(!sdr_apply(&database, 1, 1, 5, 7, &out) && out == 7,
              "owner retry deduplicated");
        print("PASS sd-in-guest-owner-restart\n");
        finish(0);
    }
    CHECK(store_open(&database, io) == STORE_OK, "reboot recovery");
    int32_t value = 99;
    CHECK(!sdr_value(&database, &value) && (value == 0 || value == 7),
          "atomic effect after cut");
    int32_t out = 99;
    CHECK(!sdr_apply(&database, 1, 1, 5, 7, &out) && out == 7,
          "retry after reboot exactly once");
    out = 99;
    CHECK(sdr_apply(&database, 1, 1, 5, 8, &out) == SDR_CONFLICT && out == 99,
          "receipt payload binding");
    CHECK(!sdr_apply(&database, 2, 1, 9, 3, &out) && out == 10,
          "second client");
    CHECK(!sdr_apply(&database, 1, 1, 10, -2, &out) && out == 8,
          "nonconsecutive ID");
    CHECK(!sdr_apply(&database, 1, 1, 20, 1, &out) && out == 9,
          "third retained receipt");
    CHECK(!sdr_apply(&database, 1, 1, 30, 1, &out) && out == 10,
          "receipt eviction");
    out = 99;
    CHECK(sdr_apply(&database, 1, 1, 5, 7, &out) == SDR_STALE && out == 99,
          "evicted cannot reapply");
    CHECK(sdr_apply(&database, 2, 2, 11, 1, &out) == SDR_STALE && out == 99,
          "wrong epoch");
    CHECK(sdr_apply(&database, 2, 1, 11, INT32_MAX, &out) == SDR_OVERFLOW &&
              out == 99,
          "overflow no effect");
    CHECK(!sdr_apply(&database, 2, 1, 9, 3, &out) && out == 10,
          "other client retained result");
    CHECK(!sdr_value(&database, &value) && value == 10 &&
              store_check(&database) == STORE_OK,
          "final effect structure");
    print("PASS sd-reboot-receipt-retention\n");
    finish(0);
}
