#ifndef CMASTERY_EVOLUTION_V1_H
#define CMASTERY_EVOLUTION_V1_H
#include <stddef.h>
/* Version 1 ABI stays independently installable. Borrow valid during callback. */
typedef void (*ac_borrowed_fn)(void *, const unsigned char *, size_t);
int ac_transform_v1(const unsigned char *, size_t, ac_borrowed_fn, void *,
                    size_t *);
#endif
