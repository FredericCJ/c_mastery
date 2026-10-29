#ifndef CMASTERY_MPV_RECEIVERS_H
#define CMASTERY_MPV_RECEIVERS_H
#include <stdint.h>
enum { MPV_OK,MPV_INVALID,MPV_RANGE };
struct mpv_read { uint32_t value; unsigned category; };
struct mpv_sample { uint32_t before,after; };
typedef void (*mpv_between_fn)(void *);
struct mpv_rw { volatile uint32_t *word; uint32_t writable; };
struct mpv_w1_c { volatile uint32_t *word; uint32_t allowed; };
/* Valid/disjoint live pointers; reads borrow, never retain. Explicit null
 * refusal performs no access. Callers serialize; volatile is not a lock. */
int mpv_read_m(uint32_t *,struct mpv_read *);
int mpv_read_c(const uint32_t *,struct mpv_read *);
int mpv_read_v(volatile uint32_t *,struct mpv_read *);
int mpv_read_cv(const volatile uint32_t *,struct mpv_read *);
int mpv_write_m(uint32_t *,uint32_t,uint32_t);
int mpv_write_v(volatile uint32_t *,uint32_t,uint32_t);
int mpv_observe_v(volatile uint32_t *,mpv_between_fn,void *,struct mpv_sample *);
int mpv_observe_cv(const volatile uint32_t *,mpv_between_fn,void *,struct mpv_sample *);
int mpv_update(struct mpv_rw,uint32_t,uint32_t);
int mpv_ack(struct mpv_w1_c,uint32_t);
#endif
