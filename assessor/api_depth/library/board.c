#include "support.h"

/* Dedicated teaching register model. No MMIO or board execution is claimed. */
int lc_board_backend(struct lc_backend *p, int value)
{
    ++p->calls;
    return value - p->bias;
}
