#include "record.h"

unsigned record_calls;

unsigned record_score(unsigned tag, unsigned size)
{
    ++record_calls;
    if (tag == 3u)
        return size * RECORD_WEIGHT;
    return size + RECORD_WEIGHT;
}
