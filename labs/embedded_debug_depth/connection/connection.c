#include "connection.h"
enum dc_status dc_init(DCSession*s,const DCManifest*m){(void)s;(void)m;return DC_UNIMPLEMENTED;}
enum dc_status dc_identify(DCSession*s,unsigned t){(void)s;(void)t;return DC_UNIMPLEMENTED;}
enum dc_status dc_halt(DCSession*s,unsigned v){(void)s;(void)v;return DC_UNIMPLEMENTED;}
enum dc_status dc_reset(DCSession*s,unsigned v){(void)s;(void)v;return DC_UNIMPLEMENTED;}
enum dc_status dc_program_authorized(const DCSession*s){(void)s;return DC_UNIMPLEMENTED;}
enum dc_status dc_reconfigure(DCSession*s,const DCManifest*m){(void)s;(void)m;return DC_UNIMPLEMENTED;}
enum dc_status dc_classify(unsigned c,unsigned p,unsigned r,unsigned*out){(void)c;(void)p;(void)r;(void)out;return DC_UNIMPLEMENTED;}
enum dc_status dc_vector(uint32_t b,uint32_t n,uint32_t v,unsigned*out){(void)b;(void)n;(void)v;(void)out;return DC_UNIMPLEMENTED;}
