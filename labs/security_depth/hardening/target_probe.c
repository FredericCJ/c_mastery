#include "target.h"

unsigned sh_target_probe(unsigned x)
{
    volatile unsigned char bytes[16];
    bytes[0] = (unsigned char)x;
    sh_target_touch(bytes);
    return bytes[0];
}
