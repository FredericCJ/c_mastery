#include "api_depth.h"
enum api_status api_append(struct api_buffer *b, const unsigned char *s, size_t n) {
    (void)b; (void)s; (void)n; return API_UNIMPLEMENTED;
}
void api_registry_init(struct api_registry *r) { (void)r; }
enum api_status api_add(struct api_registry *r, api_callback_fn f, void *c, api_drop_fn d, struct api_handle *h) {
    (void)r; (void)f; (void)c; (void)d; (void)h; return API_UNIMPLEMENTED;
}
enum api_status api_remove(struct api_registry *r, struct api_handle h) { (void)r; (void)h; return API_UNIMPLEMENTED; }
enum api_status api_dispatch(struct api_registry *r) { (void)r; return API_UNIMPLEMENTED; }
enum api_status api_destroy(struct api_registry *r) { (void)r; return API_UNIMPLEMENTED; }
