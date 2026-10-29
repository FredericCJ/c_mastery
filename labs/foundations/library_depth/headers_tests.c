#include "headers.h"
#include <inttypes.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#define CHECK(x)                                                         \
    do                                                                   \
    {                                                                    \
        if (!(x))                                                        \
        {                                                                \
            fprintf(stderr, "HEADERS-CHECK: %s line%d\n", #x, __LINE__); \
            return 1;                                                    \
        }                                                                \
    } while (0)

int main(void)
{
    char text[128];
    size_t out = 77;
    uintmax_t scanned = 0;
    int used = -1;
    CHECK(lh_size((const unsigned char *)"0", 1, &out) == LH_OK && out == 0);
    CHECK(lh_size((const unsigned char *)"0012", 4, &out) == LH_OK &&
          out == 12);
    const char *invalid[] = {"", "+1", "-1", " 1", "1x", "1 ", "0x1"};
    for (size_t i = 0; i < sizeof invalid / sizeof *invalid; ++i)
    {
        out = 77;
        CHECK(lh_size((const unsigned char *)invalid[i], strlen(invalid[i]),
                      &out) == LH_SYNTAX &&
              out == 77);
    }
    const unsigned char embedded[] = {'1', 0, '2'};
    CHECK(lh_size(embedded, 3, &out) == LH_SYNTAX && out == 77);
    CHECK(lh_size(NULL, 1, &out) == LH_ARGUMENT && out == 77);
    CHECK(lh_size(NULL, 0, &out) == LH_SYNTAX && out == 77);
    CHECK(lh_size((const unsigned char *)"1", 1, NULL) == LH_ARGUMENT);
    int n = snprintf(text, sizeof text, "%zu", SIZE_MAX);
    CHECK(n > 0 && (size_t)n < sizeof text);
    CHECK(lh_size((const unsigned char *)text, (size_t)n, &out) == LH_OK &&
          out == SIZE_MAX);
    CHECK(sscanf(text, "%" SCNuMAX "%n", &scanned, &used) == 1 && used == n &&
          scanned == (uintmax_t)SIZE_MAX);
    text[n] = '0';
    text[n + 1] = 0;
    out = 77;
    CHECK(lh_size((const unsigned char *)text, (size_t)n + 1, &out) ==
              LH_RANGE &&
          out == 77);
    if (UINTMAX_MAX > SIZE_MAX)
    {
        uintmax_t wide = (uintmax_t)SIZE_MAX + 1;
        n = snprintf(text, sizeof text, "%" PRIuMAX, wide);
        CHECK(n > 0);
        CHECK(lh_size((const unsigned char *)text, (size_t)n, &out) ==
                  LH_RANGE &&
              out == 77);
    }
#ifdef UINT32_MAX
    uint32_t u = UINT32_MAX, readback = 0;
    n = snprintf(text, sizeof text, "%" PRIu32, u);
    CHECK(n > 0);
    CHECK(sscanf(text, "%" SCNu32 "%n", &readback, &used) == 1 && used == n &&
          readback == u);
    printf("uint32=%s uint32_bytes=%zu\n", text, sizeof u);
#endif
    printf("size_bytes=%zu ptrdiff_bytes=%zu uintmax_bytes=%zu CHAR_BIT=%d "
           "SIZE_MAX=%zu PTRDIFF_MAX=%td\n",
           sizeof(size_t), sizeof(ptrdiff_t), sizeof(uintmax_t), CHAR_BIT,
           SIZE_MAX, (ptrdiff_t)PTRDIFF_MAX);
    for (int h = 0; h < 2; ++h)
        for (int old = 0; old < 2; ++old)
            for (int no = 0; no < 2; ++no)
                for (int linked = 0; linked < 2; ++linked)
                    CHECK(lh_c11_threads(h, old ? 199901L : 201710L, no,
                                         linked) ==
                          (h && !old && !no && linked));
    puts("HEADERS-PASS: full decimal admission, matching PRI/SCN and all16 "
         "capability combinations");
    return 0;
}
