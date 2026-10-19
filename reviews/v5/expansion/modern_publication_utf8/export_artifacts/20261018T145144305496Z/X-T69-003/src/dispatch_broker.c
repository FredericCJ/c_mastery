#include "dispatch_broker.h"
void ds_broker_init(DSBroker *b){(void)b;}
int ds_broker_revoke(DSBroker *b,unsigned t){(void)b;(void)t;return DS_FAILED;}
int ds_broker_apply(DSBroker *b,unsigned h,const unsigned char p[16],unsigned char o[16]){(void)b;(void)h;(void)p;(void)o;return DS_FAILED;}
void ds_broker_request(unsigned char p[16],unsigned a,unsigned b,unsigned c,uint32_t d,unsigned e,unsigned f,uint32_t g){(void)p;(void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;}
uint32_t ds_broker_u32(const unsigned char p[4]){(void)p;return 0;}
