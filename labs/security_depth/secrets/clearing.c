#include "secrets.h"
#include <string.h>

unsigned ss_dead_store(const unsigned char token[4])
{
    unsigned char copy[4];
    for (unsigned i = 0; i < 4; ++i)
        copy[i] = token[i];
    unsigned result = ss_consume(copy);
    memset(copy, 0, sizeof copy);
    return result;
}

unsigned ss_explicit_store(const unsigned char token[4])
{
    unsigned char copy[4];
    for (unsigned i = 0; i < 4; ++i)
        copy[i] = token[i];
    unsigned result = ss_consume(copy);
    ss_wipe(copy, sizeof copy);
    return result;
}
