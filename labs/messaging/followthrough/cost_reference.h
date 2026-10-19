#ifndef CMASTERY_MESSAGING_COST_REFERENCE_H
#define CMASTERY_MESSAGING_COST_REFERENCE_H
#include "messaging.h"
typedef struct {pthread_mutex_t lock;unsigned values[2];} MfmPair;
typedef struct {pthread_mutex_t lock;unsigned values[8];size_t top,bottom;} MfmDeque;
void mfm_pair_init(MfmPair *);
void mfm_pair_write(MfmPair *,unsigned,unsigned);
bool mfm_pair_try_read(MfmPair *,unsigned [2]);
void mfm_pair_copy(MfmPair *,unsigned [2]);
void mfm_pair_increment(MfmPair *);
void mfm_pair_destroy(MfmPair *);
void mfm_deque_init(MfmDeque *);
MsgStatus mfm_push(MfmDeque *,unsigned);
MsgStatus mfm_steal(MfmDeque *,unsigned *);
void mfm_deque_destroy(MfmDeque *);
#endif
