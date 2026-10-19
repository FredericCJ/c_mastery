#ifndef TF_FORMAT_H
#define TF_FORMAT_H
#include <stddef.h>
typedef enum { TF_OK, TF_FULL, TF_INVALID, TF_NOMEM, TF_FORMAT, TF_SIZE, TF_UNFINISHED } TfStatus;
typedef struct {
  void *context;
  char *(*acquire)(void *, size_t);
  void (*release)(void *, char *);
} TfAllocator;
/* Each callback has snprintf's scalar sizing/truncation return contract. */
typedef struct {
  void *context;
  int (*integer)(void *, char *, size_t, int);
  int (*real)(void *, char *, size_t, double);
} TfScalars;
/* Explicit fields avoid an uncheckable active-union-member claim. */
typedef struct { int tag, integer; double real; } TfArg;
TfStatus tf_format(char *destination, size_t capacity, size_t *required,
                   TfAllocator allocator, TfScalars scalar, unsigned count, ...);
TfStatus tf_array(char *destination, size_t capacity, size_t *required,
                  TfAllocator allocator, TfScalars scalar, const TfArg *args,
                  size_t available, unsigned count);
int tf_default_integer(void *, char *, size_t, int);
int tf_default_real(void *, char *, size_t, double);
#endif
