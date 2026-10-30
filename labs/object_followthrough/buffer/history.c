/*
 * Exhaustive enumeration of every length-4 sequence over five table
 * operations (5^4 = 625 histories), driven against a real allocator.
 * One CSV line per step:
 *   history,step,event,status,generation,retired,capacity,length,b0,b1,b2,b3
 * where b0..b3 are the first four payload bytes (0 when absent). Events:
 * 0 append, 1 resize to 4, 2 erase index 0, 3 resize to 8 with the
 * allocator's resize failing, 4 reset. The table starts with capacity 4.
 */
#include "buffer.h"
#include <stdio.h>
#include <stdlib.h>

enum
{
    EVENT_KINDS = 5,
    STEPS_PER_HISTORY = 4,
    HISTORY_COUNT = EVENT_KINDS * EVENT_KINDS * EVENT_KINDS * EVENT_KINDS,
    INITIAL_CAPACITY = 4,
    TABLE_IDENTITY = 17
};

static int fail; /* nonzero: resize() reports failure */

static void *allocate(void *context, size_t bytes)
{
    (void)context;
    return malloc(bytes);
}

static void *resize(void *context, void *old, size_t before, size_t after)
{
    (void)context;
    (void)before;
    return fail ? NULL : realloc(old, after);
}

static void release(void *context, void *p)
{
    (void)context;
    free(p);
}

int main(void)
{
    for (unsigned history = 0; history < HISTORY_COUNT; ++history)
    {
        struct ob_table t = {0};
        unsigned code = history; /* base-5 digits select the events */
        struct ob_allocator a = {NULL, allocate, resize, release};
        if (ob_create(&t, a, TABLE_IDENTITY, INITIAL_CAPACITY))
        {
            return 2;
        }
        for (unsigned step = 0; step < STEPS_PER_HISTORY; ++step)
        {
            unsigned event = code % EVENT_KINDS;
            int status;
            code /= EVENT_KINDS;
            switch (event)
            {
                case 0:
                    status = ob_append(&t, (unsigned char)(step + 1));
                    break;
                case 1:
                    status = ob_resize(&t, 4);
                    break;
                case 2:
                    status = ob_erase(&t, 0);
                    break;
                case 3:
                    fail = 1;
                    status = ob_resize(&t, 8);
                    fail = 0;
                    break;
                default:
                    ob_reset(&t);
                    status = 0;
                    break;
            }
            printf("%u,%u,%u,%d,%u,%d,%zu,%zu", history, step, event, status,
                   t.generation, t.retired, t.value ? t.value->capacity : 0,
                   t.value ? t.value->length : 0);
            for (size_t i = 0; i < 4; ++i)
            {
                printf(",%u", t.value && i < t.value->length
                                  ? t.value->payload[i]
                                  : 0);
            }
            putchar('\n');
        }
        ob_reset(&t);
    }
    return 0;
}
