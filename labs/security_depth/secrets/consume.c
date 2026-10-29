#include "secrets.h"

unsigned ss_consume(const unsigned char token[4])
{
    unsigned sum = 0;
    for (unsigned i = 0; i < 4; ++i)
        sum += token[i];
    return sum;
}
