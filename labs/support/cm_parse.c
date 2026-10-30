/*
 * Checked decimal conversion. Contract and grammar: cm_parse.h.
 */
#include "cm_parse.h"

#include <errno.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdlib.h>

enum cm_parse_status cm_parse_umax(const char *text, uintmax_t min,
                                   uintmax_t max, uintmax_t *value)
{
    if ((text == NULL) || (text[0] == '\0'))
    {
        return CM_PARSE_EMPTY;
    }

    /* strtoumax would silently accept leading whitespace and a sign (and
     * negate "-1" into UINTMAX_MAX), so the grammar is checked first. */
    for (const char *cursor = text; *cursor != '\0'; ++cursor)
    {
        if ((*cursor < '0') || (*cursor > '9'))
        {
            return CM_PARSE_SYNTAX;
        }
    }

    char *end = NULL;
    errno = 0;
    uintmax_t parsed = strtoumax(text, &end, 10);
    if (errno == ERANGE)
    {
        return CM_PARSE_RANGE;
    }
    if ((end == text) || (*end != '\0'))
    {
        return CM_PARSE_SYNTAX; /* unreachable after the digit check */
    }
    if ((parsed < min) || (parsed > max))
    {
        return CM_PARSE_RANGE;
    }
    *value = parsed;
    return CM_PARSE_OK;
}

enum cm_parse_status cm_parse_unsigned(const char *text, unsigned min,
                                       unsigned max, unsigned *value)
{
    uintmax_t parsed = 0;
    enum cm_parse_status status = cm_parse_umax(text, min, max, &parsed);

    if (status == CM_PARSE_OK)
    {
        *value = (unsigned)parsed; /* parsed <= max <= UINT_MAX: exact */
    }
    return status;
}

enum cm_parse_status cm_parse_size(const char *text, size_t min, size_t max,
                                   size_t *value)
{
    uintmax_t parsed = 0;
    enum cm_parse_status status = cm_parse_umax(text, min, max, &parsed);

    if (status == CM_PARSE_OK)
    {
        *value = (size_t)parsed; /* parsed <= max <= SIZE_MAX: exact */
    }
    return status;
}

const char *cm_parse_status_name(enum cm_parse_status status)
{
    switch (status)
    {
        case CM_PARSE_OK:
            return "ok";
        case CM_PARSE_EMPTY:
            return "empty";
        case CM_PARSE_SYNTAX:
            return "syntax";
        case CM_PARSE_RANGE:
            return "range";
        default:
            return "invalid-status";
    }
}
