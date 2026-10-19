#include "protocol.h"
#include "checkpoint_depth.h"
#ifndef CPX_MUTANT
#define CPX_MUTANT 0
#endif
static uint32_t le(const unsigned char *p,unsigned n){uint32_t v=0;for(unsigned i=0;i<n;++i)v|=(uint32_t)p[i]<<(8*i);return v;}
static void put(unsigned char *p,uint32_t v,unsigned n){for(unsigned i=0;i<n;++i)p[i]=(unsigned char)(v>>(8*i));}
CPXProtocolStatus cpx_protocol_inplace(unsigned char *bytes,size_t capacity,size_t n,unsigned version,unsigned flags,uint16_t seq,uint32_t stamp,size_t *written){
 if(!bytes || !written || n>CPX_MAX_PAYLOAD || (version!=1 && version!=2) || flags>1 || (version==1 && flags))return CPX_P_INVALID;
 size_t header=version==1?6:7+(flags?4:0);size_t total=header+n+4;
 if(capacity<total)return CPX_P_FULL;
 for(size_t i=n;i>0;--i)bytes[header+i-1]=bytes[i-1];
 bytes[0]=0xa5;bytes[1]=(unsigned char)version;put(bytes+2,seq,2);put(bytes+4,(uint32_t)n,2);
 if(version==2){bytes[6]=(unsigned char)flags;if(flags)put(bytes+7,stamp,4);}
 size_t crc_length=header+n-1;if(CPX_MUTANT==5 && version==2)crc_length=n+5;
 put(bytes+header+n,cp_crc32(bytes+1,crc_length),4);*written=total;return CPX_P_OK;
}
CPXProtocolStatus cpx_protocol_encode(const CPXMessage *m,unsigned char *bytes,size_t capacity,size_t *written){
 if(!m || !bytes || !written || m->length>CPX_MAX_PAYLOAD || (m->version!=1 && m->version!=2) || m->flags>1 || (m->version==1 && m->flags))return CPX_P_INVALID;
 size_t total=m->length+(m->version==1?10:11+(m->flags?4:0));if(capacity<total)return CPX_P_FULL;
 for(size_t i=0;i<m->length;++i)bytes[i]=m->payload[i];
 return cpx_protocol_inplace(bytes,capacity,m->length,m->version,m->flags,m->sequence,m->stamp,written);
}
CPXProtocolStatus cpx_protocol_init(CPXParser *p,size_t limit){if(!p || limit>CPX_MAX_PAYLOAD)return CPX_P_INVALID;p->used=0;p->limit=limit;p->ready=0;return CPX_P_OK;}
static void remove_one(CPXParser *p){for(size_t i=1;i<p->used;++i)p->candidate[i-1]=p->candidate[i];--p->used;}
static CPXProtocolStatus inspect(CPXParser *p){
 while(p->used && p->candidate[0]!=0xa5)remove_one(p);
 if(p->used<2)return CPX_P_MORE;
 unsigned v=p->candidate[1];if(v!=1 && v!=2){remove_one(p);return CPX_P_REJECT;}
 if(p->used<6)return CPX_P_MORE;
 size_t n=le(p->candidate+4,2);if(n>p->limit){remove_one(p);return CPX_P_REJECT;}
 unsigned flags=0;size_t header=6;
 if(v==2){if(p->used<7)return CPX_P_MORE;flags=p->candidate[6];if(flags>1){remove_one(p);return CPX_P_REJECT;}header=7+(flags?4:0);}
 size_t total=header+n+4;if(p->used<total)return CPX_P_MORE;
 if(le(p->candidate+header+n,4)!=cp_crc32(p->candidate+1,header+n-1)){remove_one(p);return CPX_P_REJECT;}
 p->message.version=v;p->message.flags=flags;p->message.sequence=(uint16_t)le(p->candidate+2,2);p->message.stamp=flags?le(p->candidate+7,4):0;p->message.length=n;
 for(size_t i=0;i<n;++i)p->message.payload[i]=p->candidate[header+i];
 for(size_t i=total;i<p->used;++i)p->candidate[i-total]=p->candidate[i];
 p->used-=total;p->ready=1;return CPX_P_FRAME;
}
CPXProtocolStatus cpx_protocol_feed(CPXParser *p,const unsigned char *bytes,size_t n,size_t *used){
 if(!p || !used || (!bytes && n))return CPX_P_INVALID;
 *used=0;if(p->ready)return CPX_P_FRAME;
 for(;;){CPXProtocolStatus result=inspect(p);if(result!=CPX_P_MORE)return result;if(*used==n)return result;if(p->used==CPX_WIRE_MAX)return CPX_P_INVALID;p->candidate[p->used++]=bytes[(*used)++];}
}
CPXProtocolStatus cpx_protocol_queue(CPXQueue *q,size_t capacity){if(!q || !capacity || capacity>4)return CPX_P_INVALID;q->capacity=capacity;q->head=q->count=0;return CPX_P_OK;}
CPXProtocolStatus cpx_protocol_deliver(CPXParser *p,CPXQueue *q){
 if(!p || !q)return CPX_P_INVALID;
 if(!p->ready)return CPX_P_MORE;
 if(q->count==q->capacity){if(CPX_MUTANT==6)p->ready=0;return CPX_P_FULL;}
 q->items[(q->head+q->count)%q->capacity]=p->message;++q->count;p->ready=0;return CPX_P_OK;
}
CPXProtocolStatus cpx_protocol_take(CPXQueue *q,CPXMessage *out){if(!q || !out)return CPX_P_INVALID;if(!q->count)return CPX_P_MORE;*out=q->items[q->head];q->head=(q->head+1)%q->capacity;--q->count;return CPX_P_OK;}
