#include "spsc.h"
/* The changed payload contains 64 words. Inspect generated code: aggregate
 * copying may introduce helpers; neither a helper nor its absence is universal.
 * No runtime provider is supplied in this strict dependency diagnostic. */
typedef struct {
  uint32_t words[64];
} ASLargeValue;
void as_implicit_copy(ASLargeValue *out, const ASLargeValue *in, ASHook hook,
                      void *context) {
  ASLargeValue value = *in;
  if (hook)
    hook(context);
  *out = value;
}
