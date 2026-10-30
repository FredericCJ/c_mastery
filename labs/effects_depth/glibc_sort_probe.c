/*
 * GNU libc-specific observation of qsort's own allocation. __libc_malloc is a
 * glibc interface outside ISO C, so this probe defines malloc and free itself
 * and forwards to the internal entry points.
 *
 * Four sorting modes are run; for each, the probe records how many malloc
 * requests qsort made, the size of the last one, and whether errno survived
 * (including a forced malloc failure in mode 3). Exit status: 1 when errno
 * changed, 2 or 3 when an array is not sorted, 4 when the recorded request
 * counts or sizes differ from the checked values.
 */
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <gnu/libc-version.h>

#define MODES 4u
#define SMALL_COUNT 64u
#define LARGE_COUNT 300u
#define RECORD_COUNT 10u
#define CHECKED_REQUEST_BYTES 1200u

void *__libc_malloc(size_t bytes);
void __libc_free(void *storage);

/* Probe state; the program is single-threaded. */
static unsigned tracing;
static unsigned fail_allocation;
static unsigned requests;
static size_t requested_bytes;

void *malloc(size_t bytes)
{
    if (tracing)
    {
        ++requests;
        requested_bytes = bytes;
        if (fail_allocation)
        {
            errno = ENOMEM;
            return NULL;
        }
    }
    return __libc_malloc(bytes);
}

void free(void *storage)
{
    __libc_free(storage);
}

static int compare_int(const void *left, const void *right)
{
    int x = *(const int *)left;
    int y = *(const int *)right;
    return (x > y) - (x < y);
}

struct ef_record
{
    int key;
    unsigned char payload[60];
};

static int is_sorted_ints(const int *values, unsigned count)
{
    for (unsigned i = 1; i < count; ++i)
    {
        if (values[i - 1] > values[i])
        {
            return 0;
        }
    }
    return 1;
}

static int is_sorted_records(const struct ef_record *records, unsigned count)
{
    for (unsigned i = 1; i < count; ++i)
    {
        if (records[i - 1].key > records[i].key)
        {
            return 0;
        }
    }
    return 1;
}

int main(void)
{
    int values[LARGE_COUNT];
    struct ef_record large[RECORD_COUNT];
    unsigned counts[MODES];
    size_t sizes[MODES];
    for (unsigned mode = 0; mode < MODES; ++mode)
    {
        for (unsigned i = 0; i < LARGE_COUNT; ++i)
        {
            values[i] = (int)(LARGE_COUNT - i);
        }
        for (unsigned i = 0; i < RECORD_COUNT; ++i)
        {
            large[i].key = (int)(RECORD_COUNT - i);
        }
        requests = 0;
        requested_bytes = 0;
        fail_allocation = mode == 3;
        errno = E2BIG;
        unsigned count = mode == 0 ? SMALL_COUNT : LARGE_COUNT;
        tracing = 1;
        if (mode == 2)
        {
            qsort(large, RECORD_COUNT, sizeof *large, compare_int);
        }
        else
        {
            qsort(values, count, sizeof *values, compare_int);
        }
        tracing = 0;
        counts[mode] = requests;
        sizes[mode] = requested_bytes;
        if (errno != E2BIG)
        {
            return 1;
        }
        if (mode == 2 ? !is_sorted_records(large, RECORD_COUNT)
                      : !is_sorted_ints(values, count))
        {
            return mode == 2 ? 2 : 3;
        }
    }
    if (counts[0] != 0 || counts[1] != 1 || sizes[1] != CHECKED_REQUEST_BYTES ||
        counts[2] != 0 || counts[3] != 1 || sizes[3] != CHECKED_REQUEST_BYTES)
    {
        return 4;
    }
    printf(
        "{\"runtime\":\"%s\",\"observed_requests\":[%u,%u,%u,%u],\"observed_sizes\":[%zu,%zu,%zu,%zu],\"malloc_failure_sorted\":true,\"errno_preserved\":true}\n",
        gnu_get_libc_version(), counts[0], counts[1], counts[2], counts[3],
        sizes[0], sizes[1], sizes[2], sizes[3]);
    return 0;
}
