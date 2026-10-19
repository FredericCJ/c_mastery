#include "format.h"
#include <stdarg.h>
#include <stdint.h>
#ifndef TF_MUTANT
#define TF_MUTANT 0
#endif
typedef struct {
  va_list *list;
  const TfArg *array;
  unsigned position;
} Reader;
static TfStatus next(Reader *r, TfArg *out) {
  out->tag = r->list ? va_arg(*r->list, int) : r->array[r->position].tag;
  if (out->tag == 'I')
    out->integer = r->list ? va_arg(*r->list, int) : r->array[r->position].integer;
  else if (out->tag == 'D')
    out->real = r->list ? va_arg(*r->list, double) : r->array[r->position].real;
  else
    return TF_INVALID; /* No unknown payload is consumed. */
  ++r->position;
  return TF_OK;
}
static int convert(TfScalars s, TfArg arg, char *out, size_t capacity) {
  return arg.tag == 'I' ? s.integer(s.context, out, capacity, arg.integer)
                        : s.real(s.context, out, capacity, arg.real);
}
static TfStatus sizing(Reader *r, unsigned count, TfScalars scalar,
                        size_t sizes[4], size_t *total) {
  size_t sum = 0;
  for (unsigned i = 0; i < count; ++i) {
    TfArg arg = {0};
    TfStatus status = next(r, &arg);
    if (status != TF_OK) return status;
    int n = convert(scalar, arg, NULL, 0);
    if (n < 0) return TF_FORMAT;
    /* Reserve the final NUL without wrapping size_t. */
    if ((uintmax_t)(unsigned)n > SIZE_MAX - sum - 1) return TF_SIZE;
    sizes[i] = (size_t)n; sum += (size_t)n;
  }
  *total = sum; return TF_OK;
}
static TfStatus size_variadic(va_list *source, unsigned count, TfScalars scalar,
                               size_t sizes[4], size_t *total) {
  va_list copy; va_copy(copy, *source);
  Reader r = {&copy, NULL, 0};
  TfStatus status = sizing(&r, count, scalar, sizes, total);
  va_end(copy); return status;
}
static TfStatus render(Reader *r, unsigned count, TfScalars scalar,
                        const size_t sizes[4], char *stage, size_t total) {
  size_t at = 0; stage[0] = 0;
  for (unsigned i = 0; i < count; ++i) {
    TfArg arg = {0};
    TfStatus status = next(r, &arg);
    if (status != TF_OK) return status;
    int n = convert(scalar, arg, stage + at, total + 1 - at);
    if (n < 0 || (uintmax_t)(unsigned)n != sizes[i]) return TF_FORMAT;
    at += sizes[i];
  }
  return at == total ? TF_OK : TF_FORMAT;
}
static TfStatus render_variadic(va_list *source, unsigned count, TfScalars scalar,
                                 const size_t sizes[4], char *stage, size_t total) {
  va_list copy; va_copy(copy, *source);
  Reader r = {&copy, NULL, 0};
  TfStatus status = render(&r, count, scalar, sizes, stage, total);
  va_end(copy); return status;
}
static int admitted(char *out, size_t capacity, size_t *required, TfAllocator a,
                     TfScalars s, unsigned count) {
  return required && (out || capacity == 0) && a.acquire && a.release &&
         s.integer && s.real && count <= 4;
}
static TfStatus finish(char *out, size_t *required, TfAllocator a, char *stage,
                        size_t total, TfStatus status) {
  if (status == TF_OK) {
    for (size_t i = 0; i <= total; ++i) out[i] = stage[i];
    *required = TF_MUTANT == 1 ? total + 1 : total;
  }
  if (TF_MUTANT != 2) a.release(a.context, stage);
  return status;
}
TfStatus tf_format(char *out, size_t capacity, size_t *required, TfAllocator a,
                     TfScalars scalar, unsigned count, ...) {
  if (!admitted(out, capacity, required, a, scalar, count)) return TF_INVALID;
  va_list source; va_start(source, count);
  size_t sizes[4], total = 0;
  TfStatus status = size_variadic(&source, count, scalar, sizes, &total);
  if (status != TF_OK) { va_end(source); return status; }
  if (capacity <= total) {
    *required = total;
    if (TF_MUTANT == 3 && out && capacity) out[0] = 0;
    va_end(source); return TF_FULL;
  }
  char *stage = a.acquire(a.context, total + 1);
  if (!stage) { va_end(source); return TF_NOMEM; }
  status = render_variadic(&source, count, scalar, sizes, stage, total);
  va_end(source);
  return finish(out, required, a, stage, total, status);
}
TfStatus tf_array(char *out, size_t capacity, size_t *required, TfAllocator a,
                    TfScalars scalar, const TfArg *args, size_t available,
                    unsigned count) {
  if (!admitted(out, capacity, required, a, scalar, count) ||
      count > available || (count && !args)) return TF_INVALID;
  Reader first = {NULL, args, 0}; size_t sizes[4], total = 0;
  TfStatus status = sizing(&first, count, scalar, sizes, &total);
  if (status != TF_OK) return status;
  if (capacity <= total) { *required = total; return TF_FULL; }
  char *stage = a.acquire(a.context, total + 1);
  if (!stage) return TF_NOMEM;
  Reader second = {NULL, args, 0};
  status = render(&second, count, scalar, sizes, stage, total);
  return finish(out, required, a, stage, total, status);
}
