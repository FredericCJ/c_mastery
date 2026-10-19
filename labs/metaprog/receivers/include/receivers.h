#ifndef CMASTERY_MPV_RECEIVERS_H
#define CMASTERY_MPV_RECEIVERS_H
#include <stdint.h>
enum { MPV_OK,MPV_INVALID,MPV_RANGE };
typedef struct { uint32_t value; unsigned category; } MpvRead;
typedef struct { uint32_t before,after; } MpvSample;
typedef void (*MpvBetween)(void *);
typedef struct { volatile uint32_t *word; uint32_t writable; } MpvRW;
typedef struct { volatile uint32_t *word; uint32_t allowed; } MpvW1C;
/* Valid/disjoint live pointers; reads borrow, never retain. Explicit null
 * refusal performs no access. Callers serialize; volatile is not a lock. */
int mpv_read_m(uint32_t *,MpvRead *);
int mpv_read_c(const uint32_t *,MpvRead *);
int mpv_read_v(volatile uint32_t *,MpvRead *);
int mpv_read_cv(const volatile uint32_t *,MpvRead *);
int mpv_write_m(uint32_t *,uint32_t,uint32_t);
int mpv_write_v(volatile uint32_t *,uint32_t,uint32_t);
int mpv_observe_v(volatile uint32_t *,MpvBetween,void *,MpvSample *);
int mpv_observe_cv(const volatile uint32_t *,MpvBetween,void *,MpvSample *);
int mpv_update(MpvRW,uint32_t,uint32_t);
int mpv_ack(MpvW1C,uint32_t);
#endif
