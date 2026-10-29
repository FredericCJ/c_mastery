#include "format_stream.h"
#include <limits.h>
#ifndef MFP_MUTATION
#define MFP_MUTATION 0
#endif
_Static_assert(CHAR_BIT == 8 && '0' == 48 && '9' == 57 && '{' == 123 &&
                   '}' == 125,
               "This laboratory DSL and wire decimal use ASCII octets");

struct mfp_arg mfp_number(uint32_t value)
{
    struct mfp_arg a = {MFP_NUMBER, value, {NULL, 0}};
    return a;
}

struct mfp_arg mfp_bytes(struct mfp_span value)
{
    struct mfp_arg a = {MFP_BYTES, 0, value};
    return a;
}

int mfp_parse(struct mfp_span source, struct mfp_program *output)
{
    struct mfp_program p = {0};
    size_t i = 0, j;
    if (!output || source.length > 32 || (source.length && !source.bytes))
        return MFP_INVALID;
    p.source_length = source.length;
    for (j = 0; j < source.length; ++j)
        p.source[j] = source.bytes[j];
    while (i < source.length)
    {
        unsigned char c = p.source[i];
        struct mfp_node n = {MFP_LITERAL, 0, i, 1};
        if (c == '{' || c == '}')
        {
            if (i + 1 < source.length && p.source[i + 1] == c)
                i += 2;
            else if (c == '{' && source.length - i >= 4 &&
                     (p.source[i + 1] == 'u' || p.source[i + 1] == 's') &&
                     p.source[i + 2] >= '0' && p.source[i + 2] <= '3' &&
                     p.source[i + 3] == '}')
            {
                n.kind = p.source[i + 1] == 'u' ? MFP_NUMBER : MFP_BYTES;
                n.index = (unsigned)(p.source[i + 2] - '0');
                n.offset = 0;
                n.length = 0;
                if (p.required[n.index] && p.required[n.index] != n.kind)
                    return MFP_TYPE;
                p.required[n.index] = n.kind;
                i += 4;
            }
            else if (MFP_MUTATION == 1)
            {
                ++i;
            }
            else
                return MFP_FORMAT;
        }
        else
            ++i;
        p.nodes[p.count++] = n;
    }
    *output = p;
    return MFP_OK;
}

int mfp_prepare(const struct mfp_program *p, const struct mfp_arg *args,
                size_t count, struct mfp_message *output)
{
    struct mfp_message m = {{0}, 0};
    size_t i, j;
    if (!p || !output || count > 4 || (count && !args) || p->count > 32 ||
        p->source_length > 32)
        return MFP_INVALID;
    for (i = 0; i < count; ++i)
    {
        if (args[i].kind != MFP_NUMBER && args[i].kind != MFP_BYTES)
            return MFP_TYPE;
        if (args[i].kind == MFP_BYTES &&
            (args[i].span.length > 16 ||
             (args[i].span.length && !args[i].span.bytes)))
            return MFP_INVALID;
    }
    for (i = 0; i < 4; ++i)
        if (p->required[i])
        {
            if (i >= count)
                return MFP_TYPE;
            if (MFP_MUTATION != 2 && p->required[i] != args[i].kind)
                return MFP_TYPE;
        }
    for (i = 0; i < p->count; ++i)
    {
        struct mfp_node n = p->nodes[i];
        if (n.kind == MFP_LITERAL)
        {
            if (n.offset >= p->source_length || n.length != 1)
                return MFP_INVALID;
            if (m.length == sizeof m.bytes)
                return MFP_FULL;
            m.bytes[m.length++] = p->source[n.offset];
        }
        else
        {
            if (n.index >= count ||
                (n.kind != MFP_NUMBER && n.kind != MFP_BYTES))
                return MFP_INVALID;
            if (MFP_MUTATION != 2 && args[n.index].kind != n.kind)
                return MFP_TYPE;
            if (n.kind == MFP_NUMBER)
            {
                struct mp_sink s = {m.bytes, sizeof m.bytes, m.length};
                if (!mp_format(&s, mp_arg_number(args[n.index].number)))
                    return MFP_FULL;
                m.length = s.used;
            }
            else
            {
                struct mfp_span span = args[n.index].span;
                if (span.length > sizeof m.bytes - m.length)
                    return MFP_FULL;
                for (j = 0; j < span.length; ++j)
                    m.bytes[m.length + j] = span.bytes[j];
                m.length += span.length;
            }
        }
    }
    *output = m;
    return MFP_OK;
}

