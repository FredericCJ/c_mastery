#ifndef TF_EXPRESSIONS_H
#define TF_EXPRESSIONS_H
#include "format.h"
typedef TfStatus (*TfStep)(void *, int *);
typedef TfStatus (*TfGate)(void *);
typedef struct { unsigned completed; int result; } TfProgress;
/* Result publication is atomic; already completed callback effects are not rolled back. */
TfStatus tf_steps(TfStep a, TfStep b, TfStep c, TfGate after_a,
                  void *context, TfProgress *progress);
TfStatus tf_narrow_signed(long long value, unsigned char *out);
TfStatus tf_narrow_size(size_t value, unsigned char *out);
#endif
