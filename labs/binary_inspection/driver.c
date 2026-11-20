#include <stdio.h>
#include "record.h"

struct record { unsigned tag, size; };

int main(void)
{
    const struct record rows[] = {{3, 5}, {1, 2}, {3, 4}};
    unsigned total = 0;
    for (unsigned i = 0; i < sizeof rows / sizeof rows[0]; ++i)
        total += record_score(rows[i].tag, rows[i].size);
    printf("total=%u calls=%u\n", total, record_calls);
    return 0;
}
