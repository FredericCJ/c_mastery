#include "async_follow.h"
int ca_create(COAllocator a,unsigned b,uint32_t c,CAFrame**d){(void)a;(void)b;(void)c;(void)d;return CO_UNIMPLEMENTED;}
int ca_reserve(CAFrame*a,CATicket*b){(void)a;(void)b;return CO_UNIMPLEMENTED;}
int ca_notify(CATicket*a,const unsigned char*b,size_t c){(void)a;(void)b;(void)c;return CO_UNIMPLEMENTED;}
int ca_cancel(CAFrame*a){(void)a;return CO_UNIMPLEMENTED;}
int ca_ack(CAFrame*a){(void)a;return CO_UNIMPLEMENTED;}
int ca_poll(CAFrame*a,unsigned char*b,size_t*c){(void)a;(void)b;(void)c;return CO_UNIMPLEMENTED;}
int ca_destroy(CAFrame**a){(void)a;return CO_UNIMPLEMENTED;}
