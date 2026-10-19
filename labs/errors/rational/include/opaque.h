#ifndef CMASTERY_RATIONAL_OPAQUE_H
#define CMASTERY_RATIONAL_OPAQUE_H
#include "errors.h"
typedef struct EfrOpaque EfrOpaque;
/* Hosted comparison only: raw suitably aligned distinct storage, not the T66
   typed-cell API. Callback/context lives until drop, no reentry/nonlocal exit. */
typedef struct {void *context;void *(*take)(void *,size_t);void (*release)(void *,void *);} EfrMemory;
er_code efr_opaque_make(EfrMemory,uint32_t,uint32_t,EfrOpaque **empty_owner);
er_code efr_opaque_consume(const EfrOpaque *,uint32_t *);
void efr_opaque_drop(EfrOpaque **owner);
#endif
