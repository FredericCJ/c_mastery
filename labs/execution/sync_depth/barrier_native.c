#include "barrier_native.h"
/* Deliberately incomplete native publication/tree scaffold. */
int cdbn_init(CDBNative *p,unsigned n){(void)p;(void)n;return CMX_UNIMPLEMENTED;}
int cdbn_wait(CDBNative *p,unsigned n){(void)p;(void)n;return CMX_UNIMPLEMENTED;}
int cdbn_destroy(CDBNative *p){(void)p;return CMX_UNIMPLEMENTED;}
int cdbt_init(CDBTree *p){(void)p;return CMX_UNIMPLEMENTED;}
int cdbt_wait(CDBTree *p,unsigned n){(void)p;(void)n;return CMX_UNIMPLEMENTED;}
int cdbt_destroy(CDBTree *p){(void)p;return CMX_UNIMPLEMENTED;}
