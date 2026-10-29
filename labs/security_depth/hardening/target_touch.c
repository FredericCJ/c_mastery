#include "target.h"

void sh_target_touch(volatile unsigned char *p)
{
    (void)*p;
}
