#ifndef CMASTERY_MESSAGING_VERSIONS_H
#define CMASTERY_MESSAGING_VERSIONS_H
#include "lifetime.h"
typedef enum msg_status (*mfv_clone_fn)(void *, const void *,
                                        struct mfo_value *);

struct mfv_store
{
    pthread_mutex_t mutex;
    struct msg_versions core;
    struct mfl_shared *payloads[8];
    struct mfl_allocator allocator;
};
enum msg_status mfv_init(struct mfv_store *, struct mfl_allocator,
                         struct mfo_value *);
enum msg_status mfv_snapshot(struct mfv_store *, struct msg_snapshot *);
enum msg_status mfv_read(struct mfv_store *, struct msg_snapshot,
                         struct mfl_shared **);
enum msg_status mfv_drop_snapshot(struct mfv_store *, struct msg_snapshot);
enum msg_status mfv_publish(struct mfv_store *, mfv_clone_fn, void *,
                            const void *);
enum msg_status mfv_destroy(struct mfv_store *);

struct mfk_store
{
    pthread_mutex_t mutex;
    struct msg_store core;
};

struct mfk_txn
{
    struct mfk_store *owner;
    struct msg_txn core;
    unsigned snapshot[2], observed;
};
enum msg_status mfk_init(struct mfk_store *, unsigned, unsigned);
enum msg_status mfk_begin(struct mfk_store *, struct mfk_txn *);
enum msg_status mfk_read(struct mfk_txn *, unsigned, unsigned *);
enum msg_status mfk_nonzero(struct mfk_txn *, unsigned *);
enum msg_status mfk_write(struct mfk_txn *, unsigned, unsigned);
enum msg_status mfk_commit(struct mfk_txn *, bool);
void mfk_destroy(struct mfk_store *);
#endif
