#include "variants.h"
int sv_baud(uint32_t clock,uint32_t baud,uint32_t ppm,struct sv_baud *out){(void)clock;(void)baud;(void)ppm;(void)out;return SV_UNIMPLEMENTED;}
int sv_word(unsigned m,unsigned parity,unsigned stop,unsigned p,unsigned h,unsigned l,uint16_t value,struct sv_word *out){(void)m;(void)parity;(void)stop;(void)p;(void)h;(void)l;(void)value;(void)out;return SV_UNIMPLEMENTED;}
int sv_capture(unsigned p,unsigned h,uint32_t period,const struct sv_edge *edges,size_t n,struct sv_capture *out){(void)p;(void)h;(void)period;(void)edges;(void)n;(void)out;return SV_UNIMPLEMENTED;}
int sv_receiver(int32_t a,int32_t b,unsigned enabled,unsigned open,unsigned characterized,int low,int high,unsigned prior,unsigned *out){(void)a;(void)b;(void)enabled;(void)open;(void)characterized;(void)low;(void)high;(void)prior;(void)out;return SV_UNIMPLEMENTED;}
int sv_stream_reset(struct sv_stream *s,uint32_t g){(void)s;(void)g;return SV_UNIMPLEMENTED;}
int sv_stream_feed(struct sv_stream *s,uint32_t g,const uint8_t *p,size_t n,size_t *used,struct cn_decoded *out){(void)s;(void)g;(void)p;(void)n;(void)used;(void)out;return SV_UNIMPLEMENTED;}
int sv_policy(unsigned ext,unsigned tx,unsigned srr,unsigned r1,unsigned r0,unsigned eof,struct sv_policy *out){(void)ext;(void)tx;(void)srr;(void)r1;(void)r0;(void)eof;(void)out;return SV_UNIMPLEMENTED;}
int sv_bank(unsigned list,const struct sd_can *a,const struct sd_can *b,uint32_t mask,struct sv_bank *out){(void)list;(void)a;(void)b;(void)mask;(void)out;return SV_UNIMPLEMENTED;}
int sv_filter(const struct sv_bank *bank,const struct sd_can *frame,unsigned *out){(void)bank;(void)frame;(void)out;return SV_UNIMPLEMENTED;}
