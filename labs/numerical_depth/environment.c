#include "numeric.h"
#include <fenv.h>
#include <float.h>
#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
struct nd_work {
  const double *x;
  size_t n;
  double result;
};
static void *worker(void *p) {
  struct nd_work *w = p;
  if (nd_sum(w->x, w->n, ND_NAIVE, &w->result))
    abort();
  return NULL;
}
int main(void) {
  volatile double one = 1, small = 0x1p-53;
  double rounded[4];
  int modes[] = {FE_TONEAREST, FE_UPWARD, FE_DOWNWARD, FE_TOWARDZERO};
  for (size_t i = 0; i < 4; i++) {
    if (fesetround(modes[i]))
      return 2;
    rounded[i] = one + small;
  }
  if (fesetround(FE_TONEAREST))
    return 2;
  volatile double a = 1 + 0x1p-27, b = 1 - 0x1p-27, c = -1;
  double separate = a * b + c, fused = fma(a, b, c), v[] = {0x1p53, 1, -0x1p53};
  double reductions[2];
  for (size_t split = 1; split <= 2; split++) {
    struct nd_work w[2] = {{v, split, 0}, {v + split, 3 - split, 0}};
    pthread_t t[2];
    for (size_t i = 0; i < 2; i++)
      if (pthread_create(t + i, NULL, worker, w + i))
        return 3;
    for (size_t i = 0; i < 2; i++)
      if (pthread_join(t[i], NULL))
        return 3;
    reductions[split - 1] = w[0].result + w[1].result;
  }
  double comp;
  if (nd_sum(v, 3, ND_NEUMAIER, &comp))
    return 4;
  struct nd_work three[3] = {{v, 1, 0}, {v + 1, 1, 0}, {v + 2, 1, 0}};
  pthread_t workers[3];
  for (size_t i = 0; i < 3; i++)
    if (pthread_create(workers + i, NULL, worker, three + i))
      return 3;
  for (size_t i = 0; i < 3; i++)
    if (pthread_join(workers[i], NULL))
      return 3;
  double three_sum = (three[0].result + three[1].result) + three[2].result;
#ifdef __FAST_MATH__
  const int fast = 1;
#else
  const int fast = 0;
#endif
  printf("{\"fast_math\":%d,\"flt_eval_method\":%d,\"rounding\":[\"%a\",\"%a\","
         "\"%a\",\"%a\"],\"separate\":\"%a\",\"fma\":\"%a\",\"two_worker_"
         "splits\":[\"%a\",\"%a\"],\"compensated\":\"%a\",\"three_workers\":\"%"
         "a\"}\n",
         fast, FLT_EVAL_METHOD, rounded[0], rounded[1], rounded[2], rounded[3],
         separate, fused, reductions[0], reductions[1], comp, three_sum);
  return 0;
}
