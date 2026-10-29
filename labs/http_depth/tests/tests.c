#include "http_depth.h"
#include <stdio.h>
#include <string.h>
#define CHECK(x)                                               \
    do                                                         \
    {                                                          \
        if (!(x))                                              \
        {                                                      \
            fprintf(stderr, "HD_CHECK:%d:%s\n", __LINE__, #x); \
            return 1;                                          \
        }                                                      \
    } while (0)

static int length(void)
{
    const char *s[] = {
        "0",           "000001024", "1025", "4294967295", "4294967296",
        "42949672960", "1025x",     "+1",   "1 2",        ""};
    int expected[] = {0, 0, 413, 413, 400, 400, 400, 400, 400, 400};
    for (unsigned i = 0; i < 10; i++)
    {
        uint32_t n = 777;
        CHECK(hd_length((const unsigned char *)s[i], strlen(s[i]), 1024, &n) ==
              expected[i]);
        if (expected[i])
            CHECK(n == 777);
    }
    uint32_t n = 999;
    CHECK(hd_length((const unsigned char *)"0", 1, 0, &n) == 0 && n == 0);
    CHECK(hd_length(NULL, 0, 1024, &n) == 400);
    return 0;
}

static int date(void)
{
    char out[30];
    memset(out, 'x', sizeof out);
    size_t n = 99;
    CHECK(hd_date(0, UINT32_MAX, UINT32_MAX, out, 0, &n) == 0 && n == 0 &&
          out[0] == 'x');
    CHECK(hd_date(1, 0, 0, out, sizeof out, &n) == 0 && n == 29 &&
          !strcmp(out, "Thu, 01 Jan 1970 00:00:00 GMT"));
    CHECK(hd_date(1, 47541, 0, out, sizeof out, &n) == 0 &&
          !strcmp(out, "Mon, 01 Mar 2100 00:00:00 GMT"));
    CHECK(hd_date(1, 11016, 86399, out, sizeof out, &n) == 0 &&
          !strcmp(out, "Tue, 29 Feb 2000 23:59:59 GMT"));
    char old[30];
    memcpy(old, out, 30);
    n = 77;
    CHECK(hd_date(1, 2932897, 0, out, 30, &n) == HD_INVALID && n == 77 &&
          !memcmp(old, out, 30));
    CHECK(hd_date(1, 0, 86400, out, 30, &n) == HD_INVALID);
    CHECK(hd_date(1, 0, 0, out, 29, &n) == HD_INVALID);
    CHECK(hd_date(1, 2932896, 86399, out, 30, &n) == 0 &&
          !strcmp(out, "Fri, 31 Dec 9999 23:59:59 GMT"));
    return 0;
}

static int lifecycle(void)
{
    struct hd_pool p = {0};
    struct hd_handle a, b;
    int fd = -1;
    unsigned mask = 99;
    CHECK(hd_admit(&p, 11, 1, &a) == 0);
    CHECK(hd_event(&p, a, 11, 15, &mask) == 0 && mask == 13);
    CHECK(hd_interest(&p, a, 0) == 0);
    CHECK(hd_event(&p, a, 11, 3, &mask) == 0 && mask == 0);
    CHECK(hd_close(&p, a, &fd) == 0 && fd == 11);
    fd = -1;
    CHECK(hd_close(&p, a, &fd) == HD_STALE && fd == -1);
    CHECK(hd_admit(&p, 11, 1, &b) == 0 && a.index == b.index &&
          a.generation != b.generation);
    mask = 99;
    CHECK(hd_event(&p, a, 11, 1, &mask) == HD_STALE && mask == 99);
    CHECK(hd_admit(&p, 11, 1, &a) == HD_INVALID);
    CHECK(hd_admit(&p, 12, 0, &a) == HD_INVALID);
    CHECK(hd_close(&p, b, &fd) == 0);
    p.slots[0].generation = UINT32_MAX - 1;
    CHECK(hd_admit(&p, 12, 1, &a) == 0 && a.generation == UINT32_MAX);
    CHECK(hd_close(&p, a, &fd) == 0 && p.slots[0].retired);
    CHECK(hd_admit(&p, 13, 1, &b) == 0 && b.index == 1);
    CHECK(hd_interest(&p, b, 2) == 0);
    CHECK(hd_event(&p, b, 13, 15, &mask) == 0 && mask == 14);
    CHECK(hd_event(&p, b, 99, 1, &mask) == HD_STALE);
    return 0;
}

int main(int argc, char **argv)
{
    if (argc != 2)
        return 2;
    int r = !strcmp(argv[1], "length")      ? length()
            : !strcmp(argv[1], "date")      ? date()
            : !strcmp(argv[1], "lifecycle") ? lifecycle()
                                            : 2;
    if (!r)
        puts("PASS HTTP depth");
    return r;
}
