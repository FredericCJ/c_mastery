#include "connection.h"
enum dc_status dc_init(struct dc_session*s,const struct dc_manifest*m){(void)s;(void)m;return DC_UNIMPLEMENTED;}
enum dc_status dc_identify(struct dc_session*s,unsigned t){(void)s;(void)t;return DC_UNIMPLEMENTED;}
enum dc_status dc_halt(struct dc_session*s,unsigned v){(void)s;(void)v;return DC_UNIMPLEMENTED;}
enum dc_status dc_reset(struct dc_session*s,unsigned v){(void)s;(void)v;return DC_UNIMPLEMENTED;}
enum dc_status dc_program_authorized(const struct dc_session*s){(void)s;return DC_UNIMPLEMENTED;}
enum dc_status dc_reconfigure(struct dc_session*s,const struct dc_manifest*m){(void)s;(void)m;return DC_UNIMPLEMENTED;}
enum dc_status dc_classify(unsigned c,unsigned p,unsigned r,unsigned*out){(void)c;(void)p;(void)r;(void)out;return DC_UNIMPLEMENTED;}
enum dc_status dc_vector(uint32_t b,uint32_t n,uint32_t v,unsigned*out){(void)b;(void)n;(void)v;(void)out;return DC_UNIMPLEMENTED;}
