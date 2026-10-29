#include "migration.h"
#include <limits.h>
#include <stdio.h>
int lp_old_consumer(int, int, int *);

static int one(int a, int b)
{
    int old = 77, modern = 77;
    int x = lp_old_consumer(a, b, &old), y = lp_sum(a, b, &modern);
    if (x != y || old != modern)
        return 0;
    return 1;
}

int main(void)
{
    int values[] = {INT_MIN, INT_MIN + 1, -10000,      -1,     0,
                    1,       10000,       INT_MAX - 1, INT_MAX};
    unsigned count = 0;
    for (unsigned i = 0; i < 9; ++i)
        for (unsigned j = 0; j < 9; ++j)
        {
            if (!one(values[i], values[j]))
                return 1;
            ++count;
        }
    for (int i = -32768; i <= 32768; ++i)
    {
        if (!one(i, -i))
            return 1;
        ++count;
    }
    printf("LP_OLD_MODERN %u\n", count);
    return 0;
}
