/* Reference implementation of the list.h contract. Assessor-only.
 *
 * FT_MUTANT selects a wrong-but-defined variant for the mutation campaign
 * (0 is the correct reference). The workflow replay also flips the
 * FT_MUTANT default line below to build the "final-only prefix check"
 * revision, so that line must stay exactly as it is. */
#include "list.h"
#include <limits.h>
_Static_assert(INT_MAX >= 524288 && '0' == 48 && '9' == 57,
               "explicit hosted ASCII int32-or-wider profile");
#ifndef FT_MUTANT
#define FT_MUTANT 0
#endif

/* Declared value range of one token and of every running sum. */
enum
{
    VALUE_MIN = -32768,
    VALUE_MAX = 32767
};

/* Largest magnitude a token may reach, by sign: |-32768| and 32767. */
enum
{
    MAGNITUDE_LIMIT_NEGATIVE = 32768,
    MAGNITUDE_LIMIT_POSITIVE = 32767
};

/* Values of struct ft_token.state. A token is untouched until its first
 * byte, may be a sign alone, then digits; any other byte makes it malformed
 * for good. */
enum token_state
{
    TOKEN_UNTOUCHED,
    TOKEN_SIGN_ONLY,
    TOKEN_DIGITS,
    TOKEN_MALFORMED
};

/* Adds one decimal digit to the magnitude, or remembers that the value is out
 * of range. The comparison against limit / 10 and limit % 10 happens before
 * magnitude * 10 + digit, so the multiply never overflows. Once range is
 * set, magnitude stops growing, but lexical inspection continues: a huge
 * number with a trailing letter is malformed, not out of range. */
static void token_add_digit(struct ft_token *token, unsigned digit)
{
    const unsigned limit =
        token->negative ? MAGNITUDE_LIMIT_NEGATIVE : MAGNITUDE_LIMIT_POSITIVE;
    const int exceeds_limit =
        token->magnitude > limit / 10 ||
        (token->magnitude == limit / 10 && digit > limit % 10);

    if (exceeds_limit)
    {
        token->range = 1;
    }
    if (!token->range)
    {
        token->magnitude = token->magnitude * 10 + digit;
    }
}

/* Advances the token state machine by one byte. Arbitrarily many leading
 * zeros need no token buffer: the state is bounded and independent of the
 * token length. */
static void token_push(struct ft_token *token, unsigned char c)
{
    if (token->state == TOKEN_MALFORMED)
    {
        return;
    }
    if (token->state == TOKEN_UNTOUCHED && (c == '+' || c == '-'))
    {
        token->negative = c == '-';
        token->state = TOKEN_SIGN_ONLY;
        return;
    }
    if (c < '0' || c > '9')
    {
        if (FT_MUTANT != 5)
        {
            token->state = TOKEN_MALFORMED;
        }
        return;
    }
    token->state = TOKEN_DIGITS;
    token_add_digit(token, (unsigned)(c - '0'));
}

/* Converts a finished token. Returns FT_OK and stores the value, FT_MALFORMED
 * for an untouched, sign-only or malformed token, or FT_RANGE; *out is
 * written only on FT_OK. */
static enum ft_status token_value(const struct ft_token *token, int32_t *out)
{
    if (token->state != TOKEN_DIGITS)
    {
        return FT_MALFORMED;
    }
    if (token->range)
    {
        return FT_RANGE;
    }
    const int32_t magnitude = (int32_t)token->magnitude;
    *out = token->negative ? -magnitude : magnitude;
    return FT_OK;
}

/* Returns 1 when sum + value lies in [VALUE_MIN, VALUE_MAX], without forming
 * a sum that could overflow. Both operands are already in that range, so the
 * bounds VALUE_MAX - value and VALUE_MIN - value fit in int32_t. */
static int sum_stays_in_range(int32_t sum, int32_t value)
{
    if (value > 0)
    {
        return sum <= VALUE_MAX - value;
    }
    if (value < 0)
    {
        return sum >= VALUE_MIN - value;
    }
    return 1;
}

/* Checks every prefix sum of values[0..count) and publishes the result.
 * Returns FT_PREFIX, leaving *out unchanged, when any running sum leaves
 * the declared range, FT_EMPTY for no values and FT_OK otherwise. */
static enum ft_status evaluate(const int32_t *values, size_t count,
                               struct ft_result *out)
{
    int32_t sum = 0;

    if (FT_MUTANT == 3)
    {
        out->count = count;
    }
    for (size_t i = 0; i < count; i++)
    {
        if (FT_MUTANT != 1 && !sum_stays_in_range(sum, values[i]))
        {
            return FT_PREFIX;
        }
        /* Even the deliberately wrong final-only check is C-defined: <=16
         * values of magnitude 32768 fit this explicit int32 arithmetic
         * profile. */
        sum += values[i];
    }
    if (sum < VALUE_MIN || sum > VALUE_MAX)
    {
        return FT_PREFIX;
    }
    *out = (struct ft_result){count, sum};
    return count > 0 ? FT_OK : FT_EMPTY;
}

/* Counts every error but stores only the first FT_ERROR_CAPACITY, so the
 * diagnostics need no storage proportional to the input. */
