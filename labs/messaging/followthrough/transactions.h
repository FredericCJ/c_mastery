#ifndef CMASTERY_MESSAGING_TRANSACTIONS_H
#define CMASTERY_MESSAGING_TRANSACTIONS_H
#include "lifetime.h"
enum mft_phase {MFT_LOCKED,MFT_CLOCK,MFT_VALIDATED,MFT_PREPUBLISH,MFT_PAYLOAD,MFT_UNLOCKED,MFT_FINISHED};
typedef bool (*mft_hook_fn)(void *,enum mft_phase,unsigned);
enum msg_status mft_commit_phase(struct msg_txn *,mft_hook_fn,void *);
struct mft_store {
    pthread_mutex_t mutex;
    struct msg_store core;
    struct mfl_allocator allocator;
    struct mfl_shared *nodes[4];
    unsigned log[64],log_count,active;
};
struct mft_txn {
    struct mft_store *owner;
    struct msg_txn core;
    struct mfl_shared *reads[4],*writes[4];
    unsigned log[4],log_count;
    bool active;
};
typedef enum msg_status (*mft_body_fn)(void *,struct mft_txn *);
enum msg_status mft_init(struct mft_store *,struct mfl_allocator);
enum msg_status mft_begin(struct mft_store *,struct mft_txn *);
enum msg_status mft_read(struct mft_txn *,unsigned,const struct mfo_value **);
enum msg_status mft_write(struct mft_txn *,unsigned,struct mfo_value *);
enum msg_status mft_remove(struct mft_txn *,unsigned);
enum msg_status mft_log(struct mft_txn *,unsigned);
void mft_abort(struct mft_txn *);
enum msg_status mft_commit(struct mft_txn *,mft_hook_fn,void *);
enum msg_status mft_run(struct mft_store *,unsigned,mft_body_fn,void *,unsigned *);
enum msg_status mft_destroy(struct mft_store *);
#endif
