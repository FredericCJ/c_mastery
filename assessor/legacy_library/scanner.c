#include "scanner.h"
#include "foundation.h"

static int literal(struct f_cursor *c, const char *s, size_t n)
{
    const uint8_t *p;
    if (f_take(c, n, &p) != F_OK)
    {
        return 0;
    }
    for (size_t i = 0; i < n; ++i)
    {
        if (p[i] != (unsigned char)s[i])
        {
            return 0;
        }
    }
    return 1;
}

enum ls_status ls_record_parse(const unsigned char *p, size_t n, int escaped,
                               struct ls_record *out)
{
    if (!out || (!p && n) || (escaped != 0 && escaped != 1))
    {
        return LS_ARGUMENT;
    }
    struct f_cursor cursor = {p, n, 0};
    struct ls_record trial = {0};
    if (!literal(&cursor, "count=", 6))
    {
        return LS_SYNTAX;
    }
    size_t first = cursor.position;
    while (cursor.position < n && p[cursor.position] >= '0' &&
           p[cursor.position] <= '9')
    {
        ++cursor.position;
    }
    size_t digits = cursor.position - first;
    if (!digits || !literal(&cursor, ";name=", 6))
    {
        return LS_SYNTAX;
    }
    enum f_status status = f_parse_i32(p + first, digits, &trial.count);
    if (status != F_OK)
    {
        return status == F_RANGE ? LS_RANGE : LS_SYNTAX;
    }
#ifdef LS_MUTANT_COMMIT
    out->count = trial.count;
#endif
    while (cursor.position < n)
    {
        const uint8_t *next;
        if (f_take(&cursor, 1, &next) != F_OK)
        {
            return LS_SYNTAX;
        }
        unsigned char ch = *next;
        if (ch == '\\' && escaped)
        {
            if (f_take(&cursor, 1, &next) != F_OK)
            {
                return LS_SYNTAX;
            }
            ch = *next;
            if (ch != ';' && ch != '\\')
            {
                return LS_SYNTAX;
            }
        }
        else if (!((ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z') ||
                   (ch >= '0' && ch <= '9') || ch == '_'))
        {
#ifdef LS_MUTANT_SUFFIX
            break;
#else
            return LS_SYNTAX;
#endif
        }
        if (trial.length == 32)
        {
            return LS_LONG;
        }
        trial.name[trial.length++] = ch;
    }
    if (!trial.length)
    {
        return LS_SYNTAX;
    }
    trial.name[trial.length] = 0;
    *out = trial;
    return LS_OK;
}

enum ls_status ls_line(FILE *f, unsigned char *out, size_t cap, size_t *length)
{
    if (!f || !out || !length || !cap || cap > 129)
    {
        return LS_ARGUMENT;
    }
    unsigned char staged[128];
    size_t n = 0;
    int over = 0, any = 0, ch;
    while ((ch = fgetc(f)) != EOF)
    {
        any = 1;
        if (ch == '\n')
        {
            break;
        }
        if (n < cap - 1)
        {
            staged[n++] = (unsigned char)ch;
        }
        else
        {
            over = 1;
        }
#ifdef LS_MUTANT_DRAIN
        if (over)
        {
            return LS_LONG;
        }
#endif
    }
    if (ferror(f))
    {
        return LS_IO;
    }
    if (!any && ch == EOF)
    {
        return LS_END;
    }
    if (over)
    {
        return LS_LONG;
    }
    for (size_t i = 0; i < n; ++i)
    {
        out[i] = staged[i];
    }
    out[n] = 0;
    *length = n;
    return LS_OK;
}
