#include "foundation.h"
int ef_budget(unsigned a,unsigned b,struct ef_budget *c){(void)a;(void)b;(void)c;return EF_STUB;}
int ef_enqueue(struct ef_route *a,const uint8_t *b,size_t c,unsigned d,unsigned e,unsigned f,uint32_t *g){(void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;return EF_STUB;}
int ef_peek(const struct ef_route *a,uint32_t b,struct edu_frame *c){(void)a;(void)b;(void)c;return EF_STUB;}
int ef_release(struct ef_route *a,uint32_t b){(void)a;(void)b;return EF_STUB;}
int ef_local(const uint8_t *a,size_t b,unsigned c,struct edu_datagram *d){(void)a;(void)b;(void)c;(void)d;return EF_STUB;}
int ef_encode(const struct edu_record *a,uint8_t *b,size_t c,size_t *d){(void)a;(void)b;(void)c;(void)d;return EF_STUB;}
int ef_wide_encode(const struct ef_wide *a,uint8_t *b,size_t c,size_t *d){(void)a;(void)b;(void)c;(void)d;return EF_STUB;}
int ef_wide_decode(const uint8_t *a,size_t b,struct ef_wide *c){(void)a;(void)b;(void)c;return EF_STUB;}
int ef_frame_encode(const struct edu_frame *a,unsigned b,uint8_t *c,size_t d,size_t *e){(void)a;(void)b;(void)c;(void)d;(void)e;return EF_STUB;}
int ef_frame_byte(struct ef_framer *a,unsigned b,uint8_t c,unsigned d,struct edu_frame *e){(void)a;(void)b;(void)c;(void)d;(void)e;return EF_STUB;}
