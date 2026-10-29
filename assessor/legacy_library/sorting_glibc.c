/* Exact GNU2.41 diagnostic only. No other program is interposed. */
#include <errno.h>
#include <gnu/libc-version.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
void *__libc_malloc(size_t);
void __libc_free(void *);
static unsigned active, deny, requests;
static size_t last;

void *malloc(size_t n)
{
    if (active)
    {
        ++requests;
        last = n;
        if (deny)
        {
            errno = ENOMEM;
            return NULL;
        }
    }
    return __libc_malloc(n);
}

void free(void *p)
{
    __libc_free(p);
}

static int cmp(const void *a, const void *b)
{
    const unsigned char *x = a, *y = b;
    return (x[0] > y[0]) - (x[0] < y[0]);
}

int main(void)
{
    if (strcmp(gnu_get_libc_version(), "2.41"))
    {
        return 2;
    }
    unsigned char records[96 * 64];
    const size_t widths[] = {32, 33, 64};
    unsigned cases = 0;
    for (unsigned fail = 0; fail < 2; ++fail)
    {
        for (unsigned w = 0; w < 3; ++w)
        {
            for (unsigned ncase = 0; ncase < 2; ++ncase)
            {
                size_t width = widths[w], n = ncase ? 96 : 20;
                for (size_t i = 0; i < n; ++i)
                {
                    memset(records + i * width, (int)i, width);
                    records[i * width] = (unsigned char)((n - i) % 7);
                    records[i * width + 1] = (unsigned char)i;
                }
                requests = 0;
                last = 0;
                deny = fail;
                errno = E2BIG;
                active = 1;
                qsort(records, n, width, cmp);
                active = 0;
                size_t expected =
                    width > 32 ? 2 * n * sizeof(void *) + width : n * width;
                if (requests != (expected > 1024) ||
                    (requests && last != expected) || errno != E2BIG)
                {
                    return 3;
                }
                unsigned seen[96] = {0};
                for (size_t i = 0; i < n; ++i)
                {
                    unsigned id = records[i * width + 1];
                    if (id >= n || seen[id]++ ||
                        records[i * width] != (n - id) % 7)
                    {
                        return 4;
                    }
                    for (size_t k = 1; k < width; ++k)
                    {
                        if (records[i * width + k] != id)
                        {
                            return 5;
                        }
                    }
                    if (i && records[(i - 1) * width] > records[i * width])
                    {
                        return 6;
                    }
                }
                printf(
                    "width=%zu count=%zu deny=%u requests=%u request_bytes=%zu "
                    "public_order_permutation=pass\n",
                    width, n, fail, requests, last);
                ++cases;
            }
        }
    }
    printf("SORTING-PASS:glibc2.41 %u threshold/failure observations;equal "
           "identity order unspecified\n",
           cases);
    return 0;
}
