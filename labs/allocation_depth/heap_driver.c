#include "allocation.h"
#include <stdio.h>

/*
 * Line-oriented driver for the host-side interval oracle.
 *
 * stdin: "capacity policy deferred" once, then repeated "op index size fail".
 * Each operation prints one line: the status, the offset:length of each of the
 * 12 tracked objects ("-" when absent), then "| payload charged free_count"
 * and the offset:size of every free block.
 *
 * Exit status: 0 end of input, 2 malformed input, 3 heap check failed.
 */
enum op
{
    OP_TAKE = 0,
    OP_RELEASE = 1,
    OP_RESIZE = 2,
    OP_COLLECT = 3
};

enum
{
    TRACKED = 12,
    ALIGNMENT = 8,
    EXIT_BAD_INPUT = 2,
    EXIT_HEAP_CORRUPT = 3
};

static int apply(struct al_heap *h, struct al_handle handles[TRACKED],
                 unsigned op, unsigned index, unsigned size, unsigned fail)
{
    struct al_handle out = {0};
    int status;
    if (op == OP_TAKE)
    {
        status = al_heap_take(h, size, ALIGNMENT, &out);
    }
    else if (op == OP_RELEASE)
    {
        status = al_heap_release(h, handles[index]);
    }
    else if (op == OP_RESIZE)
    {
        status =
            al_heap_resize(h, handles[index], size, (int)fail, SIZE_MAX, &out);
    }
    else
    {
        status = al_heap_collect(h);
    }
    if (!status && (op == OP_TAKE || op == OP_RESIZE))
    {
        handles[index] = out;
    }
    if (!status && op == OP_RELEASE)
    {
        handles[index] = (struct al_handle){0};
    }
    return status;
}

static void print_state(struct al_heap *h, const struct al_handle *handles)
{
    for (unsigned i = 0; i < TRACKED; ++i)
    {
        unsigned char *p;
        size_t n;
        if (al_heap_view(h, handles[i], &p, &n) == AL_OK)
        {
            printf(" %zu:%zu", (size_t)(p - h->bytes), n);
        }
        else
        {
            printf(" -");
        }
    }
    printf(" | %zu %zu %u", h->payload, h->charged, h->free_count);
    for (unsigned i = 0; i < h->free_count; ++i)
    {
        unsigned at = h->free_offsets[i];
        /* Block total is the little-endian 16-bit word at the block start. */
        printf(" %u:%u", at, (unsigned)h->bytes[at] + 256u * h->bytes[at + 1]);
    }
}

int main(void)
{
    struct al_heap h;
    struct al_handle handles[TRACKED] = {{0}};
    unsigned cap;
    unsigned policy;
    unsigned defer;
    if (scanf("%u%u%u", &cap, &policy, &defer) != 3 ||
        al_heap_init(&h, cap, policy, defer, 1))
    {
        return EXIT_BAD_INPUT;
    }
    unsigned op;
    unsigned index;
    unsigned size;
    unsigned fail;
    while (scanf("%u%u%u%u", &op, &index, &size, &fail) == 4)
    {
        if (index >= TRACKED)
        {
            return EXIT_BAD_INPUT;
        }
        printf("%d", apply(&h, handles, op, index, size, fail));
        print_state(&h, handles);
        puts("");
        if (al_heap_check(&h))
        {
            return EXIT_HEAP_CORRUPT;
        }
    }
    return 0;
}
