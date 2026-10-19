#include "supervision.h"
enum cs_status cs_init(CSWorld*w,unsigned n,unsigned b){(void)w;(void)n;(void)b;return CS_UNIMPLEMENTED;}
enum cs_status cs_progress(CSWorld*w,unsigned t){(void)w;(void)t;return CS_UNIMPLEMENTED;}
enum cs_status cs_window(CSWorld*w){(void)w;return CS_UNIMPLEMENTED;}
enum cs_status cs_bitmap_init(CSBitmap*w,unsigned n){(void)w;(void)n;return CS_UNIMPLEMENTED;}
enum cs_status cs_bitmap_progress(CSBitmap*w,unsigned t){(void)w;(void)t;return CS_UNIMPLEMENTED;}
enum cs_status cs_bitmap_window(CSBitmap*w){(void)w;return CS_UNIMPLEMENTED;}
enum cs_status cs_recovery_init(CSRecovery*r){(void)r;return CS_UNIMPLEMENTED;}
enum cs_status cs_failure(CSRecovery*r,enum cs_reason s){(void)r;(void)s;return CS_UNIMPLEMENTED;}
enum cs_status cs_energy(const CSPower*p,unsigned n,uint64_t*out){(void)p;(void)n;(void)out;return CS_UNIMPLEMENTED;}
