#include "format.h"
#include <limits.h>
#include <stdio.h>
#include <string.h>
#define CHECK(x)                                                   \
    do                                                             \
    {                                                              \
        if (!(x))                                                  \
        {                                                          \
            fprintf(stderr, "FORMAT-CHECK:%d:%s\n", __LINE__, #x); \
            return 1;                                              \
        }                                                          \
    } while (0)

struct backend
{
    int calls, fail_query, fail_render, change, no_term;
};

struct sized
{
    int required, calls;
};

static int sized_render(void *ctx, char *d, size_t cap)
{
    struct sized *s = ctx;
    ++s->calls;
    if (cap)
    {
        size_t n = (size_t)s->required < cap ? (size_t)s->required : cap - 1;
        for (size_t i = 0; i < n; ++i)
            d[i] = 'x';
        d[n] = 0;
    }
    return s->required;
}

static int render(void *ctx, char *d, size_t n)
{
    struct backend *b = ctx;
    ++b->calls;
    if (!n && b->fail_query)
        return -1;
    if (n && b->fail_render)
    {
        d[0] = '!';
        return -1;
    }
    if (n && b->no_term)
    {
        for (size_t i = 0; i < n; ++i)
            d[i] = 'x';
        return 6;
    }
    int result = snprintf(d, n, "%s", "abcdef");
    return n && b->change ? result + 1 : result;
}

int main(void)
{
    const size_t caps[] = {0, 1, 4, 7};
    const size_t written[] = {0, 0, 3, 6};
    for (unsigned i = 0; i < 4; ++i)
    {
        char buf[12];
        memset(buf, '#', sizeof buf);
        struct backend b = {0};
        struct lf_result r = {91, 92, 0, 0};
        enum lf_status st =
            lf_prefix(render, &b, caps[i] ? buf : NULL, caps[i], &r);
        CHECK(st == (i == 3 ? LF_OK : LF_TRUNCATED) && r.required == 6 &&
              r.written == written[i] && r.terminated == (i != 0) &&
              r.complete == (i == 3));
        CHECK(b.calls == 1);
        for (size_t j = 0; j < written[i]; ++j)
            CHECK(buf[j] == "abcdef"[j]);
        if (caps[i])
            CHECK(buf[written[i]] == 0);
        for (size_t j = caps[i]; j < sizeof buf; ++j)
            CHECK(buf[j] == '#');
    }
    for (size_t cap = 0; cap <= 8; ++cap)
    {
        char buf[12];
        memset(buf, '#', sizeof buf);
        struct backend b = {0};
        struct lf_result r = {91, 92, 0, 0};
        enum lf_status st =
            lf_transaction(render, &b, cap ? buf : NULL, cap, &r);
        if (cap <= 6)
        {
            CHECK(st == LF_CAPACITY && b.calls == 1 && r.required == 91);
            for (size_t i = 0; i < sizeof buf; ++i)
                CHECK(buf[i] == '#');
        }
        else
            CHECK(st == LF_OK && b.calls == 2 && r.required == 6 &&
                  r.written == 6 && r.terminated && r.complete &&
                  !strcmp(buf, "abcdef") && buf[7] == '#');
    }
    for (unsigned failure = 0; failure < 4; ++failure)
    {
        struct backend b = {0};
        b.fail_query = failure == 0;
        b.fail_render = failure == 1;
        b.change = failure == 2;
        b.no_term = failure == 3;
        char buf[12];
        memset(buf, '#', sizeof buf);
        struct lf_result r = {91, 92, 0, 0};
        enum lf_status s = lf_transaction(render, &b, buf, sizeof buf, &r);
        CHECK(s == (failure < 2 ? LF_ENCODING : LF_BACKEND) &&
              r.required == 91 && r.written == 92 &&
              b.calls == (failure == 0 ? 1 : 2));
        for (size_t i = 0; i < sizeof buf; ++i)
            CHECK(buf[i] == '#');
    }
    const int lengths[] = {0, 63, 64, INT_MAX};
    for (unsigned k = 0; k < 4; ++k)
    {
        struct sized s = {lengths[k], 0};
        char bytes[66];
        memset(bytes, '#', sizeof bytes);
        struct lf_result r = {91, 92, 0, 0};
        enum lf_status result = lf_transaction(sized_render, &s, bytes, 64, &r);
        CHECK(result == (k < 2 ? LF_OK : LF_CAPACITY) &&
              s.calls == (k < 2 ? 2 : 1));
        if (k < 2)
        {
            CHECK(r.required == (size_t)lengths[k] && r.complete &&
                  bytes[lengths[k]] == 0);
        }
        else
        {
            CHECK(r.required == 91);
            for (size_t j = 0; j < 64; ++j)
                CHECK(bytes[j] == '#');
        }
        CHECK(bytes[64] == '#' && bytes[65] == '#');
    }
    struct backend b = {.fail_query = 1};
    struct lf_result r = {91, 92, 0, 0};
    CHECK(lf_prefix(render, &b, NULL, 0, &r) == LF_ENCODING &&
          r.required == 91);
    enum lf_policy p = LF_U32;
    CHECK(lf_parse_policy("%n", &p) == LF_ARGUMENT && p == LF_U32);
    CHECK(lf_parse_policy("text", &p) == LF_OK && p == LF_TEXT);
    char buf[64];
    struct lf_value value = {.tag = LF_U32, .data.u32 = UINT32_MAX};
    CHECK(lf_typed(LF_U32, value, buf, sizeof buf, &r) == LF_OK &&
          !strcmp(buf, "4294967295"));
    CHECK(lf_typed(LF_TEXT, value, buf, sizeof buf, &r) == LF_ARGUMENT);
    b = (struct backend){0};
    CHECK(lf_prefix(render, &b, buf, 6, &r) == LF_TRUNCATED && !r.complete &&
          r.written == 5 && buf[5] == 0);
    value.tag = LF_TEXT;
    value.data.text = "%n";
    CHECK(lf_typed(LF_TEXT, value, buf, sizeof buf, &r) == LF_OK &&
          !strcmp(buf, "%n"));
    CHECK(snprintf(buf, sizeof buf, "%8.3s", "abcdef") == 8 &&
          !strcmp(buf, "     abc"));
    CHECK(snprintf(buf, sizeof buf, "%*.*s", -8, 3, "abcdef") == 8 &&
          !strcmp(buf, "abc     "));
    CHECK(snprintf(buf, sizeof buf, "%*.*s", 8, -3, "abcdef") == 8 &&
          !strcmp(buf, "  abcdef"));
    const char euro[] = "\xe2\x82\xac";
    CHECK(snprintf(buf, sizeof buf, "%.2s", euro) == 2 &&
          (unsigned char)buf[0] == 0xe2 && (unsigned char)buf[1] == 0x82 &&
          buf[2] == 0);
    puts("FORMAT-PASS:prefix:transaction:typed:width:byte-precision");
    return 0;
}
