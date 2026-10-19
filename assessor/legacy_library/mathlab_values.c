#include <fenv.h>
#include <float.h>
#include <math.h>
#include <stdio.h>
int main(void) {
  if (FLT_RADIX != 2 || DBL_MANT_DIG != 53 || sizeof(double) != 8 ||
      fesetround(FE_TONEAREST))
    return 1;
  const double roots[] = {0,
                          0x1p-1074,
                          0x1p-1022,
                          0x1p-100,
                          0x1.fffffffffffffp-1,
                          1,
                          0x1.0000000000001p0,
                          2,
                          0x1.fffffffffffffp1023};
  const double angles[] = {-1,
                           -0x1.fffffffffffffp-1,
                           -0.5,
                           0,
                           0.5,
                           0x1.ffffffffffffdp-1,
                           0x1.ffffffffffffep-1,
                           0x1.fffffffffffffp-1,
                           1};
  for (size_t i = 0; i < sizeof roots / sizeof *roots; ++i)
    printf("sqrt %a %a\n", roots[i], sqrt(roots[i]));
  for (size_t i = 0; i < sizeof angles / sizeof *angles; ++i)
    printf("acos %a %a\n", angles[i], acos(angles[i]));
  return 0;
}
