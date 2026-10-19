#include "format.h"
#include <stdio.h>
int tf_default_integer(void *context, char *out, size_t capacity, int value) {
  (void)context; return snprintf(out, capacity, "I=%d;", value);
}
int tf_default_real(void *context, char *out, size_t capacity, double value) {
  (void)context; return snprintf(out, capacity, "D=%.2f;", value);
}
