#ifndef SH_HARDENING_H
#define SH_HARDENING_H
#include <stddef.h>
int sh_copy(const unsigned char *src, size_t count, unsigned *out);
unsigned sh_flawed_copy(const unsigned char *src, size_t count);
unsigned sh_consume(const unsigned char *src, size_t count);
unsigned sh_optimized_away(unsigned x);
int sh_authorize(unsigned claimed_role, int trusted,
                 unsigned *synthetic_counter);
int sh_flawed_authorize(unsigned claimed_role, int trusted,
                        unsigned *synthetic_counter);
#endif
