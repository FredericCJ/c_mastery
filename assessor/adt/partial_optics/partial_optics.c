#include "partial_optics.h"
#ifndef ADL_MUTATION
#define ADL_MUTATION 0
#endif
static enum adl_status path(struct adl_config s)
{
    if (s.present > 1 || s.inner.present > 1)
    {
        return ADL_INVALID;
    }
    return s.present && s.inner.present ? ADL_OK : ADL_ABSENT;
}

enum adl_status adl_get(struct adl_config s, uint32_t *out)
{
    if (!out)
    {
        return ADL_INVALID;
    }
    enum adl_status status = path(s);
    if (status != ADL_OK)
    {
        return status;
    }
    *out = s.inner.endpoint.x;
    return ADL_OK;
}

enum adl_status adl_put(struct adl_config s, uint32_t replacement,
                        struct adl_config *out)
{
    if (!out)
    {
        return ADL_INVALID;
    }
    enum adl_status status = path(s);
    if (status != ADL_OK)
    {
        if (ADL_MUTATION != 1 || status != ADL_ABSENT)
        {
            return status;
        }
        s.present = 1;
        s.inner.present = 1;
    }
    s.inner.endpoint = ad_lens_x(s.inner.endpoint, replacement);
    if (ADL_MUTATION == 2)
    {
        s.inner.audit = replacement;
    }
    *out = s;
    return ADL_OK;
}

enum adl_status adl_update(struct adl_config s, const struct adl_step *steps,
                           size_t n, struct adl_config *out)
{
    if (!out || n > 2 || (n && !steps))
    {
        return ADL_INVALID;
    }
    for (size_t i = 0; i < n; i++)
    {
        if (!steps[i].change)
        {
            return ADL_INVALID;
        }
    }
    enum adl_status status = path(s);
    if (status != ADL_OK)
    {
        return status;
    }
    for (size_t i = 0; i < n; i++)
    {
        uint32_t value = 0;
        if (steps[i].change(steps[i].context, s.inner.endpoint.x, &value))
        {
            if (ADL_MUTATION == 6)
            {
                *out = s;
            }
            return ADL_CALLBACK;
        }
        if (ADL_MUTATION == 5 &&
            (steps[i].change(steps[i].context, s.inner.endpoint.x, &value)))
        {
            return ADL_CALLBACK;
        }
        s.inner.endpoint = ad_lens_x(s.inner.endpoint, value);
    }
    *out = s;
    return ADL_OK;
}

static enum adl_status io(struct adl_io s)
{
    if (s.version != 1 && s.version != 2)
    {
        return ADL_VERSION;
    }
    if (!s.code || s.code > (s.version == 1 ? 255u : 65535u) ||
        (s.version == 1 && s.context > 255u))
    {
        return ADL_RANGE;
    }
    return ADL_OK;
}

enum adl_status adl_review(struct adl_io s, struct adl_error *out)
{
    if (!out)
    {
        return ADL_INVALID;
    }
    enum adl_status status = io(s);
    if (status != ADL_OK)
    {
        return status;
    }
    struct adl_error result = {s.version, ADL_IO, {{s.code, s.context}}};
    if (ADL_MUTATION == 3)
    {
        result.version = 1;
    }
    *out = result;
    return ADL_OK;
}

enum adl_status adl_preview(struct adl_error s, struct adl_io *out)
{
    if (!out)
    {
        return ADL_INVALID;
    }
    if (s.version != 1 && s.version != 2)
    {
        return ADL_VERSION;
    }
    if (s.tag == ADL_SUCCESS)
    {
        return ADL_ABSENT;
    }
    if (s.tag == ADL_TIMEOUT)
    {
        if (s.version == 1)
        {
            return ADL_INVALID;
        }
        if (ADL_MUTATION == 4)
        {
            *out = (struct adl_io){2, 1, s.data.timeout};
            return ADL_OK;
        }
        return ADL_ABSENT;
    }
    if (s.tag != ADL_IO)
    {
        return ADL_INVALID;
    }
    struct adl_io result = {s.version, s.data.io.x, s.data.io.y};
    enum adl_status status = io(result);
    if (status != ADL_OK)
    {
        return status;
    }
    *out = result;
    return ADL_OK;
}
