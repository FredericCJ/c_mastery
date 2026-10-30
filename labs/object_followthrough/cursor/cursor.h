#ifndef OC_CURSOR_H
#define OC_CURSOR_H
#include "depth.h"

/*
 * Contract conventions. Unless a comment says otherwise an int function
 * returns 0 on success and -1 when admitted metadata or a range is rejected,
 * and a rejected request leaves every output as it was. Cursor functions
 * return enum fd_status (FD_INVALID for a bad argument, FD_RANGE for a
 * request outside the cursor) under the same rule. A required pointer is
 * NULL (invalid) or points to a live object of the stated type with a
 * truthful extent; native addresses are never validated. Single owner,
 * synchronous, not re-entrant. CONTRACTS.md is the normative text.
 */

/* A matrix stored as `count` contiguous rows of exactly three ints. Owns no
 * storage. */
struct oc_rows
{
    int (*rows)[3];
    size_t count;
};

/* A matrix stored as `count` separately allocated rows; lengths[i] is the
 * number of ints in rows[i]. Owns no storage. */
struct oc_table
{
    int **rows;
    const size_t *lengths;
    size_t count;
};

/*
 * A logical rows x columns rectangle read through a layout-specific callback.
 * context is passed to read unchanged; read must follow the same
 * "-1 and output preserved" rule and must not modify its context.
 */
struct oc_matrix
{
    void *context;
    size_t rows, columns;
    int (*read)(void *context, size_t row, size_t col, int *out);
};

/* A place in a sequence of fragments: segment number and index within it. */
struct oc_position
{
    size_t segment, index;
};

/* Callbacks for struct oc_matrix; context points to the matching struct. */
int oc_rows_read(void *context, size_t row, size_t col, int *out);
int oc_table_read(void *context, size_t row, size_t col, int *out);

/* Reads element (row, col) after checking both against the rectangle. */
int oc_read(const struct oc_matrix *matrix, size_t row, size_t col, int *out);

/*
 * Copies the object representation of the whole int[2][3] through character
 * access. capacity_bytes must equal sizeof(int[2][3]). The bytes are an
 * observation, not a portable wire format.
 */
int oc_representation(int (*matrix)[2][3], unsigned char *out,
                      size_t capacity_bytes);

/* Wrappers over the foundation fd_take() and fd_subview() contracts. */
enum fd_status oc_take(struct fd_cursor *cursor, size_t count,
                       const unsigned char **out);
enum fd_status oc_subview(const struct fd_cursor *cursor, size_t offset,
                          size_t count, struct fd_cursor *out);

/*
 * Steps the cursor back by count bytes: *out receives the start of the
 * preceding count bytes and position decreases by count. FD_RANGE, with
 * cursor and *out unchanged, if count exceeds the position; FD_INVALID for
 * NULL arguments or an inconsistent cursor. A zero count at the beginning
 * forms no pointer before the base.
 */
enum fd_status oc_previous(struct fd_cursor *cursor, size_t count,
                           const unsigned char **out);

/*
 * left - right for two indices (each at most length) into one byte array of
 * length elements; base must not be NULL. *out is signed; rejected if an
 * index exceeds length or the magnitude of the difference is not
 * representable in ptrdiff_t.
 */
int oc_difference(const unsigned char *base, size_t length, size_t left,
                  size_t right, ptrdiff_t *out);

/*
 * Compares two positions in fragments of the given lengths (elements,
 * count fragments) by their logical offsets: *out is -1, 0 or 1. Rejected
 * for a segment outside lengths, an index beyond its fragment, or a total
 * length that does not fit size_t. No address is formed or compared.
 */
int oc_order(const size_t *lengths, size_t count, struct oc_position left,
             struct oc_position right, int *out);

/*
 * Writes a nullable element reference as four big-endian bytes: the index of
 * pointer within base[0..count), or the all-ones value for NULL. Rejected if
 * count exceeds UINT32_MAX or pointer is neither NULL nor an element of base.
 * Native pointer bytes are never serialized.
 */
int oc_encode_index(int *base, size_t count, const int *pointer,
                    unsigned char *out);

/*
 * Reads four big-endian bytes and sets *out to the matching element of
 * base[0..count), or NULL for the null encoding. An index not below count is
 * rejected. The decoded pointer borrows base, which must outlive it.
 */
int oc_decode_index(int *base, size_t count, const unsigned char *wire,
                    int **out);

/* Round-trips a pointer through void * and a function pointer through
 * another function pointer type. Returns 0 if both are restored, else -1. */
int oc_roundtrip(int *pointer);
#endif
