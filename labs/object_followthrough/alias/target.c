/*
 * Freestanding link-audit target: the entry point calls the alias API (all
 * of it except the optional os_tag_roundtrip experiment) so that the final
 * link must contain and resolve those functions. The image is inspected,
 * never executed. No library function may be needed.
 */
#include "alias.h"

enum
{
    SCRATCH_BYTES = 32,
    TRANSFORM_BYTES = 4,
    DESTINATION_OFFSET = 16,
    XOR_MASK = 0x55,
    WIDE_ALIGNMENT_BYTES = 64
};

static struct fd_slot slot = FD_SLOT_INITIALIZER;
static struct os_wide_slot wide;
static unsigned char bytes[SCRATCH_BYTES];
static unsigned char wire[8] = {0, 0, 0, 7, 0, 0, 0, 11};
static struct fd_value value;
static struct fd_pool pool;
static struct fd_handle handle;
static struct os_container owner = {1, 17};
static int output;

static void *none(void *c, size_t s, size_t a)
{
    (void)c;
    (void)s;
    (void)a;
    return NULL;
}

static void release(void *c, void *p)
{
    (void)c;
    (void)p;
}

static int init(void *c, struct fd_value *p)
{
    (void)c;
    p->value = 7;
    p->serial = 11;
    return 0;
}

static void drop(void *c, struct fd_value *p)
{
    (void)c;
    (void)p;
}

static int wide_drop(void *c, struct os_wide *p)
{
    (void)c;
    (void)p;
    return 0;
}

static void *(*volatile allocation_call)(void *, size_t, size_t) = none;
volatile int os_target_result;

void _start(void)
{
    struct os_allocator a = {
        {NULL, none, release}, sizeof(struct os_wide), WIDE_ALIGNMENT_BYTES};
    const struct fd_value *view = NULL;
    const struct os_wide *borrow = NULL;
    int result = os_acquire(&slot, a);
    result += (int)fd_slot_initialize(&slot, init, drop, NULL);
    result += (int)fd_slot_publish(&slot, &view);
    fd_slot_reset(&slot);
    result += os_wide_acquire(&wide, a, 17, wide_drop, NULL);
    result += os_wide_borrow(&wide, &borrow);
    result += os_wide_return(&wide, &borrow);
    result += os_wide_reset(&wide);
    result += os_wire(wire, 8, &value);
    result += (int)os_host_representation(7);
    os_transform(bytes + DESTINATION_OFFSET, bytes, TRANSFORM_BYTES, XOR_MASK);
    os_ordinary(bytes + DESTINATION_OFFSET, bytes, TRANSFORM_BYTES, XOR_MASK);
    os_nested(bytes + DESTINATION_OFFSET, bytes, TRANSFORM_BYTES, XOR_MASK);
    os_shared(bytes + DESTINATION_OFFSET, bytes, bytes, TRANSFORM_BYTES);
    result += os_partition(bytes, SCRATCH_BYTES, DESTINATION_OFFSET, 0,
                           TRANSFORM_BYTES, XOR_MASK, 0);
    result += (int)os_move(bytes, SCRATCH_BYTES, 2, 0, TRANSFORM_BYTES);
    fd_pool_init(&pool);
    result += (int)fd_pool_acquire(&pool, value, &handle);
    result += (int)fd_pool_get(&pool, handle, &value);
    result += (int)fd_pool_release(&pool, handle);
    result += os_member(&owner, &owner.member, &output);
    /* os_clone must stay linked, but is called only if the allocation
     * callback (reached through a volatile pointer, so the compiler cannot
     * prove it returns NULL) produced storage. */
    {
        void *allocated =
            allocation_call(a.allocator.context, sizeof(struct fd_value),
                            _Alignof(struct fd_value));
        if (allocated)
        {
            result += os_clone(allocated, &value);
            a.allocator.deallocate(a.allocator.context, allocated);
        }
    }
#ifdef OS_TARGET_HELPER
    {
        extern void *malloc(size_t);
        void *p = malloc(4);
        if (p)
        {
            result++;
        }
    }
#endif
    os_target_result = result;
    for (;;)
    {
    }
}
