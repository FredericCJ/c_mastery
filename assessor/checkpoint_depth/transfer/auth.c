#include "auth.h"
#ifndef CPX_MUTANT
#define CPX_MUTANT 0
#endif
static void put(unsigned char *p,uint64_t v,unsigned n){for(unsigned i=0;i<n;++i)p[i]=(unsigned char)(v>>(8*i));}
static uint64_t get(const unsigned char *p,unsigned n){uint64_t v=0;for(unsigned i=0;i<n;++i)v|=(uint64_t)p[i]<<(8*i);return v;}
enum cpx_auth_status cpx_auth_sign(const struct cpx_auth_state *s,uint64_t sequence,const unsigned char *payload,size_t n,unsigned char *out,size_t capacity,size_t *written,cpx_mac_fn mac){
 if(!s || !written || !mac || !out || (!payload && n) || n>CPX_AUTH_PAYLOAD)return CPX_AUTH_INVALID;
 if(capacity<n+58)return CPX_AUTH_FULL;
 unsigned char stage[CPX_AUTH_WIRE];stage[0]='P';stage[1]='3';stage[2]='A';stage[3]='1';put(stage+4,s->key_id,4);put(stage+8,s->epoch,8);put(stage+16,sequence,8);put(stage+24,n,2);
 for(size_t i=0;i<n;++i)stage[26+i]=payload[i];
 unsigned offset=CPX_MUTANT==7?26:0;
 if(!mac(s->key,32,stage+offset,n+26-offset,stage+26+n))return CPX_AUTH_FAILED;
 for(size_t i=0;i<n+58;++i)out[i]=stage[i];
 *written=n+58;return CPX_AUTH_OK;
}
enum cpx_auth_status cpx_auth_open(struct cpx_auth_state *s,const unsigned char *wire,size_t n,unsigned char *out,size_t capacity,size_t *written,cpx_mac_fn mac){
 if(!s || !wire || !out || !written || !mac || n<58 || n>CPX_AUTH_WIRE)return CPX_AUTH_INVALID;
 if(wire[0]!='P' || wire[1]!='3' || wire[2]!='A' || wire[3]!='1' || get(wire+4,4)!=s->key_id || get(wire+8,8)!=s->epoch)return CPX_AUTH_INVALID;
 size_t length=(size_t)get(wire+24,2);if(length>CPX_AUTH_PAYLOAD || n!=length+58)return CPX_AUTH_INVALID;
 if(capacity<length)return CPX_AUTH_FULL;
 uint64_t sequence=get(wire+16,8);if(s->seen && sequence<=s->last_sequence)return CPX_AUTH_REPLAY;
 if(CPX_MUTANT==8){s->last_sequence=sequence;s->seen=1;}
 unsigned char tag[32];unsigned offset=CPX_MUTANT==7?26:0;
 if(!mac(s->key,32,wire+offset,length+26-offset,tag) || !cpx_tag_equal(tag,wire+26+length))return CPX_AUTH_FAILED;
 for(size_t i=0;i<length;++i)out[i]=wire[26+i];
 s->last_sequence=sequence;s->seen=1;*written=length;return CPX_AUTH_OK;
}
