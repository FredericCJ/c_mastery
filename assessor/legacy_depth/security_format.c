/* Withheld repair: trusted format keeps input bytes out of formatting control. */
#include <stddef.h>
#include <stdio.h>

int teaching_render(const char *format, int *marker, char *out, size_t capacity)
{
    (void)marker;
    return snprintf(out, capacity, "%s", format);
}
