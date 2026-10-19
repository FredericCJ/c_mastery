#ifndef CMASTERY_EVOLUTION_V1_H
#define CMASTERY_EVOLUTION_V1_H
#include <stddef.h>
/* Version 1 ABI stays independently installable. Borrow valid during callback. */
typedef void (*AcBorrowed)(void *, const unsigned char *, size_t);
int ac_transform_v1(const unsigned char *, size_t, AcBorrowed, void *, size_t *);
#endif
