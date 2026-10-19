#include "follow.h"
FDStatus ff_interval(size_t a,size_t b,size_t c,size_t d,FFInterval *out) {
    (void)a;(void)b;(void)c;(void)d;(void)out;return FD_STATE;
}
int ff_copy_initialize(void *context,FDValue *destination) {
    (void)context;(void)destination;return 1;
}
FDStatus ff_encode(FDRecord record,unsigned char *out,size_t capacity) {
    (void)record;(void)out;(void)capacity;return FD_STATE;
}
FDStatus ff_decode(const unsigned char *wire,size_t length,FDRecord *out) {
    (void)wire;(void)length;(void)out;return FD_STATE;
}
