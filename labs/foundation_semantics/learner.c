#include "semantics.h"

/* Learner scaffold: every function is deliberately unfinished. It compiles
 * and returns its sentinel (FS_OUTSIDE or FS_NOT_IMPLEMENTED) so the public
 * tests fail until you implement the contract in semantics.h. */

enum fs_index fs_index(intmax_t index, size_t length)
{
    (void)index;
    (void)length;
    return FS_OUTSIDE;
}

enum fs_status fs_sum(const int *input, size_t length, int *out)
{
    (void)input;
    (void)length;
    (void)out;
    return FS_NOT_IMPLEMENTED;
}

enum fs_status fs_row(const int (*row)[3], int *out)
{
    (void)row;
    (void)out;
    return FS_NOT_IMPLEMENTED;
}

enum fs_status fs_matrix(const int (*rows)[3], size_t count, int *out)
{
    (void)rows;
    (void)count;
    (void)out;
    return FS_NOT_IMPLEMENTED;
}

enum fs_status fs_branch(int branch, int *out)
{
    (void)branch;
    (void)out;
    return FS_NOT_IMPLEMENTED;
}

enum fs_status fs_calls(struct fs_calls *out)
{
    (void)out;
    return FS_NOT_IMPLEMENTED;
}

enum fs_status fs_comma(int *value, int *final)
{
    (void)value;
    (void) final;
    return FS_NOT_IMPLEMENTED;
}
