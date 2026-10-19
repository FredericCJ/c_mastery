#include "bytes.h"
enum ABStatus ab_write(uint64_t a,unsigned b,uint32_t c,uint64_t*d){(void)a;(void)b;(void)c;(void)d;return AB_UNSUPPORTED;}
enum ABStatus ab_word(const unsigned char*a,size_t b,unsigned c,uint32_t*d){(void)a;(void)b;(void)c;(void)d;return AB_UNSUPPORTED;}
enum ABStatus ab_address(uint64_t a,uint64_t b,unsigned c,int64_t d,unsigned e,uint64_t f,uint64_t*g,uint64_t*h){(void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;return AB_UNSUPPORTED;}
enum ABStatus ab_record_decode(const unsigned char*a,size_t b,unsigned c,AWRecord*d){(void)a;(void)b;(void)c;(void)d;return AB_UNSUPPORTED;}
enum ABStatus ab_record_encode(const AWRecord*a,unsigned b,unsigned char*c,size_t d){(void)a;(void)b;(void)c;(void)d;return AB_UNSUPPORTED;}
enum ABStatus ab_record_copy(AWRecord*a,const AWRecord*b){(void)a;(void)b;return AB_UNSUPPORTED;}
unsigned ab_thumb_length(uint16_t a){(void)a;return 0;}
enum ABStatus ab_x86(const unsigned char*a,size_t b,uint64_t c,unsigned d,ABInstruction*e){(void)a;(void)b;(void)c;(void)d;(void)e;return AB_UNSUPPORTED;}
enum ABStatus ab_thumb(const unsigned char*a,size_t b,uint32_t c,unsigned d,ABInstruction*e){(void)a;(void)b;(void)c;(void)d;(void)e;return AB_UNSUPPORTED;}
