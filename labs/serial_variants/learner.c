#include "variants.h"
int sv_baud(uint32_t clock,uint32_t baud,uint32_t ppm,SvBaud *out){(void)clock;(void)baud;(void)ppm;(void)out;return SV_UNIMPLEMENTED;}
int sv_word(unsigned m,unsigned parity,unsigned stop,unsigned p,unsigned h,unsigned l,uint16_t value,SvWord *out){(void)m;(void)parity;(void)stop;(void)p;(void)h;(void)l;(void)value;(void)out;return SV_UNIMPLEMENTED;}
int sv_capture(unsigned p,unsigned h,uint32_t period,const SvEdge *edges,size_t n,SvCapture *out){(void)p;(void)h;(void)period;(void)edges;(void)n;(void)out;return SV_UNIMPLEMENTED;}
int sv_receiver(int32_t a,int32_t b,unsigned enabled,unsigned open,unsigned characterized,int low,int high,unsigned prior,unsigned *out){(void)a;(void)b;(void)enabled;(void)open;(void)characterized;(void)low;(void)high;(void)prior;(void)out;return SV_UNIMPLEMENTED;}
int sv_stream_reset(SvStream *s,uint32_t g){(void)s;(void)g;return SV_UNIMPLEMENTED;}
int sv_stream_feed(SvStream *s,uint32_t g,const uint8_t *p,size_t n,size_t *used,CnDecoded *out){(void)s;(void)g;(void)p;(void)n;(void)used;(void)out;return SV_UNIMPLEMENTED;}
int sv_policy(unsigned ext,unsigned tx,unsigned srr,unsigned r1,unsigned r0,unsigned eof,SvPolicy *out){(void)ext;(void)tx;(void)srr;(void)r1;(void)r0;(void)eof;(void)out;return SV_UNIMPLEMENTED;}
int sv_bank(unsigned list,const SDCan *a,const SDCan *b,uint32_t mask,SvBank *out){(void)list;(void)a;(void)b;(void)mask;(void)out;return SV_UNIMPLEMENTED;}
int sv_filter(const SvBank *bank,const SDCan *frame,unsigned *out){(void)bank;(void)frame;(void)out;return SV_UNIMPLEMENTED;}
