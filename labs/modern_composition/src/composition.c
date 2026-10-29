#include "composition.h"

/* Complete these adapters using your own four prerequisite implementations. */
struct mc_publish mc_publish_pair(const unsigned char *p, size_t n,
                                  struct v4_allocator a, struct ow_owner *o)
{
    (void)p;
    (void)n;
    (void)a;
    (void)o;
    struct mc_publish result = {er_error(ER_NOT_IMPLEMENTED, 0),
                                OW_NOT_IMPLEMENTED, false};
    return result;
}

enum msg_status mc_read_init(struct mc_read *r, struct co_async_port p,
                             size_t n, uint32_t g, unsigned char out[8],
                             msg_receiver_fn f, void *ctx,
                             ow_cleanup_fn cleanup, void *cleanup_ctx)
{
    (void)r;
    (void)p;
    (void)n;
    (void)g;
    (void)out;
    (void)f;
    (void)ctx;
    (void)cleanup;
    (void)cleanup_ctx;
    return MSG_FAILED;
}

enum msg_status mc_read_poll(struct mc_read *r)
{
    (void)r;
    return MSG_FAILED;
}

enum msg_status mc_read_cancel(struct mc_read *r)
{
    (void)r;
    return MSG_FAILED;
}
