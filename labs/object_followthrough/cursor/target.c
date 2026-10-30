/*
 * Freestanding link-audit target: the entry point calls every function of
 * the cursor API (oc_read twice, once per row layout) so that the final link
 * must contain and resolve them. The image is inspected, never executed. No
 * library function may be needed.
 */
#include "cursor.h"

enum
{
    BYTE_COUNT = 4,
    ROW_COUNT = 2,
    COLUMN_COUNT = 3
};

static unsigned char bytes[BYTE_COUNT] = {1, 2, 3, 4};
static unsigned char copy[sizeof(int[ROW_COUNT][COLUMN_COUNT])];
static unsigned char wire[BYTE_COUNT];
static int values[ROW_COUNT][COLUMN_COUNT] = {{1, 2, 3}, {4, 5, 6}};
static int out;
static const unsigned char *view;
static int *nullable;
static ptrdiff_t difference;
static size_t lengths[ROW_COUNT] = {COLUMN_COUNT, COLUMN_COUNT};
static int *table[ROW_COUNT] = {values[0], values[1]};
/* Result sink: keeps every call observable so none can be discarded. */
volatile int oc_target_result;

void _start(void)
{
    struct oc_rows rows = {values, 2};
    struct oc_table fragmented = {table, lengths, 2};
    struct oc_matrix m = {&rows, 2, 3, oc_rows_read};
    struct fd_cursor c = {bytes, 4, 0};
    struct fd_cursor sub;
    int result = oc_read(&m, 1, 2, &out);
    m.context = &fragmented;
    m.read = oc_table_read;
    result += oc_read(&m, 1, 2, &out);
    result += oc_representation(&values, copy, sizeof copy);
    result += (int)oc_take(&c, 1, &view);
    result += (int)oc_subview(&c, 0, 2, &sub);
    result += (int)oc_previous(&c, 1, &view);
    result += oc_difference(bytes, 4, 4, 0, &difference);
    result += oc_order(lengths, 2, (struct oc_position){0, 3},
                       (struct oc_position){1, 0}, &out);
    result += oc_encode_index(values[0], 3, &values[0][2], wire);
    result += oc_decode_index(values[0], 3, wire, &nullable);
    result += oc_roundtrip(nullable);
#ifdef OC_TARGET_HELPER
    {
        extern void *malloc(size_t);
        nullable = malloc(4);
    }
#endif
    oc_target_result = result;
    for (;;)
    {
    }
}
