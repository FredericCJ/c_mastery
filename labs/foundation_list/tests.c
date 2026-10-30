#include "list.h"
#include <stdio.h>
#include <stdlib.h>
#define CHECK(x)                                                            \
    do                                                                      \
    {                                                                       \
        if (!(x))                                                           \
        {                                                                   \
            fprintf(stderr, "FT_CHECK:%s:%d:%s\n", __FILE__, __LINE__, #x); \
            exit(1);                                                        \
        }                                                                   \
    } while (0)

/* Sentinel result: a call that fails must leave exactly these values. */
#define UNTOUCHED_COUNT 91
#define UNTOUCHED_SUM 92

#define ARRAY_LENGTH(array) (sizeof(array) / sizeof((array)[0]))

static struct ft_result untouched_result(void)
{
    return (struct ft_result){UNTOUCHED_COUNT, UNTOUCHED_SUM};
}

static void token_sums(void)
{
    const char *normal[] = {"2", "-5", "9"};
    const char *bad[] = {"32767", "1", "-1"};
    struct ft_result out = untouched_result();
    struct ft_errors e;

    CHECK(ft_tokens(normal, 3, 0, &out, &e) == FT_OK && out.count == 3 &&
          out.sum == 6 && e.total == 0);

    /* Every prefix must fit: 32767 + 1 leaves the range even though the
     * final sum 32767 would fit. The diagnostic index is the token count. */
    out = untouched_result();
    CHECK(ft_tokens(bad, 3, 0, &out, &e) == FT_PREFIX && out.count == 91 &&
          out.sum == 92);
    CHECK(e.total == 1 && e.items[0].token == 3 &&
          e.items[0].status == FT_PREFIX);

    CHECK(ft_tokens(NULL, 0, 0, &out, &e) == FT_EMPTY && out.count == 0 &&
          out.sum == 0);
}

static void token_grammar(void)
{
    const char *spelling[] = {"+", "12x", "", " 1", "1 ", "++1", "--2"};
    const char *range[] = {"32768", "-32769",
                           "9999999999999999999999999999999999999"};
    const char *limits[] = {"-32768", "32767", "1", "+00000"};
    struct ft_result out = untouched_result();
    struct ft_errors e;

    for (size_t i = 0; i < ARRAY_LENGTH(spelling); i++)
    {
        CHECK(ft_tokens(&spelling[i], 1, 0, &out, &e) == FT_MALFORMED);
    }
    for (size_t i = 0; i < ARRAY_LENGTH(range); i++)
    {
        CHECK(ft_tokens(&range[i], 1, 0, &out, &e) == FT_RANGE);
    }
    CHECK(ft_tokens(limits, 4, 0, &out, &e) == FT_OK && out.sum == 0);
}

static void token_capacity_and_collection(void)
{
    const char *too_many[] = {"0", "0", "0", "0", "0"};
    const char *errors[] = {"bad", "99999", "+", "1", "2", "--3",
                            "0",   "0",     "0", "0", "0"};
    struct ft_result out = untouched_result();
    struct ft_errors e;

    CHECK(ft_tokens(too_many, 5, 0, &out, &e) == FT_CAPACITY_ERROR &&
          out.count == 91 && out.sum == 92);

    /* Collect-all: tokens 0, 1, 2 and 5 are lexical or range errors, token 3
     * is the only valid token, and tokens 4 and 6..10 exceed the capacity of
     * 4: ten errors in all. The count is exact but only the first
     * FT_ERROR_CAPACITY (8) are stored. */
    CHECK(ft_tokens(errors, 11, 1, &out, &e) == FT_MALFORMED &&
          out.count == 91 && out.sum == 92 && e.total == 10 && e.stored == 8);
    CHECK(e.items[0].token == 0 && e.items[1].status == FT_RANGE &&
          e.items[3].token == 4 && e.items[3].status == FT_CAPACITY_ERROR);
}

static void cases(void)
{
    token_sums();
    token_grammar();
    token_capacity_and_collection();
    puts("FT_TOKEN_PASS grammar ranges capacity4 atomic_prefix "
         "collected10_stored8");
}

/* Feeds "2 -5 9" in two chunks at every split point, including both ends. */
static void stream_every_split(void)
{
    const unsigned char bytes[] = "2 -5 9";
    const size_t length = sizeof bytes - 1;

    for (size_t split = 0; split <= length; split++)
    {
        struct ft_stream s;
        struct ft_result r = untouched_result();

        ft_stream_init(&s);
        CHECK(ft_stream_feed(&s, bytes, split) == FT_OK);
        CHECK(ft_stream_feed(&s, bytes + split, length - split) == FT_OK);
        CHECK(ft_stream_finish(&s, &r) == FT_OK && r.count == 3 && r.sum == 6);
        CHECK(ft_stream_finish(&s, &r) == FT_STATE && r.sum == 6);
        CHECK(ft_stream_feed(&s, NULL, 0) == FT_STATE);
    }
}

static void stream_edge_inputs(void)
{
    struct ft_stream s;
    struct ft_result r = untouched_result();

    ft_stream_init(&s);
    CHECK(ft_stream_feed(&s, (const unsigned char *)" \t\r\n", 4) == FT_OK);
    CHECK(ft_stream_finish(&s, &r) == FT_EMPTY && r.sum == 0);

    ft_stream_init(&s);
    CHECK(ft_stream_feed(&s, (const unsigned char *)"+", 1) == FT_OK);
    r = untouched_result();
    CHECK(ft_stream_finish(&s, &r) == FT_MALFORMED && r.sum == 92);

    /* An embedded NUL is a malformed byte, not the end of the token. */
    const unsigned char embedded[] = {'1', 0, '2'};
    ft_stream_init(&s);
    CHECK(ft_stream_feed(&s, embedded, 3) == FT_OK);
    CHECK(ft_stream_finish(&s, &r) == FT_MALFORMED && r.sum == 92);

    /* A thousand leading zeros need no token buffer. */
    ft_stream_init(&s);
    for (unsigned i = 0; i < 1000; i++)
    {
        CHECK(ft_stream_feed(&s, (const unsigned char *)"0", 1) == FT_OK);
    }
    CHECK(ft_stream_feed(&s, (const unsigned char *)"7", 1) == FT_OK);
    CHECK(ft_stream_finish(&s, &r) == FT_OK && r.sum == 7 && r.count == 1);
}

/* Feeds each failing request one byte at a time. The error may surface
 * during feeding or only at finish, but finish must report it and publish
 * nothing. */
static void stream_sticky_errors(void)
{
    const char *failures[] = {"32768", "0 0 0 0 0", "32767 1 -1"};

    const enum ft_status statuses[] = {FT_RANGE, FT_CAPACITY_ERROR, FT_PREFIX};

    for (unsigned j = 0; j < ARRAY_LENGTH(failures); ++j)
    {
        struct ft_stream s;
        struct ft_result r = untouched_result();
        enum ft_status feed = FT_OK;

        ft_stream_init(&s);
        for (const unsigned char *p = (const unsigned char *)failures[j];
             *p != '\0' && feed == FT_OK; ++p)
        {
            feed = ft_stream_feed(&s, p, 1);
        }
        CHECK(feed == FT_OK || feed == statuses[j]);
        CHECK(ft_stream_finish(&s, &r) == statuses[j] && r.count == 91 &&
              r.sum == 92);
    }
}

static void streams(void)
{
    stream_every_split();
    stream_edge_inputs();
    stream_sticky_errors();
    puts("FT_STREAM_PASS all7splits EOF_once embeddedNUL long_leading_zeros");
}

int main(void)
{
    cases();
    streams();
    puts("FT_PASS bounded_list_and_changed_input");
    return 0;
}
