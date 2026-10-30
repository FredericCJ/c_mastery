#ifndef FS_SEMANTICS_H
#define FS_SEMANTICS_H
#include <stddef.h>
#include <stdint.h>

/* Result of every fs_* function that reports a status. On any status other
 * than FS_OK the caller's output objects are left unchanged. */
enum fs_status
{
    FS_OK,
    /* Null pointer, aliased outputs or a branch other than 0 or 1. */
    FS_INVALID,
    /* A signed addition would leave the range of int. */
    FS_RANGE,
    /* Reserved: no reference function returns it. */
    FS_UNINITIALIZED,
    /* Returned by the unfinished learner stubs. */
    FS_NOT_IMPLEMENTED
};

/* Where an index falls relative to an object of a given element count. */
enum fs_index
{
    /* Neither an element nor the one-past position. */
    FS_OUTSIDE,
    /* 0 <= index < length: the element may be accessed. */
    FS_ELEMENT,
    /* index == length: the pointer may be formed but not dereferenced. */
    FS_ONE_PAST
};

/* Observation record for fs_calls(). */
struct fs_calls
{
    int sum;
    int x;
    int y;
    unsigned order; /* decimal digits of the call bodies run: 12 or 21 */
};

/* Classifies index against an object of length elements, in signed
 * arithmetic before any conversion to an unsigned count. Pure metadata:
 * forms no pointer and touches no memory. Never fails. */
enum fs_index fs_index(intmax_t index, size_t length);

/* Sums length ints with every addition checked.
 * Pre: input is a live array of length ints (may be NULL when length is 0);
 *      out is a live object disjoint from the array. Both are only borrowed.
 * Returns FS_OK and publishes the total (0 when empty), FS_INVALID when out
 * is NULL or input is NULL with length > 0, FS_RANGE on overflow. *out is
 * written only on FS_OK. Does not block or allocate. */
enum fs_status fs_sum(const int *input, size_t length, int *out);

/* As fs_sum() for exactly one row of three ints; the extent comes from the
 * pointed-to type. Returns FS_INVALID when row is NULL, plus the fs_sum()
 * failures. */
enum fs_status fs_row(const int (*row)[3], int *out);

/* Sums count rows of three ints in row-major order, checking every
 * intermediate total. count is the number of row objects, not ints.
 * Pre: rows is NULL only when count is 0; out is live and disjoint.
 * Returns FS_OK (0 for no rows), FS_INVALID or FS_RANGE; *out is written
 * only on FS_OK. A table of int pointers is a different type and is rejected
 * at translation time. */
enum fs_status fs_matrix(const int (*rows)[3], size_t count, int *out);

/* branch must be 0 or 1; both branches initialize the result (1 gives 7,
 * 0 gives 11). Publication is deferred: an invalid branch or a NULL out
 * returns FS_INVALID and leaves *out unchanged. */
enum fs_status fs_branch(int branch, int *out);

/* Starts from x=1, y=2, calls one function returning ++x and another
 * returning y*=2, and records their sum and body order. The order is
 * indeterminately sequenced, so *out records 12 or 21; sum, x and y are
 * fixed. Returns FS_INVALID when out is NULL. */
enum fs_status fs_calls(struct fs_calls *out);

/* Evaluates the comma operator in (i++, i++) for i = 1 and publishes the
 * value of the expression through value and the final i through final.
 * Returns FS_INVALID, changing nothing, when either pointer is NULL or the
 * two point to the same object. */
enum fs_status fs_comma(int *value, int *final);
#endif
