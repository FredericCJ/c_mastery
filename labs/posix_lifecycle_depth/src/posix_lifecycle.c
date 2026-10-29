#include "posix_lifecycle.h"
int pl_launch(const char *p, char *const a[], unsigned f, struct pl_launch *o) {
  (void)p; (void)a; (void)f; (void)o; return -1;
}
int pl_pool(unsigned n, unsigned f, struct pl_pool *o) {
  (void)n; (void)f; (void)o; return -1;
}
