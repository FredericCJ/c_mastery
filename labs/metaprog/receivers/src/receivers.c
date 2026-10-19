#include "receivers.h"
#define READ(name,type) int name(type *p,MpvRead *out){(void)p;(void)out;return MPV_INVALID;}
READ(mpv_read_m,uint32_t)
READ(mpv_read_c,const uint32_t)
READ(mpv_read_v,volatile uint32_t)
READ(mpv_read_cv,const volatile uint32_t)
#undef READ
int mpv_write_m(uint32_t *p,uint32_t m,uint32_t v){(void)p;(void)m;(void)v;return MPV_INVALID;}
int mpv_write_v(volatile uint32_t *p,uint32_t m,uint32_t v){(void)p;(void)m;(void)v;return MPV_INVALID;}
int mpv_observe_v(volatile uint32_t *p,MpvBetween b,void *c,MpvSample *o){(void)p;(void)b;(void)c;(void)o;return MPV_INVALID;}
int mpv_observe_cv(const volatile uint32_t *p,MpvBetween b,void *c,MpvSample *o){(void)p;(void)b;(void)c;(void)o;return MPV_INVALID;}
int mpv_update(MpvRW v,uint32_t c,uint32_t s){(void)v;(void)c;(void)s;return MPV_INVALID;}
int mpv_ack(MpvW1C v,uint32_t f){(void)v;(void)f;return MPV_INVALID;}
