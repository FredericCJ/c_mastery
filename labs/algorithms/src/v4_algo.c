#include "v4_algo.h"
/* Implement one unit at a time. Returning failure keeps the scaffold buildable.
 */
int v4_reserve(V4Array *a, size_t n) {
  (void)a;
  (void)n;
  return -1;
}
int v4_list_push(V4List *a, int32_t n) {
  (void)a;
  (void)n;
  return -1;
}
int v4_list_pop(V4List *a, int32_t *n) {
  (void)a;
  (void)n;
  return -1;
}
int v4_free_take(V4Free *a, unsigned n, unsigned *p) {
  (void)a;
  (void)n;
  (void)p;
  return -1;
}
int v4_free_return(V4Free *a, unsigned n, unsigned p) {
  (void)a;
  (void)n;
  (void)p;
  return -1;
}
int v4_radix(uint32_t *a, size_t n, uint32_t *p, size_t c) {
  (void)a;
  (void)n;
  (void)p;
  (void)c;
  return -1;
}
int v4_knapsack(const unsigned *w, const unsigned *v, size_t n, unsigned c,
                unsigned *p) {
  (void)w;
  (void)v;
  (void)n;
  (void)c;
  (void)p;
  return -1;
}
int v4_filter(V4Filter *f, int32_t n, int32_t *a, int32_t *b) {
  (void)f;
  (void)n;
  (void)a;
  (void)b;
  return -1;
}
