#ifndef FT_LIST_H
#define FT_LIST_H
#include <stddef.h>
#include <stdint.h>

/* Maximum number of values one request may hold. Rebuild with
 * -DFT_CAPACITY=n (1..16) to change the capacity. */
#ifndef FT_CAPACITY
#define FT_CAPACITY 4
#endif
_Static_assert(FT_CAPACITY >= 1 && FT_CAPACITY <= 16,
               "explicit list capacity model");

/* Number of per-token diagnostics a struct ft_errors can store. */
enum
{
    FT_ERROR_CAPACITY = 8
};

/* The command-line tool prints the status name (see ft_name) and exits with
 * the enumerator value, so the numbers are part of the interface. */
enum ft_status
{
    FT_OK,
    /* No tokens: a successful request with count 0 and sum 0. */
    FT_EMPTY,
    /* A token is not [+-]?[0-9]+ (or a stream byte is NUL). */
    FT_MALFORMED,
    /* A token's value is outside [-32768, 32767]. */
    FT_RANGE,
    /* More than FT_CAPACITY tokens. */
    FT_CAPACITY_ERROR,
    /* Some running sum leaves [-32768, 32767]. */
    FT_PREFIX,
    /* Null pointer or collect flag other than 0 or 1. */
    FT_INVALID,
    /* Stream used after ft_stream_finish(). */
    FT_STATE,
    /* Returned by the unfinished learner stubs. */
    FT_NOT_IMPLEMENTED
};

/* Published only on FT_OK and FT_EMPTY; every error leaves it unchanged. */
struct ft_result
{
    size_t count;
    int32_t sum;
};

/* One diagnostic: the index of the offending token, or the token count for
 * an arithmetic (FT_PREFIX) failure, and its status. */
struct ft_error
{
    size_t token;
    enum ft_status status;
};

/* Caller-owned diagnostics. items holds the first FT_ERROR_CAPACITY errors in
 * encounter order, stored counts them, and total counts all errors found,
 * including those that did not fit. */
struct ft_errors
{
    struct ft_error items[FT_ERROR_CAPACITY];
    size_t stored;
    size_t total;
};

/* Lexical state of the token being read, owned by the implementation. Callers
 * must not edit it: public fields are representation, not permission to
 * forge a state. */
struct ft_token
{
    unsigned state;
    unsigned negative;
    unsigned magnitude;
    unsigned range;
};

/* Incremental request state. Create it with ft_stream_init(). */
struct ft_stream
{
    struct ft_token token;
    int32_t values[FT_CAPACITY];
    size_t count;
    enum ft_status status;
    unsigned finished;
};

/* Parses count decimal tokens and sums them.
 * Pre: tokens is an array of count live, null-terminated strings (it may be
 *      NULL only when count is 0); out and errors are live, writable and
 *      disjoint from each other and from the input. All are borrowed for the
 *      call; nothing is retained. collect is 0 (fail fast) or 1 (inspect
 *      every token).
 * Returns FT_INVALID, changing nothing, for a null out or errors, a null
 * array with count > 0, or collect not 0/1. A null token yields FT_INVALID
 * for that token. Otherwise errors is reset first and each token yields at
 * most one diagnostic (malformed or range before capacity). The return value
 * is the first token error; arithmetic is checked only when every token is
 * valid and failure returns FT_PREFIX with the diagnostic index set to count.
 * Every error leaves *out unchanged; FT_OK and FT_EMPTY publish it.
 * Never blocks or allocates. */
enum ft_status ft_tokens(const char *const *tokens, size_t count, int collect,
                         struct ft_result *out, struct ft_errors *errors);

/* Prepares stream for a new request. A null stream is ignored. */
void ft_stream_init(struct ft_stream *stream);

/* Feeds count bytes. Space, tab, CR and LF separate tokens; any other byte,
 * including NUL, belongs to a token. A chunk boundary is not a token
 * boundary. The bytes are borrowed only during the call; bytes may be NULL
 * when count is 0.
 * Returns FT_OK; FT_INVALID for a null stream or a null pointer with
 * count > 0; FT_STATE after ft_stream_finish(); or the sticky token error
 * (FT_MALFORMED, FT_RANGE or FT_CAPACITY_ERROR) once one has been found. */
enum ft_status ft_stream_feed(struct ft_stream *stream,
                              const unsigned char *bytes, size_t count);

/* Ends the request: completes the last token once, checks every prefix sum
 * and publishes *out on FT_OK or FT_EMPTY. Returns FT_INVALID for a null
 * argument, FT_STATE when called twice, or the sticky or arithmetic error
 * with *out unchanged. */
enum ft_status ft_stream_finish(struct ft_stream *stream,
                                struct ft_result *out);

/* Returns a static, never-null name for status ("unknown" if out of range).
 * The caller must not modify or free it. */
const char *ft_name(enum ft_status status);
#endif
