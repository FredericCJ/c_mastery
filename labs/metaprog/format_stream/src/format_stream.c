#include "format_stream.h"

struct mfp_arg mfp_number(uint32_t v)
{
    struct mfp_arg a = {MFP_NUMBER, v, {NULL, 0}};
    return a;
}

struct mfp_arg mfp_bytes(struct mfp_span v)
{
    struct mfp_arg a = {MFP_BYTES, 0, v};
    return a;
}

int mfp_parse(struct mfp_span s, struct mfp_program *p)
{
    (void)s;
    (void)p;
    return MFP_INVALID;
}

int mfp_prepare(const struct mfp_program *p, const struct mfp_arg *a, size_t n,
                struct mfp_message *m)
{
    (void)p;
    (void)a;
    (void)n;
    (void)m;
    return MFP_INVALID;
}

int mfp_render(const struct mfp_program *p, const struct mfp_arg *a, size_t n,
               struct mp_sink *s)
{
    (void)p;
    (void)a;
    (void)n;
    (void)s;
    return MFP_INVALID;
}

int mfp_start(struct mfp_stream *s, const struct mfp_message *m)
{
    (void)s;
    (void)m;
    return MFP_INVALID;
}

int mfp_step(struct mfp_stream *s, mfp_writer_fn w, void *c)
{
    (void)s;
    (void)w;
    (void)c;
    return MFP_INVALID;
}

int mfp_cancel(struct mfp_stream *s)
{
    (void)s;
    return MFP_INVALID;
}
