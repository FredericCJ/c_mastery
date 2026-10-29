#include "format.h"
#include <stdarg.h>
#include <stdint.h>
#ifndef TF_MUTANT
#define TF_MUTANT 0
#endif
struct tf_reader
{
    va_list *list;
    const struct tf_arg *array;
    unsigned position;
};

static enum tf_status next(struct tf_reader *r, struct tf_arg *out)
{
    out->tag = r->list ? va_arg(*r->list, int) : r->array[r->position].tag;
    if (out->tag == 'I')
    {
        out->integer =
            r->list ? va_arg(*r->list, int) : r->array[r->position].integer;
    }
    else if (out->tag == 'D')
    {
        out->real =
            r->list ? va_arg(*r->list, double) : r->array[r->position].real;
    }
    else
    {
        return TF_INVALID; /* No unknown payload is consumed. */
    }
    ++r->position;
    return TF_OK;
}

static int convert(struct tf_scalars s, struct tf_arg arg, char *out,
                   size_t capacity)
{
    return arg.tag == 'I' ? s.integer(s.context, out, capacity, arg.integer)
                          : s.real(s.context, out, capacity, arg.real);
}

static enum tf_status sizing(struct tf_reader *r, unsigned count,
                             struct tf_scalars scalar, size_t sizes[4],
                             size_t *total)
{
    size_t sum = 0;
    for (unsigned i = 0; i < count; ++i)
    {
        struct tf_arg arg = {0};
        enum tf_status status = next(r, &arg);
        if (status != TF_OK)
        {
            return status;
        }
        int n = convert(scalar, arg, NULL, 0);
        if (n < 0)
        {
            return TF_FORMAT;
        }
        /* Reserve the final NUL without wrapping size_t. */
        if ((uintmax_t)(unsigned)n > SIZE_MAX - sum - 1)
        {
            return TF_SIZE;
        }
        sizes[i] = (size_t)n;
        sum += (size_t)n;
    }
    *total = sum;
    return TF_OK;
}

static enum tf_status size_variadic(va_list *source, unsigned count,
                                    struct tf_scalars scalar, size_t sizes[4],
                                    size_t *total)
{
    va_list copy;
    va_copy(copy, *source);
    struct tf_reader r = {&copy, NULL, 0};
    enum tf_status status = sizing(&r, count, scalar, sizes, total);
    va_end(copy);
    return status;
}

static enum tf_status render(struct tf_reader *r, unsigned count,
                             struct tf_scalars scalar, const size_t sizes[4],
                             char *stage, size_t total)
{
    size_t at = 0;
    stage[0] = 0;
    for (unsigned i = 0; i < count; ++i)
    {
        struct tf_arg arg = {0};
        enum tf_status status = next(r, &arg);
        if (status != TF_OK)
        {
            return status;
        }
        int n = convert(scalar, arg, stage + at, total + 1 - at);
        if (n < 0 || (uintmax_t)(unsigned)n != sizes[i])
        {
            return TF_FORMAT;
        }
        at += sizes[i];
    }
    return at == total ? TF_OK : TF_FORMAT;
}

static enum tf_status render_variadic(va_list *source, unsigned count,
                                      struct tf_scalars scalar,
                                      const size_t sizes[4], char *stage,
                                      size_t total)
{
    va_list copy;
    va_copy(copy, *source);
    struct tf_reader r = {&copy, NULL, 0};
    enum tf_status status = render(&r, count, scalar, sizes, stage, total);
    va_end(copy);
    return status;
}

static int admitted(char *out, size_t capacity, size_t *required,
                    struct tf_allocator a, struct tf_scalars s, unsigned count)
{
    return required && (out || capacity == 0) && a.acquire && a.release &&
           s.integer && s.real && count <= 4;
}

static enum tf_status finish(char *out, size_t *required, struct tf_allocator a,
                             char *stage, size_t total, enum tf_status status)
{
    if (status == TF_OK)
    {
        for (size_t i = 0; i <= total; ++i)
        {
            out[i] = stage[i];
        }
        *required = TF_MUTANT == 1 ? total + 1 : total;
    }
    if (TF_MUTANT != 2)
    {
        a.release(a.context, stage);
    }
    return status;
}

enum tf_status tf_format(char *out, size_t capacity, size_t *required,
                         struct tf_allocator a, struct tf_scalars scalar,
                         unsigned count, ...)
{
    if (!admitted(out, capacity, required, a, scalar, count))
    {
        return TF_INVALID;
    }
    va_list source;
    va_start(source, count);
    size_t sizes[4], total = 0;
    enum tf_status status =
        size_variadic(&source, count, scalar, sizes, &total);
    if (status != TF_OK)
    {
        va_end(source);
        return status;
    }
    if (capacity <= total)
    {
        *required = total;
        if (TF_MUTANT == 3 && out && capacity)
        {
            out[0] = 0;
        }
        va_end(source);
        return TF_FULL;
    }
    char *stage = a.acquire(a.context, total + 1);
    if (!stage)
    {
        va_end(source);
        return TF_NOMEM;
    }
    status = render_variadic(&source, count, scalar, sizes, stage, total);
    va_end(source);
    return finish(out, required, a, stage, total, status);
}

enum tf_status tf_array(char *out, size_t capacity, size_t *required,
                        struct tf_allocator a, struct tf_scalars scalar,
                        const struct tf_arg *args, size_t available,
                        unsigned count)
{
    if (!admitted(out, capacity, required, a, scalar, count) ||
        count > available || (count && !args))
    {
        return TF_INVALID;
    }
    struct tf_reader first = {NULL, args, 0};
    size_t sizes[4], total = 0;
    enum tf_status status = sizing(&first, count, scalar, sizes, &total);
    if (status != TF_OK)
    {
        return status;
    }
    if (capacity <= total)
    {
        *required = total;
        return TF_FULL;
    }
    char *stage = a.acquire(a.context, total + 1);
    if (!stage)
    {
        return TF_NOMEM;
    }
    struct tf_reader second = {NULL, args, 0};
    status = render(&second, count, scalar, sizes, stage, total);
    return finish(out, required, a, stage, total, status);
}
