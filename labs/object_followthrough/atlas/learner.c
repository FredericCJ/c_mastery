#include "atlas.h"
int oa_resize(OaOwner *a,size_t b,const OaAllocator *c){(void)a;(void)b;(void)c;return -1;}
void oa_release(OaOwner *a,const OaAllocator *b){(void)a;(void)b;}
int oa_label(char *a,size_t b,OaView *c){(void)a;(void)b;(void)c;return -1;}
int oa_deep_copy(OaView a,char *b,size_t c,OaView *d){(void)a;(void)b;(void)c;(void)d;return -1;}
int oa_equal(const OaRecord *a,const OaRecord *b){(void)a;(void)b;return 0;}
int oa_encode(const OaRecord *a,unsigned char *b,size_t c){(void)a;(void)b;(void)c;return -1;}
int oa_place(size_t a,size_t b,size_t c,size_t d,size_t e,size_t *f){(void)a;(void)b;(void)c;(void)d;(void)e;(void)f;return -1;}
int oa_slot(OaStorage *a,size_t b,size_t c,OaRecord **d){(void)a;(void)b;(void)c;(void)d;return -1;}
int oa_wide_admit(size_t a){(void)a;return 0;}
int oa_decode_u32(const unsigned char *a,size_t b,uint32_t *c){(void)a;(void)b;(void)c;return -1;}
