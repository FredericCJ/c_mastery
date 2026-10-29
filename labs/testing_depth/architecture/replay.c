#include "architecture.h"
#include <stdio.h>

int main(void)
{
    /* Every length-seven word over PUSH7,PUSH8,POP,PEEK, for each layout. */
    for (unsigned layout = 0; layout < 4; layout++)
        for (unsigned word = 0; word < 16384; word++)
        {
            struct ta_queue q;
            struct ta_storage s;
            unsigned code = word, accepted = 0, removed = 0;
            if (ta_init(&q, &s, (enum ta_layout)layout, 2,
                        2 + (layout == TA_RING)) != TA_OK)
                return 2;
            for (unsigned step = 0; step < 7; step++)
            {
                unsigned op = code % 4;
                unsigned char out = 99;
                enum ta_status status;
                code /= 4;
                if (op < 2)
                {
                    status = ta_push(&q, (unsigned char)(7 + op));
                    accepted += status == TA_OK;
                }
                else
                {
                    status = op == 2 ? ta_pop(&q, &out) : ta_peek(&q, &out);
                    removed += op == 2 && status == TA_OK;
                }
                printf("%u %u %u %u %u %u %zu %u %u %d\n", layout, word, step,
                       op, (unsigned)status, (unsigned)out, ta_count(&q),
                       accepted, removed, ta_invariant(&q));
            }
        }
    return 0;
}
