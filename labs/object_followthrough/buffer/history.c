#include "buffer.h"
#include <stdio.h>
#include <stdlib.h>
static int fail;

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
    for (unsigned history = 0; history < 625; ++history)
    {
        struct ob_table t = {0};
        unsigned code = history;
        struct ob_allocator a = {NULL, allocate, resize, release};
        if (ob_create(&t, a, 17, 4))
            return 2;
        for (unsigned step = 0; step < 4; ++step)
        {
            unsigned event = code % 5;
            int status;
            code /= 5;
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
                printf(",%u", t.value && i < t.value->length
                                  ? t.value->payload[i]
                                  : 0);
            putchar('\n');
        }
        ob_reset(&t);
    }
    return 0;
}
