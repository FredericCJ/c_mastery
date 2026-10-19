#ifndef CMASTERY_MPV_MACROS_H
#define CMASTERY_MPV_MACROS_H
#include "receivers.h"
#ifndef MPV_MUTATION
#define MPV_MUTATION 0
#endif
#define MPV_CHOOSE(p) _Generic((p),uint32_t*:mpv_read_m,const uint32_t*:mpv_read_c,volatile uint32_t*:mpv_read_v,const volatile uint32_t*:mpv_read_cv)
#if MPV_MUTATION==1
#define MPV_READ(p,out) ((void)(p),MPV_CHOOSE(p)((p),(out)))
#else
#define MPV_READ(p,out) MPV_CHOOSE(p)((p),(out))
#endif
#define MPV_WRITE(p,allowed,value) _Generic((p),uint32_t*:mpv_write_m,volatile uint32_t*:mpv_write_v)((p),(allowed),(value))
#define MPV_OBSERVE(p,between,context,out) _Generic((p),volatile uint32_t*:mpv_observe_v,const volatile uint32_t*:mpv_observe_cv)((p),(between),(context),(out))
#endif
