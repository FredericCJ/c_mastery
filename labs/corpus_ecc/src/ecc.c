#include "ecc.h"
enum ec_status ec_encode(unsigned n,uint16_t data,uint16_t *out){(void)n;(void)data;(void)out;return EC_INVALID;}
enum ec_status ec_decode(unsigned n,uint16_t word,struct ec_result *out){(void)n;(void)word;(void)out;return EC_INVALID;}
enum ec_status ec_erasures(unsigned n,uint16_t word,uint16_t mask,unsigned e,struct ec_result *out){(void)n;(void)word;(void)mask;(void)e;(void)out;return EC_INVALID;}
enum ec_status ec_interleave(unsigned n,const uint16_t words[4],uint8_t bits[64]){(void)n;(void)words;(void)bits;return EC_INVALID;}
enum ec_status ec_deinterleave(unsigned n,const uint8_t bits[64],uint16_t words[4]){(void)n;(void)bits;(void)words;return EC_INVALID;}
enum ec_status ec_interleave_phase(unsigned n,const uint16_t words[4],unsigned p,uint8_t bits[64]){(void)n;(void)words;(void)p;(void)bits;return EC_INVALID;}
enum ec_status ec_deinterleave_phase(unsigned n,const uint8_t bits[64],unsigned p,uint16_t words[4]){(void)n;(void)bits;(void)p;(void)words;return EC_INVALID;}
