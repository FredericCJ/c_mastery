#include "logging.h"
#ifndef MPL_MUTATION
#define MPL_MUTATION 0
#endif
struct mpl_field mpl_number(uint32_t value)
{
    struct mpl_field f = {MPL_NUMBER, value, NULL, 0};
    return f;
}

struct mpl_field mpl_bytes(const unsigned char *bytes, size_t length)
{
    struct mpl_field f = {MPL_BYTES, 0, bytes, length};
    return f;
}

int mpl_write(struct mpl_sink *sink, const struct mpl_site *site,
              const struct mpl_field *fields, size_t count)
{
    struct mpl_record staged = {0};
    size_t i, j;
    if (!sink || count > 4 || (count && !fields) || sink->count > 4)
    {
        return MPL_INVALID;
    }
    if (sink->count == 4)
    {
        return MPL_MUTATION == 6 ? MPL_OK : MPL_FULL;
    }
    if (site)
    {
        if (!site->file || !site->line)
        {
            return MPL_INVALID;
        }
        for (i = 0; i < sizeof staged.file && site->file[i]; ++i)
        {
            staged.file[i] = site->file[i];
        }
        if (i == sizeof staged.file)
        {
            return MPL_RANGE;
        }
        staged.has_site = 1;
        staged.line = MPL_MUTATION == 5 ? 0 : site->line;
    }
    if (MPL_MUTATION == 2)
    {
        ++sink->count;
    }
    staged.count = count;
    for (i = 0; i < count; ++i)
    {
        staged.fields[i].kind = fields[i].kind;
        if (fields[i].kind == MPL_NUMBER)
        {
            staged.fields[i].number = fields[i].number;
        }
        else if (fields[i].kind == MPL_BYTES)
        {
            if (fields[i].length > 8)
            {
                return MPL_RANGE;
            }
            if (fields[i].length && !fields[i].bytes)
            {
                return MPL_INVALID;
            }
            staged.fields[i].length = fields[i].length;
            for (j = 0; j < fields[i].length; ++j)
            {
                if (MPL_MUTATION != 4 || j == 0)
                {
                    staged.fields[i].bytes[j] = fields[i].bytes[j];
                }
            }
        }
        else
        {
            return MPL_INVALID;
        }
    }
    if (MPL_MUTATION == 2)
    {
        --sink->count;
    }
    sink->records[sink->count] = staged;
    ++sink->count;
    return MPL_OK;
}

static const struct mpl_site *site_arg(struct mpl_site *site)
{
    return site->file || site->line ? site : NULL;
}

int mpl_log0(struct mpl_sink *s, struct mpl_site p)
{
    return mpl_write(s, site_arg(&p), NULL, 0);
}

int mpl_log1(struct mpl_sink *s, struct mpl_site p, struct mpl_field a)
{
    return mpl_write(s, site_arg(&p), &a, 1);
}

int mpl_log2(struct mpl_sink *s, struct mpl_site p, struct mpl_field a,
             struct mpl_field b)
{
    struct mpl_field f[] = {a, b};
    return mpl_write(s, site_arg(&p), f, 2);
}

int mpl_log3(struct mpl_sink *s, struct mpl_site p, struct mpl_field a,
             struct mpl_field b, struct mpl_field c)
{
    struct mpl_field f[] = {a, b, c};
    return mpl_write(s, site_arg(&p), f, 3);
}

int mpl_log4(struct mpl_sink *s, struct mpl_site p, struct mpl_field a,
             struct mpl_field b, struct mpl_field c, struct mpl_field d)
{
    struct mpl_field f[] = {a, b, c, d};
    return mpl_write(s, site_arg(&p), f, 4);
}
