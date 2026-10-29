#include "transactions.h"
int st_init(struct st_txn *t,store *s){(void)t;(void)s;return STORE_UNIMPLEMENTED;}
int st_begin(struct st_txn *t){(void)t;return STORE_UNIMPLEMENTED;}
int st_put(struct st_txn *t,const void *k,size_t n,const void *v,size_t z){(void)t;(void)k;(void)n;(void)v;(void)z;return STORE_UNIMPLEMENTED;}
int st_delete(struct st_txn *t,const void *k,size_t n){(void)t;(void)k;(void)n;return STORE_UNIMPLEMENTED;}
int st_savepoint(struct st_txn *t,struct st_savepoint *s){(void)t;(void)s;return STORE_UNIMPLEMENTED;}
int st_rollback(struct st_txn *t,struct st_savepoint s){(void)t;(void)s;return STORE_UNIMPLEMENTED;}
int st_release(struct st_txn *t,struct st_savepoint s){(void)t;(void)s;return STORE_UNIMPLEMENTED;}
int st_commit(struct st_txn *t){(void)t;return STORE_UNIMPLEMENTED;}
int st_abort(struct st_txn *t){(void)t;return STORE_UNIMPLEMENTED;}
