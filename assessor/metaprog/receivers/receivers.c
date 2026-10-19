#include "receivers.h"
#ifndef MPV_MUTATION
#define MPV_MUTATION 0
#endif
#define MPV_READ(name,type,kind) int name(type *p,MpvRead *out) { uint32_t value;if(!p||!out)return MPV_INVALID;value=*p;out->value=value;out->category=kind;return MPV_OK; }
MPV_READ(mpv_read_m,uint32_t,1)
MPV_READ(mpv_read_c,const uint32_t,2)
MPV_READ(mpv_read_v,volatile uint32_t,3)
MPV_READ(mpv_read_cv,const volatile uint32_t,MPV_MUTATION==2?3:4)
#undef MPV_READ
#define MPV_WRITE(name,type) int name(type *p,uint32_t allowed,uint32_t value) { if(!p)return MPV_INVALID;if(value&~allowed){if(MPV_MUTATION==6)*p=value;return MPV_RANGE;}*p=value;return MPV_OK; }
MPV_WRITE(mpv_write_m,uint32_t)
MPV_WRITE(mpv_write_v,volatile uint32_t)
#undef MPV_WRITE
#define MPV_OBSERVE(name,type) int name(type *p,MpvBetween between,void *context,MpvSample *out) { uint32_t before,after;if(!p||!between||!out)return MPV_INVALID;before=*p;between(context);after=MPV_MUTATION==3?before:*p;out->before=before;out->after=after;return MPV_OK; }
MPV_OBSERVE(mpv_observe_v,volatile uint32_t)
MPV_OBSERVE(mpv_observe_cv,const volatile uint32_t)
#undef MPV_OBSERVE
int mpv_update(MpvRW view,uint32_t clear,uint32_t set) {
    uint32_t old;if(!view.word)return MPV_INVALID;
    if((clear|set)&~view.writable)return MPV_RANGE;
    old=*view.word;*view.word=(old&~(MPV_MUTATION==5?0:clear))|set;return MPV_OK;
}
int mpv_ack(MpvW1C view,uint32_t flags) {
    if(!view.word)return MPV_INVALID;
    if(flags&~view.allowed)return MPV_RANGE;
    *view.word=MPV_MUTATION==4?(*view.word|flags):flags;return MPV_OK;
}
