#include "list.h"

/* Learner scaffold: every function is deliberately unfinished. It compiles
 * and returns FT_NOT_IMPLEMENTED so the public tests fail until you
 * implement the contract in list.h. */

enum ft_status ft_tokens(const char *const *tokens, size_t count, int collect,
                         struct ft_result *out, struct ft_errors *errors)
{
    (void)tokens;
    (void)count;
    (void)collect;
    (void)out;
    (void)errors;
    return FT_NOT_IMPLEMENTED;
}

void ft_stream_init(struct ft_stream *stream)
{
    (void)stream;
}

enum ft_status ft_stream_feed(struct ft_stream *stream,
                              const unsigned char *bytes, size_t count)
{
    (void)stream;
    (void)bytes;
    (void)count;
    return FT_NOT_IMPLEMENTED;
}

enum ft_status ft_stream_finish(struct ft_stream *stream, struct ft_result *out)
{
    (void)stream;
    (void)out;
    return FT_NOT_IMPLEMENTED;
}

const char *ft_name(enum ft_status status)
{
    (void)status;
    return "not-implemented";
}
