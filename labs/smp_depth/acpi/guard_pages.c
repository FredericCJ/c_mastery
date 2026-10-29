#define _GNU_SOURCE
#include "checks.h"
#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>

int main(void)
{
    if (sa_checks())
    {
        fputs("SA_GUARD_CHECK:ordinary-contract\n", stderr);
        return 1;
    }
    long page = sysconf(_SC_PAGESIZE);
    if (page < 200)
        return 2;
    unsigned char *memory = mmap(0, (size_t)page * 2, PROT_READ | PROT_WRITE,
                                 MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (memory == MAP_FAILED ||
        mprotect(memory + page, (size_t)page, PROT_NONE))
        return 2;
    unsigned char input[76];
    sa_base(input, 76);
    for (unsigned i = 0; i < 4; ++i)
    {
        input[44 + 8 * i] = 0;
        input[45 + 8 * i] = 8;
        input[46 + 8 * i] = (unsigned char)i;
        input[47 + 8 * i] = (unsigned char)(2 * i);
        sa_w32(input + 48 + 8 * i, 1);
    }
    sa_sum(input, 76);
    for (unsigned n = 0; n < 76; ++n)
    {
        unsigned char *start = memory + page - n;
        for (unsigned j = 0; j < n; ++j)
            start[j] = input[j];
        struct sa_topology result = {0}, old = {0};
        result.count = 9;
        old = result;
        if (sa_parse(start, n, 16, 0, &result) == SA_OK ||
            !sa_same(&result, &old, sizeof result))
        {
            fputs("SA_GUARD_CHECK:truncation\n", stderr);
            return 1;
        }
    }
    if (munmap(memory, (size_t)page * 2))
        return 2;
    puts("SA_GUARD_PASS:76-protected-truncations");
    return 0;
}
