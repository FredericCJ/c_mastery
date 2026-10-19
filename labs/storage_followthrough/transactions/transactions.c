#include "transactions.h"
int st_init(StTxn *t,store *s){(void)t;(void)s;return STORE_UNIMPLEMENTED;}
int st_begin(StTxn *t){(void)t;return STORE_UNIMPLEMENTED;}
int st_put(StTxn *t,const void *k,size_t n,const void *v,size_t z){(void)t;(void)k;(void)n;(void)v;(void)z;return STORE_UNIMPLEMENTED;}
int st_delete(StTxn *t,const void *k,size_t n){(void)t;(void)k;(void)n;return STORE_UNIMPLEMENTED;}
int st_savepoint(StTxn *t,StSavepoint *s){(void)t;(void)s;return STORE_UNIMPLEMENTED;}
int st_rollback(StTxn *t,StSavepoint s){(void)t;(void)s;return STORE_UNIMPLEMENTED;}
int st_release(StTxn *t,StSavepoint s){(void)t;(void)s;return STORE_UNIMPLEMENTED;}
int st_commit(StTxn *t){(void)t;return STORE_UNIMPLEMENTED;}
int st_abort(StTxn *t){(void)t;return STORE_UNIMPLEMENTED;}
