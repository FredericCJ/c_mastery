#include "systems.h"
int s_pipeline(const SInstruction*p,size_t n,unsigned f,unsigned o[16],unsigned*c){(void)p;(void)n;(void)f;(void)o;(void)c;return S_UNIMPLEMENTED;}
int s_i2c_drive(const SI2c*s,unsigned*r){(void)s;(void)r;return S_UNIMPLEMENTED;}
int s_i2c_sample(SI2c*s,unsigned c,unsigned d){(void)s;(void)c;(void)d;return S_UNIMPLEMENTED;}
int s_checksum_add(SChecksum*s,const unsigned char*p,size_t n){(void)s;(void)p;(void)n;return S_UNIMPLEMENTED;}
int s_checksum_finish(const SChecksum*s,uint16_t*r){(void)s;(void)r;return S_UNIMPLEMENTED;}
int s_paired(const uint32_t*a,const uint32_t*b,size_t n,int64_t*m,unsigned*w){(void)a;(void)b;(void)n;(void)m;(void)w;return S_UNIMPLEMENTED;}
int s_reduce(const unsigned char*p,size_t n,SClassify f,void*c,unsigned b,unsigned char o[32],size_t*l,unsigned*k){(void)p;(void)n;(void)f;(void)c;(void)b;(void)o;(void)l;(void)k;return S_UNIMPLEMENTED;}
int s_replace(const unsigned char*p,size_t n,unsigned f,unsigned policy,SReplacement*r){(void)p;(void)n;(void)f;(void)policy;(void)r;return S_UNIMPLEMENTED;}
int s_wait_event(SWait*s,unsigned a,unsigned o){(void)s;(void)a;(void)o;return S_UNIMPLEMENTED;}
int s_shoot_begin(SShootdown*s,unsigned t){(void)s;(void)t;return S_UNIMPLEMENTED;}
int s_shoot_ack(SShootdown*s,unsigned c,uint32_t g){(void)s;(void)c;(void)g;return S_UNIMPLEMENTED;}
int s_shoot_reclaim(SShootdown*s){(void)s;return S_UNIMPLEMENTED;}
int s_peer_submit(SPeer*s,uint32_t*e,uint32_t*q){(void)s;(void)e;(void)q;return S_UNIMPLEMENTED;}
int s_peer_ack(SPeer*s,uint32_t e,uint32_t q){(void)s;(void)e;(void)q;return S_UNIMPLEMENTED;}
int s_peer_restart(SPeer*s,unsigned q){(void)s;(void)q;return S_UNIMPLEMENTED;}
int s_peer_ready(SPeer*s,uint32_t e){(void)s;(void)e;return S_UNIMPLEMENTED;}
