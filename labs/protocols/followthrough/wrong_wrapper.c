#include "protocols.h"

int wrong(struct pt_closed *closed, struct pt_ready *ready)
{
    return pt_close(closed, ready);
}