static void record_error(struct ft_errors *errors, size_t token,
                         enum ft_status status)
{
    if (errors->stored < FT_ERROR_CAPACITY)
    {
        errors->items[errors->stored++] = (struct ft_error){token, status};
    }
    ++errors->total;
    if (FT_MUTANT == 4)
    {
        errors->total = 1;
    }
}

/* Classifies the token at position index. A null string is FT_INVALID. One
 * diagnostic per token: a lexical or range error takes precedence over the
 * capacity error. *value is written only on FT_OK. */
static enum ft_status classify_token(const char *text, size_t index,
                                     int32_t *value)
{
    if (text == NULL)
    {
        return FT_INVALID;
    }

    struct ft_token token = {0};
    for (const unsigned char *p = (const unsigned char *)text; *p != '\0'; p++)
    {
        token_push(&token, *p);
    }

    enum ft_status status = token_value(&token, value);
    if (status == FT_OK && index >= FT_CAPACITY)
    {
        status = FT_CAPACITY_ERROR;
    }
    return status;
}

enum ft_status ft_tokens(const char *const *tokens, size_t count, int collect,
                         struct ft_result *out, struct ft_errors *errors)
{
    if (out == NULL || errors == NULL || (count > 0 && tokens == NULL) ||
        (collect != 0 && collect != 1))
    {
        return FT_INVALID;
    }
    *errors = (struct ft_errors){0};

    /* Slot i is written only for a token that passed classify_token(), and
     * that never happens for i >= FT_CAPACITY, so the array cannot overflow. */
    int32_t values[FT_CAPACITY];
    enum ft_status first_error = FT_OK;
    for (size_t i = 0; i < count; i++)
    {
        int32_t value = 0;
        const enum ft_status status = classify_token(tokens[i], i, &value);

        if (status == FT_OK)
        {
            values[i] = value;
        }
        else
        {
            record_error(errors, i, status);
            if (first_error == FT_OK)
            {
                first_error = status;
            }
            if (!collect)
            {
                return status;
            }
        }
    }
    if (first_error != FT_OK)
    {
        return first_error;
    }

    /* Arithmetic is assessed only after every token passed. Its diagnostic
     * uses index count, one past the last token, as an aggregate marker. */
    const enum ft_status status = evaluate(values, count, out);
    if (status == FT_PREFIX)
    {
        record_error(errors, count, FT_PREFIX);
    }
    return status;
}

void ft_stream_init(struct ft_stream *stream)
{
    if (stream != NULL)
    {
        *stream = (struct ft_stream){0};
    }
}

/* Moves the finished token into stream->values. On any error the status
 * becomes sticky: it is stored in stream->status and returned. Invariant on
 * success: the token is reset, ready for the next one. */
static enum ft_status stream_complete_token(struct ft_stream *stream)
{
    int32_t value = 0;
    enum ft_status status = token_value(&stream->token, &value);

    if (status == FT_OK && stream->count == FT_CAPACITY)
    {
        status = FT_CAPACITY_ERROR;
    }
    if (status != FT_OK)
    {
        stream->status = status;
        return status;
    }
    stream->values[stream->count++] = value;
    stream->token = (struct ft_token){0};
    return FT_OK;
}

/* The only token separators; the chunk boundary is not one. */
static int is_separator(unsigned char c)
{
    return c == ' ' || c == '\t' || c == '\r' || c == '\n';
}

enum ft_status ft_stream_feed(struct ft_stream *stream,
                              const unsigned char *bytes, size_t count)
{
    if (stream == NULL || (count > 0 && bytes == NULL))
    {
        return FT_INVALID;
    }
    if (stream->finished)
    {
        return FT_STATE;
    }
    if (stream->status != FT_OK)
    {
        return stream->status;
    }
    for (size_t i = 0; i < count; i++)
    {
        if (!is_separator(bytes[i]))
        {
            token_push(&stream->token, bytes[i]);
        }
        else if (stream->token.state != TOKEN_UNTOUCHED &&
                 stream_complete_token(stream) != FT_OK)
        {
            return stream->status;
        }
    }
    return FT_OK;
}

enum ft_status ft_stream_finish(struct ft_stream *stream, struct ft_result *out)
{
    if (stream == NULL || out == NULL)
    {
        return FT_INVALID;
    }
    if (stream->finished)
    {
        return FT_STATE;
    }
    stream->finished = 1;
    if (stream->status != FT_OK)
    {
        return stream->status;
    }
    /* End of input is a token boundary: flush the last token exactly once. */
    if (stream->token.state != TOKEN_UNTOUCHED && FT_MUTANT != 2 &&
        stream_complete_token(stream) != FT_OK)
    {
        return stream->status;
    }
    stream->status = evaluate(stream->values, stream->count, out);
    return stream->status;
}

const char *ft_name(enum ft_status status)
{
    /* Indexed by enum ft_status; keep in the same order. */
    static const char *const names[] = {
        "ok",     "empty",   "malformed", "range",          "capacity",
        "prefix", "invalid", "state",     "not-implemented"};
    return (unsigned)status < sizeof names / sizeof names[0] ? names[status]
                                                             : "unknown";
}
