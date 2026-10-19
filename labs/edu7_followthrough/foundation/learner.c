#include "foundation.h"
int ef_budget(unsigned a,unsigned b,EfBudget *c){(void)a;(void)b;(void)c;return EF_STUB;}
int ef_enqueue(EfRoute *a,const uint8_t *b,size_t c,unsigned d,unsigned e,unsigned f,uint32_t *g){(void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;return EF_STUB;}
int ef_peek(const EfRoute *a,uint32_t b,EduFrame *c){(void)a;(void)b;(void)c;return EF_STUB;}
int ef_release(EfRoute *a,uint32_t b){(void)a;(void)b;return EF_STUB;}
int ef_local(const uint8_t *a,size_t b,unsigned c,EduDatagram *d){(void)a;(void)b;(void)c;(void)d;return EF_STUB;}
int ef_encode(const EduRecord *a,uint8_t *b,size_t c,size_t *d){(void)a;(void)b;(void)c;(void)d;return EF_STUB;}
int ef_wide_encode(const EfWide *a,uint8_t *b,size_t c,size_t *d){(void)a;(void)b;(void)c;(void)d;return EF_STUB;}
int ef_wide_decode(const uint8_t *a,size_t b,EfWide *c){(void)a;(void)b;(void)c;return EF_STUB;}
int ef_frame_encode(const EduFrame *a,unsigned b,uint8_t *c,size_t d,size_t *e){(void)a;(void)b;(void)c;(void)d;(void)e;return EF_STUB;}
int ef_frame_byte(EfFramer *a,unsigned b,uint8_t c,unsigned d,EduFrame *e){(void)a;(void)b;(void)c;(void)d;(void)e;return EF_STUB;}
