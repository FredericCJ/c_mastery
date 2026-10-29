#ifndef TF_EXPRESSIONS_H
#define TF_EXPRESSIONS_H
#include "format.h"
typedef enum tf_status (*tf_step_fn)(void *, int *);
typedef enum tf_status (*tf_gate_fn)(void *);
struct tf_progress { unsigned completed; int result; };
/* Result publication is atomic; already completed callback effects are not rolled back. */
enum tf_status tf_steps(tf_step_fn a, tf_step_fn b, tf_step_fn c, tf_gate_fn after_a,
                  void *context, struct tf_progress *progress);
enum tf_status tf_narrow_signed(long long value, unsigned char *out);
enum tf_status tf_narrow_size(size_t value, unsigned char *out);
#endif
