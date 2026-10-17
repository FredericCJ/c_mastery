#include "composition.h"
/* Complete these adapters using your own four prerequisite implementations. */
McPublish mc_publish_pair(const unsigned char *p, size_t n, V4Allocator a, OwOwner *o) {
    (void)p; (void)n; (void)a; (void)o;
    McPublish result = {er_error(ER_NOT_IMPLEMENTED, 0), OW_NOT_IMPLEMENTED, false};
    return result;
}
MsgStatus mc_read_init(McRead *r, COAsyncPort p, size_t n, uint32_t g,
                       unsigned char out[8], MsgReceiver f, void *ctx,
                       OwCleanup cleanup, void *cleanup_ctx) {
    (void)r; (void)p; (void)n; (void)g; (void)out; (void)f; (void)ctx;
    (void)cleanup; (void)cleanup_ctx;
    return MSG_FAILED;
}
MsgStatus mc_read_poll(McRead *r) { (void)r; return MSG_FAILED; }
MsgStatus mc_read_cancel(McRead *r) { (void)r; return MSG_FAILED; }
