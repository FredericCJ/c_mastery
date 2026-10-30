/* Link-audit entry only: no reset startup/vector table and no hardware access.
 * It exists so the final link pulls in fd_extent, fd_encode, fd_decode,
 * fd_subview, fd_take, fd_transform, fd_move, fd_pool_init, fd_pool_acquire,
 * fd_pool_get, fd_pool_release and fd_apply; the status words below keep the
 * calls observable to the optimizer. Its results are never inspected, and it
 * is never run. */
#include "depth.h"

volatile unsigned fd_audit_status;
volatile int32_t fd_audit_value;
static struct fd_pool pool;

/* Arbitrary inputs that satisfy each call's preconditions. */
#define AUDIT_COUNTER_START 10
#define AUDIT_MOVE_COUNT 7u

void _start(void)
{
    unsigned char wire[FD_WIRE_BYTES];
    unsigned char moved[FD_WIRE_BYTES];
    size_t extent;
    const struct fd_record input = {3, 2, -3};
    struct fd_record decoded;
    struct fd_handle handle;
    struct fd_value value;
    struct fd_cursor cursor;
    struct fd_cursor sub;
    const unsigned char *view;
    int32_t counter = AUDIT_COUNTER_START;

    fd_audit_status = (unsigned)fd_extent(
        FD_WIRE_HEADER_BYTES, 1, FD_WIRE_PAYLOAD_BYTES, FD_WIRE_BYTES, &extent);
    fd_audit_status += (unsigned)fd_encode(input, wire, FD_WIRE_BYTES);
    fd_audit_status += (unsigned)fd_decode(wire, FD_WIRE_BYTES, &decoded);
    cursor.base = wire;
    cursor.length = FD_WIRE_BYTES;
    cursor.position = 0;
    fd_audit_status += (unsigned)fd_subview(&cursor, 0, FD_WIRE_BYTES, &sub);
    fd_audit_status += (unsigned)fd_take(&sub, FD_WIRE_BYTES, &view);
    fd_transform(moved, wire, FD_WIRE_BYTES, 1);
    fd_audit_status +=
        (unsigned)fd_move(moved, FD_WIRE_BYTES, 1, 0, AUDIT_MOVE_COUNT);
    fd_pool_init(&pool);
    value.value = decoded.value;
    value.serial = decoded.count;
    fd_audit_status += (unsigned)fd_pool_acquire(&pool, value, &handle);
    fd_audit_status += (unsigned)fd_pool_get(&pool, handle, &value);
    fd_audit_status += (unsigned)fd_pool_release(&pool, handle);
    fd_audit_status += (unsigned)fd_apply(wire, FD_WIRE_BYTES, 1, &counter);
    fd_audit_value = counter;
    for (;;)
    {
    }
}
