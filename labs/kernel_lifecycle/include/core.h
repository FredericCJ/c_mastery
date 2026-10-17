#ifndef B06_KL_H
#define B06_KL_H
#include <stddef.h>
#include <stdint.h>
#include <limits.h>
#include <math.h>
typedef struct { unsigned refs, dead; } KLRef;
typedef struct { unsigned queued, running, closed, released; } KLWork;
typedef struct { unsigned readers, retired; } KLGrace;
unsigned kl_unwind(unsigned mask);
int kl_copy(unsigned char *dst, size_t cap, const unsigned char *src, size_t n, size_t fault_after);
int kl_ref(KLRef *r, unsigned op);
int kl_lock_allowed(unsigned context, unsigned kind, unsigned rt);
int kl_work(KLWork *w, unsigned event);
int kl_grace(KLGrace *g, unsigned event, unsigned id);
int kl_decode(const unsigned char *p, size_t n, unsigned *out);
int kl_can_unload(unsigned handles, unsigned callbacks, unsigned closed);
#endif
