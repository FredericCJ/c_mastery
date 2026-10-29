#include "numeric.h"
#include <stdio.h>
int main(void) {
  char op;
  while (scanf(" %c", &op) == 1) {
    int status = ND_BAD;
    double out[257] = {0};
    size_t count = 1;
    if (op == 's') {
      unsigned m;
      size_t n;
      double x[4096];
      if (scanf("%u%zu", &m, &n) != 2 || n > 4096)
        return 2;
      for (size_t i = 0; i < n; i++)
        if (scanf("%la", &x[i]) != 1)
          return 2;
      status = nd_sum(x, n, m, out);
    } else if (op == 'i') {
      size_t n;
      int clamp;
      double q, x[256], y[256];
      if (scanf("%zu%d%la", &n, &clamp, &q) != 3 || n > 256)
        return 2;
      for (size_t i = 0; i < n; i++)
        if (scanf("%la%la", x + i, y + i) != 2)
          return 2;
      status = nd_interp(x, y, n, q, clamp, out);
    } else if (op == 'e') {
      double y, h, r;
      size_t n;
      if (scanf("%la%la%la%zu", &y, &h, &r, &n) != 4 || n > 256)
        return 2;
      status = nd_euler(y, h, r, n, out, 257);
      count = n + 1;
    } else if (op == 'g') {
      size_t n;
      double a[9], b[3], tol;
      struct nd_cost cost;
      if (scanf("%zu%la", &n, &tol) != 2 || n < 2 || n > 3)
        return 2;
      for (size_t i = 0; i < n * n; i++)
        if (scanf("%la", a + i) != 1)
          return 2;
      for (size_t i = 0; i < n; i++)
        if (scanf("%la", b + i) != 1)
          return 2;
      status = nd_solve((struct nd_view){a, n * n, n, n, n, 1}, b, n, tol, out, &cost);
      count = n;
    } else
      return 2;
    printf("%d", status);
    if (status == ND_OK)
      for (size_t i = 0; i < count; i++)
        printf(" %a", out[i]);
    putchar('\n');
  }
  return 0;
}
