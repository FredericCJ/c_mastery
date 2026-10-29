#include "opaque.h"

void forbidden(struct efr_opaque *p)
{
    p->value.denominator = 0;
}
