/* CM-SPECIMEN: constraint violation (table of int pointers passed where an
 * array of three-int rows is required). Compile-only negative translation:
 * the build must be rejected with an "incompatible" diagnostic, never
 * linked. */
#include "semantics.h"

int fs_wrong(void)
{
    int a[3] = {1, 2, 3};
    int *rows[2] = {a, a};
    int out;
    /* int ** is a different representation from int (*)[3]. */
    return fs_matrix(rows, 2, &out);
}
