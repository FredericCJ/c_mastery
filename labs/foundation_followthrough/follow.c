#include "follow.h"
enum fd_status ff_interval(size_t a,size_t b,size_t c,size_t d,struct ff_interval *out) {
    (void)a;(void)b;(void)c;(void)d;(void)out;return FD_STATE;
}
int ff_copy_initialize(void *context,struct fd_value *destination) {
    (void)context;(void)destination;return 1;
}
enum fd_status ff_encode(struct fd_record record,unsigned char *out,size_t capacity) {
    (void)record;(void)out;(void)capacity;return FD_STATE;
}
enum fd_status ff_decode(const unsigned char *wire,size_t length,struct fd_record *out) {
    (void)wire;(void)length;(void)out;return FD_STATE;
}
