#ifndef LC_SUPPORT_H
#define LC_SUPPORT_H
#include <stddef.h>
typedef struct {
  int (*acquire)(void *, unsigned);
  void (*release)(void *, unsigned);
  void *context;
} LCOps;
typedef struct {
  unsigned mask;
  LCOps ops;
} LCPair;
int lc_construct(LCPair *, LCOps, int optional);
int lc_construct_nested(LCPair *, LCOps, int optional);
int lc_construct_helpers(LCPair *, LCOps, int optional);
void lc_destroy(LCPair *);
typedef struct {
  unsigned calls;
  int bias;
} LCBackend;
int lc_host_backend(LCBackend *, int);
int lc_board_backend(LCBackend *, int);
typedef struct {
  int (*apply)(LCBackend *, int);
  LCBackend *context;
} LCBackendOps;
int lc_backend_run(LCBackendOps, int);
#endif
