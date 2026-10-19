#ifndef CMASTERY_ERRORS_RATIONAL_H
#define CMASTERY_ERRORS_RATIONAL_H
#include "errors.h"
typedef struct {int32_t numerator;uint32_t denominator;} EfrSigned;
uint32_t efr_gcd(uint32_t,uint32_t,unsigned *steps);
er_code efr_consume(er_fraction,uint32_t *quotient);
er_code efr_add_naive(er_fraction,er_fraction,er_fraction *);
er_code efr_add(er_fraction,er_fraction,er_fraction *);
er_code efr_signed_make(int32_t,uint32_t,EfrSigned *);
er_code efr_signed_add(EfrSigned,EfrSigned,EfrSigned *);
er_code efr_signed_consume(EfrSigned,int32_t *quotient);
#endif
