#include "ownership.h"
/* Explicit comparative translation; the retained provider is never edited. */
#define memory_order_release memory_order_acq_rel
#define ORC_STRING_INNER(x) #x
#define ORC_STRING(x) ORC_STRING_INNER(x)
#ifndef ORC_PROVIDER
#error supply the exact selected ownership implementation
#endif
#include ORC_STRING(ORC_PROVIDER)
