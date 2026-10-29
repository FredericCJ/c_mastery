#include <stdint.h>
#include <stdio.h>
int xi_wide_queue_push(uint32_t);
int xi_wide_queue_pop(uint32_t *);

int main(void)
{
    uint32_t value = 99;
    for (unsigned round = 0; round < 1000; ++round)
    {
        for (unsigned i = 0; i < 3; ++i)
        {
            if (!xi_wide_queue_push(3 * round + i))
            {
                return 1;
            }
        }
        if (xi_wide_queue_push(77))
        {
            return 1;
        }
        for (unsigned i = 0; i < 3; ++i)
        {
            if (!xi_wide_queue_pop(&value) || value != 3 * round + i)
            {
                return 1;
            }
        }
        value = 99;
        if (xi_wide_queue_pop(&value) || value != 99)
        {
            return 1;
        }
    }
    puts("XI_WIDE_HOST_PASS records=3000 index_bits=64 capacity=3");
    return 0;
}
