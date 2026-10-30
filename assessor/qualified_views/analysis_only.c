/*
 * CM-SPECIMEN: returns pointers to expired block-scope objects, and reads an
 * indeterminate value.
 *
 * Analysis only: the campaign compiles this file to an object (-c) and never
 * links or calls it. Do not "fix" the defects; the lab teaches how to classify
 * them (see ANALYSIS.md) and how to repair the pattern with caller-owned
 * storage (fv_initialize in views.c).
 */
#include "views.h"

/* Defect: the compound literal expires when the function returns. */
const int *escape(void)
{
    return (int[]){1, 2, 3, 4};
}

/*
 * Defect: returning the struct by value copies the pointer, not the array it
 * points to, so the view still designates expired storage.
 */
struct fv_view wrapped_escape(void)
{
    return (struct fv_view){(int[]){1, 2, 3, 4}, 4};
}

/* Defect: reads a member of an automatic object that was never stored to. */
int indeterminate(void)
{
    struct fv_record uninitialized;
    return uninitialized.values[0];
}
