#include "barrier_native.h"
/* Deliberately incomplete native publication/tree scaffold. */
int cdbn_init(struct cdb_native *p,unsigned n){(void)p;(void)n;return CMX_UNIMPLEMENTED;}
int cdbn_wait(struct cdb_native *p,unsigned n){(void)p;(void)n;return CMX_UNIMPLEMENTED;}
int cdbn_destroy(struct cdb_native *p){(void)p;return CMX_UNIMPLEMENTED;}
int cdbt_init(struct cdb_tree *p){(void)p;return CMX_UNIMPLEMENTED;}
int cdbt_wait(struct cdb_tree *p,unsigned n){(void)p;(void)n;return CMX_UNIMPLEMENTED;}
int cdbt_destroy(struct cdb_tree *p){(void)p;return CMX_UNIMPLEMENTED;}
