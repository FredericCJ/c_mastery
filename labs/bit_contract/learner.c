#include "bits.h"
FbStatus fb_shift(unsigned width,FbWord input,int count,FbShift policy,FbWord *out){(void)width;(void)input;(void)count;(void)policy;(void)out;return FB_UNFINISHED;}
FbStatus fb_extract(unsigned width,FbWord input,unsigned low,unsigned high,FbWord *out){(void)width;(void)input;(void)low;(void)high;(void)out;return FB_UNFINISHED;}
FbStatus fb_insert(unsigned width,FbWord input,FbEdit edit,FbWord *out){(void)width;(void)input;(void)edit;(void)out;return FB_UNFINISHED;}
FbStatus fb_patch(unsigned width,FbWord input,FbEdit first,FbEdit second,FbPatch policy,FbWord *out){(void)width;(void)input;(void)first;(void)second;(void)policy;(void)out;return FB_UNFINISHED;}
FbStatus fb_stats(unsigned width,FbWord input,FbStats *out){(void)width;(void)input;(void)out;return FB_UNFINISHED;}
FbStatus fb_encode(FbWord value,FbOrder order,unsigned char *out,size_t capacity){(void)value;(void)order;(void)out;(void)capacity;return FB_UNFINISHED;}
FbStatus fb_decode(FbSpan input,FbOrder order,FbWord *out){(void)input;(void)order;(void)out;return FB_UNFINISHED;}
FbStatus fb_decode_two(FbSpan first,FbSpan second,FbOrder order,FbWord *out){(void)first;(void)second;(void)order;(void)out;return FB_UNFINISHED;}
FbStatus fb_record(FbSpan input,FbUnknown policy,FbRecord *out){(void)input;(void)policy;(void)out;return FB_UNFINISHED;}
FbStatus fb_negotiate(FbSpan input,FbPlugin *out){(void)input;(void)out;return FB_UNFINISHED;}
FbStatus fb_register_init(FbRegister *reg,unsigned state,unsigned read_clear){(void)reg;(void)state;(void)read_clear;return FB_UNFINISHED;}
unsigned fb_register_read(FbRegister *reg){(void)reg;return 0;}
void fb_register_write(FbRegister *reg,unsigned value){(void)reg;(void)value;}
FbStatus fb_register_update(FbRegister *reg,unsigned low_value,unsigned clear_status){(void)reg;(void)low_value;(void)clear_status;return FB_UNFINISHED;}
