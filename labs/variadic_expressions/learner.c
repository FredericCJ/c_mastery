#include "format.h"
#include "expressions.h"
TfStatus tf_format(char *out,size_t capacity,size_t *required,TfAllocator a,
                    TfScalars s,unsigned count,...) {
 (void)out;(void)capacity;(void)required;(void)a;(void)s;(void)count;
 return TF_UNFINISHED;
}
TfStatus tf_array(char *out,size_t capacity,size_t *required,TfAllocator a,
                   TfScalars s,const TfArg *args,size_t available,unsigned count) {
 (void)out;(void)capacity;(void)required;(void)a;(void)s;(void)args;(void)available;(void)count;
 return TF_UNFINISHED;
}
TfStatus tf_steps(TfStep a,TfStep b,TfStep c,TfGate gate,void *ctx,TfProgress *p) {
 (void)a;(void)b;(void)c;(void)gate;(void)ctx;(void)p;return TF_UNFINISHED;
}
TfStatus tf_narrow_signed(long long v,unsigned char *p) {
 (void)v;(void)p;return TF_UNFINISHED;
}
TfStatus tf_narrow_size(size_t v,unsigned char *p) {
 (void)v;(void)p;return TF_UNFINISHED;
}
