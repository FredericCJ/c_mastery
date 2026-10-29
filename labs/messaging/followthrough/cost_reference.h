#ifndef CMASTERY_MESSAGING_COST_REFERENCE_H
#define CMASTERY_MESSAGING_COST_REFERENCE_H
#include "messaging.h"

struct mfm_pair
{
    pthread_mutex_t lock;
    unsigned values[2];
};

struct mfm_deque
{
    pthread_mutex_t lock;
    unsigned values[8];
    size_t top, bottom;
};

void mfm_pair_init(struct mfm_pair *);
void mfm_pair_write(struct mfm_pair *, unsigned, unsigned);
bool mfm_pair_try_read(struct mfm_pair *, unsigned[2]);
void mfm_pair_copy(struct mfm_pair *, unsigned[2]);
void mfm_pair_increment(struct mfm_pair *);
void mfm_pair_destroy(struct mfm_pair *);
void mfm_deque_init(struct mfm_deque *);
enum msg_status mfm_push(struct mfm_deque *, unsigned);
enum msg_status mfm_steal(struct mfm_deque *, unsigned *);
void mfm_deque_destroy(struct mfm_deque *);
#endif
