#include "adt.h"

int probe(enum ad_tag tag)
{
    switch (tag)
    {
        case AD_EMPTY:
            return 0;
        case AD_NUMBER:
            return 1;
        default:
            return -1;
    }
}
