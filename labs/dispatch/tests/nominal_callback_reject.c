#include "dispatch_extensions.h"
static long wrong(const void *p,int *out){(void)p;(void)out;return 0;}
const struct ds_ops invalid={wrong};
