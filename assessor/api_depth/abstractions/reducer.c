#include "reducer.h"
enum { REJECT,START,SEND,ACK,RETRY,FAIL,CANCEL };
static int valid(AmMachine m,AmEvent event,AmTransition *out) {
 return out&&m.state>=AM_IDLE&&m.state<=AM_FAILED&&event>=AM_START&&event<=AM_CANCEL&&
 m.sequence<=7&&(m.state!=AM_IDLE||m.sequence==0)&&
 ((m.state!=AM_WAIT&&m.state!=AM_FAILED)||m.sequence>0);
}
static AsStatus transition(AmMachine m,int action,AmTransition *out) {
 AmTransition next={m,{AM_NO_EFFECT,0}};
 switch(action) {
 case START:next.next.state=AM_READY;break;
 case SEND:
  if(m.sequence==7)return AS_FULL;
  ++next.next.sequence;next.next.state=AM_WAIT;
  next.effect=(AmEffect){AM_WRITE,next.next.sequence};break;
 case ACK:next.next.state=AM_READY;break;
 case RETRY:next.next.state=AM_WAIT;next.effect=(AmEffect){AM_WRITE,m.sequence};
#ifdef AM_MUTANT_RETRY_KEY
  ++next.effect.key;
#endif
  break;
 case FAIL:next.next.state=AM_FAILED;break;
 case CANCEL:
  /* Cancellation does not retract an issued effect: retain identity for query. */
  next.next.state=AM_FAILED;break;
 default:return AS_INVALID;
 }
 *out=next;return AS_OK;
}
AsStatus am_switch(AmMachine m,AmEvent e,AmTransition *out) {
 if(!valid(m,e,out))return AS_INVALID;
 int action=REJECT;
 switch(m.state) {
 case AM_IDLE:if(e==AM_START)action=START;break;
 case AM_READY:if(e==AM_SEND)action=SEND;break;
 case AM_WAIT:
  if(e==AM_ACK)action=ACK;else if(e==AM_TIMEOUT)action=RETRY;
  else if(e==AM_EFFECT_FAILURE)action=FAIL;else if(e==AM_CANCEL)action=CANCEL;
  break;
 case AM_FAILED:if(e==AM_TIMEOUT)action=RETRY;else if(e==AM_ACK)action=ACK;break;
 }
 return transition(m,action,out);
}
AsStatus am_table(AmMachine m,AmEvent e,AmTransition *out) {
 static const unsigned char actions[4][6]={{START,REJECT,REJECT,REJECT,REJECT,REJECT},
 {REJECT,SEND,REJECT,REJECT,REJECT,REJECT},{REJECT,REJECT,ACK,RETRY,FAIL,CANCEL},
 {REJECT,REJECT,ACK,RETRY,REJECT,REJECT}};
 if(!valid(m,e,out))return AS_INVALID;
 return transition(m,actions[m.state][e],out);
}
static int idle(AmEvent e){return e==AM_START?START:REJECT;}
static int ready(AmEvent e){return e==AM_SEND?SEND:REJECT;}
static int waiting(AmEvent e) {
 switch(e){case AM_ACK:return ACK;case AM_TIMEOUT:return RETRY;
 case AM_EFFECT_FAILURE:return FAIL;case AM_CANCEL:return CANCEL;default:return REJECT;}
}
static int failed(AmEvent e){return e==AM_TIMEOUT?RETRY:e==AM_ACK?ACK:REJECT;}
AsStatus am_functions(AmMachine m,AmEvent e,AmTransition *out) {
 static int(*const dispatch[4])(AmEvent)={idle,ready,waiting,failed};
 if(!valid(m,e,out))return AS_INVALID;
 return transition(m,dispatch[m.state](e),out);
}
AsStatus am_sparse(unsigned state,unsigned sequence,AmEvent e,AmTransition *out) {
 static const unsigned identities[4]={11,29,71,101};
 for(unsigned i=0;i<4;++i)if(state==identities[i])return am_table((AmMachine){(AmState)i,sequence},e,out);
 return AS_INVALID;
}
AsStatus am_execute(AmBackend *b,AmEffect effect) {
 if(!b||effect.kind!=AM_WRITE||!effect.key||effect.key>7||b->attempts>=31||
    b->committed_key>7||b->value!=b->committed_key)return AS_INVALID;
 if(effect.key!=b->committed_key&&effect.key!=b->committed_key+1)return AS_INVALID;
 ++b->attempts;
 if(effect.key==b->committed_key) {
#ifndef AM_MUTANT_REPLAY
  return AS_OK;
#endif
 }
 if(b->fail_before){b->fail_before=0;return AS_FAILED;}
 ++b->value;b->committed_key=effect.key;
 if(b->lose_ack){b->lose_ack=0;return AS_FAILED;}
 return AS_OK;
}
AsStatus am_hot_acquire(AmHot *h,AmLease *out) {
 if(!h||!out||!h->current.execute||!h->current.epoch||h->inflight_current>=3)return AS_INVALID;
 ++h->inflight_current;*out=(AmLease){h->current,1};return AS_OK;
}
AsStatus am_hot_replace(AmHot *h,AmStrategy next) {
 if(!h||!h->current.execute||!h->current.release||!h->current.context||
    !next.execute||!next.release||!next.context||next.context==h->current.context||!next.epoch||
    next.epoch<=h->current.epoch||h->retired.execute)return AS_INVALID;
 AmStrategy old=h->current;
 if(h->inflight_current){h->retired=old;h->inflight_retired=h->inflight_current;}
 h->current=next;h->inflight_current=0;
 if(!h->inflight_retired&&old.release)old.release(old.context);
 return AS_OK;
}
AsStatus am_hot_release(AmHot *h,AmLease *lease) {
 if(!h||!lease||!lease->live)return AS_INVALID;
 unsigned epoch=lease->selected.epoch;
 if(epoch==h->current.epoch&&h->inflight_current)--h->inflight_current;
 else if(epoch==h->retired.epoch&&h->inflight_retired) {
  --h->inflight_retired;
  if(!h->inflight_retired) {
   AmStrategy old=h->retired;h->retired=(AmStrategy){0};
   lease->live=0;old.release(old.context);return AS_OK;
  }
 }else return AS_INVALID;
 lease->live=0;return AS_OK;
}
AsStatus am_hot_close(AmHot *h) {
 if(!h||h->inflight_current||h->inflight_retired)return AS_INVALID;
 AmStrategy old=h->current;h->current=(AmStrategy){0};
 if(old.release)old.release(old.context);
 return AS_OK;
}
