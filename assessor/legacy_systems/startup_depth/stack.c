#include "startup.h"

static int word(const unsigned char *p, size_t n, size_t at, unsigned w,
                uint64_t *out)
{
    if (at > n || w > n - at)
        return 0;
    uint64_t value = 0;
    for (unsigned i = 0; i < w; i++)
        value |= (uint64_t)p[at + i] << (i * 8);
    *out = value;
    return 1;
}

enum sd_status sd_stack_decode(const unsigned char *p, size_t n, unsigned w,
                               struct sd_stack *out)
{
    if (!out || (!p && n))
        return SD_ARGUMENT;
    if (w != 4 && w != 8)
        return SD_UNSUPPORTED;
    if (n > 1024u * 1024u)
        return SD_UNSUPPORTED;
    uint64_t argc, value;
    size_t at = w;
    if (!word(p, n, 0, w, &argc))
        return SD_RANGE;
    if (argc > 4096)
        return SD_UNSUPPORTED;
    for (uint64_t i = 0; i < argc; i++, at += w)
    {
        if (!word(p, n, at, w, &value))
            return SD_RANGE;
        if (!value)
            return SD_FORMAT;
    }
    if (!word(p, n, at, w, &value))
        return SD_RANGE;
    if (value)
        return SD_FORMAT;
    at += w;
    size_t envp = at;
    unsigned env = 0;
    for (;;)
    {
        if (!word(p, n, at, w, &value))
            return SD_RANGE;
        at += w;
        if (!value)
            break;
        if (env++ == 4096)
            return SD_UNSUPPORTED;
    }
    size_t auxv = at;
    unsigned aux = 0;
    for (;;)
    {
        uint64_t kind;
        if (!word(p, n, at, w, &kind) || !word(p, n, at + w, w, &value))
            return SD_RANGE;
        at += 2 * w;
        if (!kind)
            break;
        if (aux++ == 256)
            return SD_UNSUPPORTED;
    }
    out->argc = argc;
    out->argv = w;
    out->envp = envp;
    out->auxv = auxv;
    out->extent = at;
    out->width = w;
    return SD_OK;
}

enum sd_status sd_string_lookup(uint64_t address, const struct sd_mapping *maps,
                                size_t count, const unsigned char **out,
                                size_t *length)
{
    if (!out || !length || (!maps && count))
        return SD_ARGUMENT;
    if (count > 4096)
        return SD_UNSUPPORTED;
    const unsigned char *match = NULL;
    size_t available = 0;
    for (size_t i = 0; i < count; i++)
    {
        const struct sd_mapping *m = &maps[i];
        if ((!m->bytes && m->length) || m->length > 65536 ||
            m->length > UINT64_MAX - m->label)
            return SD_ARGUMENT;
        if (address >= m->label && address - m->label < m->length)
        {
            if (match)
                return SD_FORMAT;
            size_t offset = (size_t)(address - m->label);
            match = m->bytes + offset;
            available = m->length - offset;
        }
    }
    if (!match)
        return SD_RANGE;
    size_t n = 0;
    while (n < available && match[n])
        n++;
    if (n == available)
        return SD_FORMAT;
    *out = match;
    *length = n;
    return SD_OK;
}
