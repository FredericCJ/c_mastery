/*
 * Allocation helper built as a separate shared object (libeffects_hidden.so)
 * and linked into process_test. The campaign compiles it without any
 * `--wrap` option.
 */
#include <stdlib.h>

void *ef_hidden_take(size_t bytes)
{
    return malloc(bytes);
}

void ef_hidden_release(void *storage)
{
    free(storage);
}
