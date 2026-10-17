/* Intentionally flawed ORIGINAL teaching target, not a production formatter.
 * Execute only with format == "%n", a live int marker, and disjoint output
 * storage of at least one byte. Other formats require a separate type audit.
 * The demonstration changes a synthetic marker only; no input from a network,
 * command line, credentials, or third-party application is consumed.
 */
#include <stddef.h>
#include <stdio.h>

int teaching_render(const char *format, int *marker, char *out, size_t capacity)
{
    return snprintf(out, capacity, format, marker);
}
