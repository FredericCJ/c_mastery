#include "owned.h"
int nv_owned_init(NVOwned *p){(void)p;return NV_UNIMPLEMENTED;}
int nv_owned_admit(NVOwned *p,int f,HFHandle *h){(void)p;(void)f;(void)h;return NV_UNIMPLEMENTED;}
int nv_owned_borrow(NVOwned *p,HFHandle h,NVLease *l){(void)p;(void)h;(void)l;return NV_UNIMPLEMENTED;}
int nv_owned_close(NVOwned *p,HFHandle h){(void)p;(void)h;return NV_UNIMPLEMENTED;}
int nv_owned_release(NVLease *l){(void)l;return NV_UNIMPLEMENTED;}
int nv_owned_destroy(NVOwned *p){(void)p;return NV_UNIMPLEMENTED;}
