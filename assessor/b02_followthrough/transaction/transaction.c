#include "transaction.h"
#ifndef BF_MUTANT
#define BF_MUTANT 0
#endif
int bf_construct(unsigned style,B2Backend b,unsigned*out){
 if(style>3||!out||!b.acquire||!b.release)return B2_INVALID;
 if(style==0)return b2_transaction(b,out);
 if(style==1){
  if(b.acquire(b.context,0))return B2_FAILED;
  if(b.acquire(b.context,1)){if(BF_MUTANT!=2)b.release(b.context,0);return B2_FAILED;}
  if(b.acquire(b.context,2)){b.release(b.context,1);b.release(b.context,0);return B2_FAILED;}
  *out=3;return B2_OK;
 }
 if(style==2){
  if(!b.acquire(b.context,0)){
   if(!b.acquire(b.context,1)){
    if(!b.acquire(b.context,2)){*out=3;return B2_OK;}
    b.release(b.context,1);
   }
   b.release(b.context,0);
  }
  return B2_FAILED;
 }
 if(b.acquire(b.context,0))goto failed0;
 if(b.acquire(b.context,1))goto failed1;
 if(b.acquire(b.context,2))goto failed2;
 *out=3;return B2_OK;
 failed2:b.release(b.context,1);
 failed1:b.release(b.context,0);
 failed0:return B2_FAILED;
}
void bf_tx_init(BfTx*t){*t=(BfTx){0};t->stage=4;}
static int cleanup(BfTx*t){
 t->state=BF_TX_PENDING;
 for(unsigned i=3;i;){--i;if(!(t->mask&(1u<<i)))continue;
  int error=t->backend.release(t->backend.context,i);
  if(error){t->failed_cleanup|=1u<<i;t->cleanup_error=error;
   if(BF_MUTANT==4)t->primary=error;
   if(BF_MUTANT==5)t->mask&=~(1u<<i);
   if(BF_MUTANT!=6)return BF_TX_RECOVERY;
  }else t->mask&=~(1u<<i);
 }
 if(t->mask)return BF_TX_RECOVERY;
 t->state=BF_TX_DONE;return t->primary?B2_FAILED:B2_OK;
}
int bf_tx_begin(BfTx*t,BfTxBackend b,unsigned*out){
 if(!t||!out||t->state!=BF_TX_IDLE||!b.acquire||!b.release||!b.commit)return B2_INVALID;
 t->backend=b;
 if(BF_MUTANT==3)*out=3;
 for(unsigned i=0;i<3;++i){int error=b.acquire(b.context,i);if(error){t->primary=error;t->stage=i;return cleanup(t);}t->mask|=1u<<i;}
 int error=b.commit(b.context);if(error){t->primary=error;t->stage=3;return cleanup(t);}
 t->state=BF_TX_LIVE;*out=3;return B2_OK;
}
int bf_tx_close(BfTx*t){
 if(!t||t->state==BF_TX_IDLE)return B2_INVALID;
 if(t->state==BF_TX_DONE)return t->primary?B2_FAILED:B2_OK;
 return cleanup(t);
}
