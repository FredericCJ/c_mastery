#ifndef B06_KL_H
#define B06_KL_H
#include <stddef.h>
#include <stdint.h>
#include <limits.h>
#include <math.h>
struct kl_ref { unsigned refs, dead; };
struct kl_work { unsigned queued, running, closed, released; };
struct kl_grace { unsigned readers, retired; };
unsigned kl_unwind(unsigned mask);
int kl_copy(unsigned char *dst, size_t cap, const unsigned char *src, size_t n, size_t fault_after);
int kl_ref(struct kl_ref *r, unsigned op);
int kl_lock_allowed(unsigned context, unsigned kind, unsigned rt);
int kl_work(struct kl_work *w, unsigned event);
int kl_grace(struct kl_grace *g, unsigned event, unsigned id);
int kl_decode(const unsigned char *p, size_t n, unsigned *out);
int kl_can_unload(unsigned handles, unsigned callbacks, unsigned closed);
#endif