int mfp_render(const struct mfp_program *p, const struct mfp_arg *args,
               size_t count, struct mp_sink *sink)
{
    struct mfp_message m;
    size_t i;
    int status;
    if (!sink || sink->used > sink->capacity ||
        (sink->capacity && !sink->bytes))
        return MFP_INVALID;
    status = mfp_prepare(p, args, count, &m);
    if (status != MFP_OK)
        return status;
    if (m.length > sink->capacity - sink->used)
    {
        if (MFP_MUTATION == 3)
            ++sink->used;
        return MFP_FULL;
    }
    for (i = 0; i < m.length; ++i)
        sink->bytes[sink->used + i] = m.bytes[i];
    sink->used += m.length;
    return MFP_OK;
}

int mfp_start(struct mfp_stream *s, const struct mfp_message *m)
{
    struct mfp_stream next = {0};
    if (!s || !m || m->length > 128 || s->state > MFP_STOPPED)
        return MFP_INVALID;
    if (s->state == MFP_ACTIVE)
        return MFP_BUSY;
    next.message = *m;
    next.state = MFP_ACTIVE;
    *s = next;
    return MFP_OK;
}

int mfp_step(struct mfp_stream *s, mfp_writer_fn writer, void *context)
{
    struct mfp_write_result r;
    size_t left;
    if (!s || s->state > MFP_STOPPED || s->message.length > 128 ||
        s->offset > s->message.length)
        return MFP_INVALID;
    if (s->state == MFP_DONE)
        return MFP_OK;
    if (s->state == MFP_FAILED)
        return s->failure;
    if (s->state == MFP_STOPPED && MFP_MUTATION != 6)
        return MFP_CANCELLED;
    if (s->state != MFP_ACTIVE &&
        !(MFP_MUTATION == 6 && s->state == MFP_STOPPED))
        return MFP_INVALID;
    left = s->message.length - s->offset;
    if (!left)
    {
        s->state = MFP_DONE;
        return MFP_OK;
    }
    if (!writer)
        return MFP_INVALID;
    r = writer(context, s->message.bytes + s->offset, left);
    if (r.accepted > left || r.status > MFP_WRITE_ERROR ||
        (r.status == MFP_WRITE_OK && !r.accepted))
    {
        s->state = MFP_FAILED;
        s->uncertain = 1;
        s->failure = MFP_PROTOCOL;
        return MFP_PROTOCOL;
    }
    if (MFP_MUTATION != 4 &&
        !(MFP_MUTATION == 5 && r.status == MFP_WRITE_ERROR))
        s->offset += r.accepted;
    if (r.status == MFP_WRITE_ERROR)
    {
        s->state = MFP_FAILED;
        s->failure = MFP_IO;
        return MFP_IO;
    }
    if (s->offset == s->message.length)
    {
        s->state = MFP_DONE;
        return MFP_OK;
    }
    return r.status == MFP_WRITE_AGAIN ? MFP_AGAIN : MFP_MORE;
}

int mfp_cancel(struct mfp_stream *s)
{
    if (!s || s->state > MFP_STOPPED)
        return MFP_INVALID;
    if (s->state == MFP_ACTIVE)
    {
        s->state = MFP_STOPPED;
        return MFP_CANCELLED;
    }
    if (s->state == MFP_STOPPED)
        return MFP_CANCELLED;
    if (s->state == MFP_DONE)
        return MFP_OK;
    return s->state == MFP_FAILED ? s->failure : MFP_INVALID;
}
