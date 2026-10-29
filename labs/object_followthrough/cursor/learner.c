#include "cursor.h"

int oc_rows_read(void *a, size_t b, size_t c, int *d)
{
    (void)a;
    (void)b;
    (void)c;
    (void)d;
    return -1;
}

int oc_table_read(void *a, size_t b, size_t c, int *d)
{
    (void)a;
    (void)b;
    (void)c;
    (void)d;
    return -1;
}

int oc_read(const struct oc_matrix *a, size_t b, size_t c, int *d)
{
    (void)a;
    (void)b;
    (void)c;
    (void)d;
    return -1;
}

int oc_representation(int (*a)[2][3], unsigned char *b, size_t c)
{
    (void)a;
    (void)b;
    (void)c;
    return -1;
}

enum fd_status oc_take(struct fd_cursor *a, size_t b, const unsigned char **c)
{
    (void)a;
    (void)b;
    (void)c;
    return FD_NOT_IMPLEMENTED;
}

enum fd_status oc_subview(const struct fd_cursor *a, size_t b, size_t c,
                          struct fd_cursor *d)
{
    (void)a;
    (void)b;
    (void)c;
    (void)d;
    return FD_NOT_IMPLEMENTED;
}

enum fd_status oc_previous(struct fd_cursor *a, size_t b,
                           const unsigned char **c)
{
    (void)a;
    (void)b;
    (void)c;
    return FD_NOT_IMPLEMENTED;
}

int oc_difference(const unsigned char *a, size_t b, size_t c, size_t d,
                  ptrdiff_t *e)
{
    (void)a;
    (void)b;
    (void)c;
    (void)d;
    (void)e;
    return -1;
}

int oc_order(const size_t *a, size_t b, struct oc_position c,
             struct oc_position d, int *e)
{
    (void)a;
    (void)b;
    (void)c;
    (void)d;
    (void)e;
    return -1;
}

int oc_encode_index(int *a, size_t b, const int *c, unsigned char *d)
{
    (void)a;
    (void)b;
    (void)c;
    (void)d;
    return -1;
}

int oc_decode_index(int *a, size_t b, const unsigned char *c, int **d)
{
    (void)a;
    (void)b;
    (void)c;
    (void)d;
    return -1;
}

int oc_roundtrip(int *a)
{
    (void)a;
    return -1;
}
