#include "posix_lifecycle.h"
int pl_launch(const char *p, char *const a[], unsigned f, PLLaunch *o) {
  (void)p; (void)a; (void)f; (void)o; return -1;
}
int pl_pool(unsigned n, unsigned f, PLPool *o) {
  (void)n; (void)f; (void)o; return -1;
}
