/* CM-SPECIMEN: constraint violation (const int * passed where a pointer to a
 * three-int row is required). Compile-only negative translation: the build
 * must be rejected with an "incompatible" diagnostic, never linked. */
#include "semantics.h"

int fs_wrong(void)
{
    const int a[3] = {1, 2, 3};
    int out;
    /* a decays to const int *, not const int (*)[3]. */
    return fs_row(a, &out);
}
