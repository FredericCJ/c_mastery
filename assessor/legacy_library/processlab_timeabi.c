#define _POSIX_C_SOURCE 200809L
#include "processlab.h"
#include <errno.h>
#include <stdio.h>
#define TYPE(x)                                   \
    _Generic((x),                                 \
        int: "int",                               \
        long: "long",                             \
        long long: "long-long",                   \
        unsigned int: "unsigned-int",             \
        unsigned long: "unsigned-long",           \
        unsigned long long: "unsigned-long-long", \
        float: "float",                           \
        double: "double",                         \
        long double: "long-double",               \
        default: "other")

int main(void)
{
    struct tm ordinary, future = {.tm_year = 77};
    if (lt_calendar(2000, &ordinary) != LT_OK)
    {
        return 1;
    }
    errno = 0;
    enum lt_status status = lt_calendar(2040, &future);
    int error = errno;
    if (sizeof(time_t) == 4)
    {
        if (status != LT_UNAVAILABLE || error != EOVERFLOW ||
            future.tm_year != 77)
        {
            return 1;
        }
    }
    else if (status != LT_OK || future.tm_year != 140)
    {
        return 1;
    }
    printf(
        "PROCESSLAB-PASS:time_bytes=%zu time_type=%s clock_bytes=%zu "
        "clock_type=%s future_status=%d errno=%d;no raw time_t serialization\n",
        sizeof(time_t), TYPE((time_t)0), sizeof(clock_t), TYPE((clock_t)0),
        status, error);
    return 0;
}
