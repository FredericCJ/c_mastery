#include <fenv.h>
#include <stdio.h>
#include <tgmath.h>
_Static_assert(_Generic(sqrt(4), double: 1, default: 0),
               "integer selects double");
_Static_assert(_Generic(sqrt(4.0f), float: 1, default: 0),
               "float selects float");
_Static_assert(_Generic(sqrt((long double complex)4),
                   long double complex: 1,
                   default: 0),
               "long complex selects complex");

double lm_real(double x)
{
    return sqrt(x);
}

double lm_integer(int x)
{
    return sqrt(x);
}

double complex lm_complex(double complex x)
{
    return sqrt(x);
}

long double complex lm_long_complex(long double complex x)
{
    return sqrt(x);
}

int main(void)
{
    double complex z = lm_complex(-1.0 + 0.0 * I);
    long double complex l = lm_long_complex(-1.0L + 0.0L * I);
    if (lm_integer(4) != 2 || creal(z) != 0 || cimag(z) != 1 ||
        creall(l) != 0 || cimagl(l) != 1)
        return 1;
    puts(
        "MATHLAB-PASS:type-generic real,integer,complex,long-complex contracts");
    return 0;
}
