#include "format.h"
#include "expressions.h"
enum tf_status tf_format(char *out,size_t capacity,size_t *required,struct tf_allocator a,
                    struct tf_scalars s,unsigned count,...) {
 (void)out;(void)capacity;(void)required;(void)a;(void)s;(void)count;
 return TF_UNFINISHED;
}
enum tf_status tf_array(char *out,size_t capacity,size_t *required,struct tf_allocator a,
                   struct tf_scalars s,const struct tf_arg *args,size_t available,unsigned count) {
 (void)out;(void)capacity;(void)required;(void)a;(void)s;(void)args;(void)available;(void)count;
 return TF_UNFINISHED;
}
enum tf_status tf_steps(tf_step_fn a,tf_step_fn b,tf_step_fn c,tf_gate_fn gate,void *ctx,struct tf_progress *p) {
 (void)a;(void)b;(void)c;(void)gate;(void)ctx;(void)p;return TF_UNFINISHED;
}
enum tf_status tf_narrow_signed(long long v,unsigned char *p) {
 (void)v;(void)p;return TF_UNFINISHED;
}
enum tf_status tf_narrow_size(size_t v,unsigned char *p) {
 (void)v;(void)p;return TF_UNFINISHED;
}
