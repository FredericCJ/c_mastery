#include "cost_reference.h"
#include <stdlib.h>
static void need(int rc) {if (rc) abort();}
void mfm_pair_init(MfmPair *p) {need(pthread_mutex_init(&p->lock,NULL));p->values[0]=p->values[1]=0;}
void mfm_pair_write(MfmPair *p,unsigned x,unsigned y) {
    need(pthread_mutex_lock(&p->lock));p->values[0]=x;p->values[1]=y;need(pthread_mutex_unlock(&p->lock));
}
bool mfm_pair_try_read(MfmPair *p,unsigned out[2]) {
    if (pthread_mutex_trylock(&p->lock)) return false;
    out[0]=p->values[0];out[1]=p->values[1];need(pthread_mutex_unlock(&p->lock));return true;
}
void mfm_pair_copy(MfmPair *p,unsigned out[2]) {
    need(pthread_mutex_lock(&p->lock));out[0]=p->values[0];out[1]=p->values[1];need(pthread_mutex_unlock(&p->lock));
}
void mfm_pair_increment(MfmPair *p) {
    need(pthread_mutex_lock(&p->lock));if (p->values[0]!=p->values[1]) abort();
    ++p->values[0];++p->values[1];need(pthread_mutex_unlock(&p->lock));
}
void mfm_pair_destroy(MfmPair *p) {need(pthread_mutex_destroy(&p->lock));}
void mfm_deque_init(MfmDeque *p) {need(pthread_mutex_init(&p->lock,NULL));p->top=p->bottom=1;}
MsgStatus mfm_push(MfmDeque *p,unsigned x) {
    need(pthread_mutex_lock(&p->lock));MsgStatus s=MSG_OK;
    if (p->bottom>=SIZE_MAX-1) s=MSG_OVERFLOW;
    else if (p->bottom-p->top==8) s=MSG_FULL;
    else p->values[p->bottom++%8]=x;
    need(pthread_mutex_unlock(&p->lock));return s;
}
MsgStatus mfm_steal(MfmDeque *p,unsigned *out) {
    need(pthread_mutex_lock(&p->lock));MsgStatus s=MSG_EMPTY;
    if (p->top!=p->bottom) {*out=p->values[p->top++%8];s=MSG_OK;}
    need(pthread_mutex_unlock(&p->lock));return s;
}
void mfm_deque_destroy(MfmDeque *p) {need(pthread_mutex_destroy(&p->lock));}
