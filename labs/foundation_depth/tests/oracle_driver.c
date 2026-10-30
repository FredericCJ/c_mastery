/* Line-oriented driver for the independent oracle in
 * tools/verify_foundation_depth_fixtures.py. It reads commands from stdin and
 * prints one result line per command:
 *
 *   E offset count width limit        -> "E <status> <result>"
 *   D b0 b1 b2 b3 b4 b5 b6 b7 (hex)  -> "D <status> <flags> <count> <value>"
 *
 * Exit status: 0 at end of input, 2 for any malformed command. */
#include "depth.h"
#include <stdio.h>
#include <inttypes.h>

#define OPERATION_OK 0
#define OPERATION_MALFORMED 2

/* Sentinels prove that a rejected call leaves its output untouched. */
#define EXTENT_SENTINEL 917u
#define RECORD_SENTINEL_FLAGS 3
#define RECORD_SENTINEL_COUNT 999
#define RECORD_SENTINEL_VALUE 42

static int run_extent(void)
{
    size_t offset;
    size_t count;
    size_t width;
    size_t limit;
    if (scanf("%zu %zu %zu %zu", &offset, &count, &width, &limit) != 4)
    {
        return OPERATION_MALFORMED;
    }
    size_t result = EXTENT_SENTINEL;
    const enum fd_status status =
        fd_extent(offset, count, width, limit, &result);
    printf("E %d %zu\n", status, result);
    return OPERATION_OK;
}

/* Reads FD_WIRE_BYTES hexadecimal octets into `bytes`. */
static int read_wire(unsigned char bytes[FD_WIRE_BYTES])
{
    for (unsigned i = 0; i < FD_WIRE_BYTES; i++)
    {
        unsigned octet;
        if (scanf("%x", &octet) != 1 || octet > UCHAR_MAX)
        {
            return OPERATION_MALFORMED;
        }
        bytes[i] = (unsigned char)octet;
    }
    return OPERATION_OK;
}

static int run_decode(void)
{
    unsigned char bytes[FD_WIRE_BYTES];
    if (read_wire(bytes) != OPERATION_OK)
    {
        return OPERATION_MALFORMED;
    }
    struct fd_record record = {RECORD_SENTINEL_FLAGS, RECORD_SENTINEL_COUNT,
                               RECORD_SENTINEL_VALUE};
    const enum fd_status status = fd_decode(bytes, sizeof bytes, &record);
    printf("D %d %u %u %" PRId32 "\n", status, (unsigned)record.flags,
           (unsigned)record.count, record.value);
    return OPERATION_OK;
}

int main(void)
{
    printf("SIZE_MAX %zu\n", SIZE_MAX);
    char operation;
    while (scanf(" %c", &operation) == 1)
    {
        int outcome;
        if (operation == 'E')
        {
            outcome = run_extent();
        }
        else if (operation == 'D')
        {
            outcome = run_decode();
        }
        else
        {
            outcome = OPERATION_MALFORMED;
        }
        if (outcome != OPERATION_OK)
        {
            return outcome;
        }
    }
    return OPERATION_OK;
}
