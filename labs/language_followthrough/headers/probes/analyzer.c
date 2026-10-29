#include <stddef.h>
#if HH_MODEL == 0
int hh_analysis(size_t i)
{
    int values[4] = {10, 20, 30, 40};
    return values[i];
}
#elif HH_MODEL == 1
int hh_analysis(size_t i)
{
    int values[4] = {10, 20, 30, 40};
    if (i == 4)
    {
        return values[i];
    }
    return 0;
}
#else
int hh_analysis(size_t i)
{
    int values[4] = {10, 20, 30, 40};
    if (i >= 4)
    {
        return 0;
    }
    return values[i];
}
#endif
