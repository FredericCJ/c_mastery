/* Original intentionally unwrapped DSO: --wrap rewrites the executable's
 * unresolved references, not an already-linked shared object's relocations. */
#include <stdlib.h>

void *ef_hidden_take(size_t n)
{
    return malloc(n);
}

void ef_hidden_release(void *p)
{
    free(p);
}
