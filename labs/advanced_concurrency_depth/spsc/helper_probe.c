#include "spsc.h"

/* The changed payload contains 64 words. Inspect generated code: aggregate
 * copying may introduce helpers; neither a helper nor its absence is universal.
 * No runtime provider is supplied in this strict dependency diagnostic. */
struct as_large_value
{
    uint32_t words[64];
};

void as_implicit_copy(struct as_large_value *out,
                      const struct as_large_value *in, as_hook_fn hook,
                      void *context)
{
    struct as_large_value value = *in;
    if (hook)
        hook(context);
    *out = value;
}
