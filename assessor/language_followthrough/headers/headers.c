#include <headers.h>
#ifdef HH_MUTANT_SHARED
unsigned hh_shared_state;
#endif
int hh_read(const int *values, size_t length, size_t index, int *out)
{
    if (!values || !out || index >= length)
    {
#ifdef HH_MUTANT_OUTPUT
        if (out)
            *out = 0;
#endif
        return -1;
    }
    *out = values[index];
    return 0;
}
