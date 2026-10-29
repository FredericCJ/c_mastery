#include "halfduplex.h"
#ifndef CPX_MUTANT
#define CPX_MUTANT 0
#endif
enum cpf_status cpx_halfduplex_begin(struct cpx_half_duplex *h,const struct cp_message *m,uint16_t delay,cpx_direction_fn direction,void *context){
 if(!h || !m || !direction || h->driving || !delay || delay>=32768)return CPF_INVALID;
 cpf_tx_init(&h->tx,3,1);enum cpf_status status=cpf_tx_begin(&h->tx,m,0);if(status!=CPF_OK)return status;
 h->turnaround=delay;h->holding=0;h->driving=1;h->direction=direction;h->context=context;direction(context,1);return CPF_OK;
}
enum cpf_status cpx_halfduplex_step(struct cpx_half_duplex *h,uint16_t now,int txe,int tc,cpf_write_fn write,void *context){
 if(!h || !h->driving || !write)return CPF_INVALID;
 if(!h->holding){
  enum cpf_status result=cpf_tx_poll(&h->tx,h->tx.generation,txe,tc,write,context);
  if(CPX_MUTANT==11 && h->tx.state==CPF_WIRE){h->direction(h->context,0);h->driving=0;return CPF_OK;}
  if(result!=CPF_OK)return result;
  h->holding=1;h->start=now;
 }
 uint16_t elapsed;if(!cpf_elapsed16(h->start,now,&elapsed))return CPF_PENDING;
 if(elapsed<h->turnaround && CPX_MUTANT!=12)return CPF_PENDING;
 h->direction(h->context,0);h->driving=0;return CPF_OK;
}
