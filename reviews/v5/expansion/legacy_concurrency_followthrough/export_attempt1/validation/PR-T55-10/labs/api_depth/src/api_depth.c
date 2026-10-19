#include "api_depth.h"
ApiStatus api_append(ApiBuffer *b, const unsigned char *s, size_t n) {
    (void)b; (void)s; (void)n; return API_UNIMPLEMENTED;
}
void api_registry_init(ApiRegistry *r) { (void)r; }
ApiStatus api_add(ApiRegistry *r, ApiCallback f, void *c, ApiDrop d, ApiHandle *h) {
    (void)r; (void)f; (void)c; (void)d; (void)h; return API_UNIMPLEMENTED;
}
ApiStatus api_remove(ApiRegistry *r, ApiHandle h) { (void)r; (void)h; return API_UNIMPLEMENTED; }
ApiStatus api_dispatch(ApiRegistry *r) { (void)r; return API_UNIMPLEMENTED; }
ApiStatus api_destroy(ApiRegistry *r) { (void)r; return API_UNIMPLEMENTED; }
