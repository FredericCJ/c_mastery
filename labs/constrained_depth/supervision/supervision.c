#include "supervision.h"
enum cs_status cs_init(struct cs_world*w,unsigned n,unsigned b){(void)w;(void)n;(void)b;return CS_UNIMPLEMENTED;}
enum cs_status cs_progress(struct cs_world*w,unsigned t){(void)w;(void)t;return CS_UNIMPLEMENTED;}
enum cs_status cs_window(struct cs_world*w){(void)w;return CS_UNIMPLEMENTED;}
enum cs_status cs_bitmap_init(struct cs_bitmap*w,unsigned n){(void)w;(void)n;return CS_UNIMPLEMENTED;}
enum cs_status cs_bitmap_progress(struct cs_bitmap*w,unsigned t){(void)w;(void)t;return CS_UNIMPLEMENTED;}
enum cs_status cs_bitmap_window(struct cs_bitmap*w){(void)w;return CS_UNIMPLEMENTED;}
enum cs_status cs_recovery_init(struct cs_recovery*r){(void)r;return CS_UNIMPLEMENTED;}
enum cs_status cs_failure(struct cs_recovery*r,enum cs_reason s){(void)r;(void)s;return CS_UNIMPLEMENTED;}
enum cs_status cs_energy(const struct cs_power*p,unsigned n,uint64_t*out){(void)p;(void)n;(void)out;return CS_UNIMPLEMENTED;}
