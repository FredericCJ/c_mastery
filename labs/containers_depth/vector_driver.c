/*
 * Line-driven harness for cd_vector, checked against the independent list
 * model in vector_oracle.py.  Each stdin line is "<op> <at> <value> <fail>":
 *   z  destroy and re-initialize        i  insert value at index `at`
 *   e  erase index `at`                 r  reserve capacity `at`
 *   s  shrink to fit
 * A nonzero `fail` makes every block acquisition on that line fail.  After each
 * line the driver prints "<status> <length> <capacity> <live blocks> <values>".
 * Exit status: 0 clean, 2 init failure, 3 the vector leaked a block.
 */
#include "containers.h"
#include <stdio.h>
#include <stdlib.h>

struct cd_alloc
{
    unsigned fail;
    unsigned live;
};

static int *take(void *v, size_t n)
{
    struct cd_alloc *a = v;
    if (a->fail)
    {
        return NULL;
    }
    int *p = malloc(n * sizeof *p);
    if (p)
    {
        ++a->live;
    }
    return p;
}

static void release(void *v, int *p)
{
    struct cd_alloc *a = v;
    --a->live;
    free(p);
}

static enum cd_status vector_start(struct cd_vector *vector,
                                   struct cd_alloc *allocator)
{
    return cd_vector_init(vector,
                          (struct cd_allocator){allocator, take, release});
}

/* Applies one operation; an unknown op code leaves CD_INVALID. */
static enum cd_status apply(struct cd_vector *vector,
                            struct cd_alloc *allocator, char op, size_t at,
                            int value)
{
    int erased = 0;
    switch (op)
    {
        case 'z':
            cd_vector_destroy(vector);
            return vector_start(vector, allocator);
        case 'i':
            return cd_vector_insert(vector, at, value);
        case 'e':
            return cd_vector_erase(vector, at, &erased);
        case 'r':
            return cd_vector_reserve(vector, at);
        case 's':
            return cd_vector_shrink(vector);
        default:
            return CD_INVALID;
    }
}

static void print_state(enum cd_status status, const struct cd_vector *vector,
                        unsigned live_blocks)
{
    printf("%d %zu %zu %u", status, vector->length, vector->capacity,
           live_blocks);
    for (size_t i = 0; i < vector->length; ++i)
    {
        printf(" %d", vector->data[i]);
    }
    putchar('\n');
}

int main(void)
{
    struct cd_alloc allocator = {0};
    struct cd_vector vector;
    if (vector_start(&vector, &allocator) != CD_OK)
    {
        return 2;
    }
    char op;
    size_t at;
    int value;
    unsigned fail;
    while (scanf(" %c %zu %d %u", &op, &at, &value, &fail) == 4)
    {
        allocator.fail = fail;
        enum cd_status status = apply(&vector, &allocator, op, at, value);
        print_state(status, &vector, allocator.live);
    }
    return cd_vector_destroy(&vector) == CD_OK && allocator.live == 0 ? 0 : 3;
}
