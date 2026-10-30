/*
 * Link-time interposition probe. The campaign links this file with
 * `--wrap=ef_local_alloc` (a GNU linker option) and compares the two counters.
 * Exit status: 1 when the allocation fails, 2 when a counter is unexpected.
 */
#include <stddef.h>
#include <stdio.h>

#define STORAGE_BYTES 16u
#define REQUEST_BYTES 8u

static unsigned requests;
static unsigned hooks;
static unsigned char storage[STORAGE_BYTES];

/* Counts direct calls; storage is static, so nothing is ever freed. */
__attribute__((visibility("hidden"))) void *ef_local_alloc(size_t bytes)
{
    ++requests;
    return bytes <= sizeof storage ? storage : NULL;
}

/* The linker's wrapper symbol: counts calls that were redirected to it. */
void *__wrap_ef_local_alloc(size_t bytes)
{
    ++hooks;
    return bytes <= sizeof storage ? storage : NULL;
}

int main(void)
{
    unsigned char *block = ef_local_alloc(REQUEST_BYTES);
    if (!block)
    {
        return 1;
    }
    *block = 7;
    if (requests != 1 || hooks != 0)
    {
        return 2;
    }
    printf("{\"same_translation_unit_requests\":%u,\"link_wrap_hooks\":%u}\n",
           requests, hooks);
    return 0;
}
