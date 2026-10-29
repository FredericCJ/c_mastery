#ifndef LIBRARY_PROCESSLAB_H
#define LIBRARY_PROCESSLAB_H
#include <time.h>

enum lt_status
{
    LT_OK,
    LT_ARGUMENT,
    LT_UNAVAILABLE,
    LT_RANGE,
    LT_NOT_IMPLEMENTED
};
enum lt_status lt_cpu_interval(clock_t begin, clock_t end,
                               long double *seconds);
/* External synchronization must cover lookup through completed copy. */
enum lt_status lt_environment(const char *name, char out[64]);
/* Synthetic February29 in year2000, otherwise January1; caller pins TZ/locale.
 */
enum lt_status lt_calendar(int year, struct tm *copy);
#endif
