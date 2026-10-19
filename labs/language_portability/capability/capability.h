#ifndef LP_CAPABILITY_H
#define LP_CAPABILITY_H
#include <stddef.h>
#include <stdint.h>
#ifndef LP_MUTANT
#define LP_MUTANT 0
#endif
typedef struct {
#ifdef LP_LAYOUT2
    unsigned present;
    intmax_t value;
#else
    intmax_t value;
    unsigned present;
#endif
} LpValue;
/* All live input/output spans are disjoint; output is unchanged on refusal.
   Caller owns record storage. No pointers to automatic compound literals escape. */
int lp_record(unsigned present, intmax_t value, LpValue *out);
int lp_format(LpValue value, char *out, size_t capacity);
/* n <= 16, full int range, checked doubling; NULL spans are legal only at n=0. */
int lp_transform(const int *restrict source, int *restrict destination, size_t n);
/* Four bits: language=1, header=2, link=4, run=8. Invalid masks refuse. */
int lp_capability(unsigned required, unsigned observed, unsigned *usable);
#if LP_MUTANT == 3
#define LP_TRACE(function, ...) ((function)(__VA_ARGS__), (function)(__VA_ARGS__))
#else
#define LP_TRACE(function, ...) ((function)(__VA_ARGS__))
#endif
#endif
