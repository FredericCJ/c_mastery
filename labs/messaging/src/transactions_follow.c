#include "transactions.h"
enum msg_status mft_commit_phase(struct msg_txn*t,mft_hook_fn h,void*c){(void)t;(void)h;(void)c;return MSG_INVALID;}
enum msg_status mft_init(struct mft_store*s,struct mfl_allocator a){(void)s;(void)a;return MSG_INVALID;}
enum msg_status mft_begin(struct mft_store*s,struct mft_txn*t){(void)s;(void)t;return MSG_INVALID;}
enum msg_status mft_read(struct mft_txn*t,unsigned k,const struct mfo_value**v){(void)t;(void)k;(void)v;return MSG_INVALID;}
enum msg_status mft_write(struct mft_txn*t,unsigned k,struct mfo_value*v){(void)t;(void)k;(void)v;return MSG_INVALID;}
enum msg_status mft_remove(struct mft_txn*t,unsigned k){(void)t;(void)k;return MSG_INVALID;}
enum msg_status mft_log(struct mft_txn*t,unsigned e){(void)t;(void)e;return MSG_INVALID;}
void mft_abort(struct mft_txn*t){(void)t;}
enum msg_status mft_commit(struct mft_txn*t,mft_hook_fn h,void*c){(void)t;(void)h;(void)c;return MSG_INVALID;}
enum msg_status mft_run(struct mft_store*s,unsigned l,mft_body_fn b,void*c,unsigned*a){(void)s;(void)l;(void)b;(void)c;(void)a;return MSG_INVALID;}
enum msg_status mft_destroy(struct mft_store*s){(void)s;return MSG_INVALID;}
