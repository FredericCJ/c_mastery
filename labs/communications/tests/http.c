#include "com_http.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(x)                                                    \
    do                                                              \
    {                                                               \
        if (!(x))                                                   \
        {                                                           \
            fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #x); \
            return 1;                                               \
        }                                                           \
    } while (0)

static int finish(struct http_parser *p, const void *v, size_t n)
{
    size_t off = 0;
    int r = HTTP_NEED;
    while (off < n)
    {
        size_t used;
        r = http_feed(p, (const unsigned char *)v + off, n - off, &used);
        off += used;
        if (r != HTTP_NEED && r != HTTP_CONTINUE)
            break;
        if (!used)
            break;
    }
    return r;
}

static const char *authority = "127.0.0.1:8080";

static int headers(void)
{
    const char *requests[] = {
        "GET /health HTTP/1.1\r\nHost: 127.0.0.1:8080\r\n\r\n",
        "GET http://127.0.0.1:8080/health HTTP/1.1\r\nHost: "
        "ignored.example\r\n\r\n"};
    for (size_t i = 0; i < sizeof requests / sizeof *requests; i++)
    {
        const char *s = requests[i];
        size_t len = strlen(s);
        for (size_t split = 0; split <= len; split++)
        {
            struct http_parser p;
            http_init(&p, authority);
            int a = finish(&p, s, split);
            CHECK(a == HTTP_NEED || a == HTTP_DONE);
            CHECK(finish(&p, s + split, len - split) == HTTP_DONE ||
                  p.state == HTTP_FINISHED);
            CHECK(p.request.body_length == 0);
            CHECK(!strcmp(p.request.method, "GET"));
            CHECK(!strcmp(p.request.target, "/health"));
            CHECK(!strcmp(p.request.authority, authority));
        }
    }
    const char *bad[] = {
        "GET /health HTTP/1.1\nHost: 127.0.0.1:8080\n\n",
        "GET /health HTTP/1.1\r\nHost : x\r\n\r\n",
        "GET /health HTTP/1.1\r\nHost: x\r\nHost: y\r\n\r\n",
        "GET /health HTTP/1.1\r\nX: y\r\n\r\n",
        "GET /health HTTP/1.1\r\nHost: 127.0.0.1:8080\r\n folded\r\n\r\n"};
    for (size_t i = 0; i < sizeof bad / sizeof *bad; i++)
    {
        struct http_parser p;
        http_init(&p, authority);
        CHECK(finish(&p, bad[i], strlen(bad[i])) == HTTP_ERROR);
        CHECK(p.request.status == 400);
    }
    return 0;
}

static int framing(void)
{
    {
        /* sizeof on the explicit array includes the embedded NUL body byte. */
        const unsigned char req[] =
            "POST http://127.0.0.1:8080/echo HTTP/1.1\r\nHost: "
            "ignored.example\r\nContent-Length: 3\r\n\r\na\0b";
        const char *s = (const char *)req;
        size_t len = sizeof req - 1;
        for (size_t split = 0; split <= len; split++)
        {
            struct http_parser p;
            http_init(&p, authority);
            int a = finish(&p, s, split);
            CHECK(a == HTTP_NEED || a == HTTP_DONE);
            CHECK(finish(&p, s + split, len - split) == HTTP_DONE ||
                  p.state == HTTP_FINISHED);
            CHECK(p.request.body_length == 3 &&
                  !memcmp(p.request.body, "a\0b", 3));
            CHECK(!strcmp(p.request.authority, authority));
        }
    }

    struct edu_case
    {
        const char *fields;
        int status;
    } cases[] = {{"Content-Length: 0\r\nContent-Length: 0\r\n", 400},
                 {"Content-Length: 0\r\nTransfer-Encoding: chunked\r\n", 400},
                 {"Transfer-Encoding: gzip, chunked\r\n", 501},
                 {"Transfer-Encoding: chunked, gzip\r\n", 400},
                 {"Transfer-Encoding: chunked; x=y\r\n", 400},
                 {"Transfer-Encoding: chunked, chunked\r\n", 400},
                 {"Content-Length: 184467440737095516160\r\n", 400},
                 {"Content-Length: 1025\r\n", 413},
                 {"Expect: someday\r\n", 417},
                 {"Upgrade: example\r\n", 501}};

    for (size_t i = 0; i < sizeof cases / sizeof *cases; i++)
    {
        char s[512];
        snprintf(s, sizeof s, "POST /echo HTTP/1.1\r\nHost: %s\r\n%s\r\n",
                 authority, cases[i].fields);
        struct http_parser p;
        http_init(&p, authority);
        CHECK(finish(&p, s, strlen(s)) == HTTP_ERROR);
        CHECK(p.request.status == cases[i].status);
    }
    const char *s =
        "POST /echo HTTP/1.1\r\nHost: 127.0.0.1:8080\r\nContent-Length: "
        "3\r\nExpect: 100-continue\r\n\r\n";
    struct http_parser p;
    http_init(&p, authority);
    size_t used;
    CHECK(http_feed(&p, s, strlen(s), &used) == HTTP_CONTINUE);
    CHECK(used == strlen(s));
    CHECK(finish(&p, "abc", 3) == HTTP_DONE);
    for (size_t i = 1; i < strlen(s); i++)
    {
        http_init(&p, authority);
        finish(&p, s, i);
        CHECK(http_eof(&p) == HTTP_ERROR);
    }
    return 0;
}

static int chunks(void)
{
    const char *s =
        "POST /echo HTTP/1.1\r\nHost: 127.0.0.1:8080\r\nTransfer-Encoding: "
        "chunked\r\n\r\n2;x=\"a\\\"b\"\r\nab\r\n1\r\nc\r\n0\r\nHost: "
        "irrelevant\r\nX-End: yes\r\n\r\n";
    for (size_t i = 0; i <= strlen(s); i++)
    {
        struct http_parser p;
        http_init(&p, authority);
        finish(&p, s, i);
        int r = finish(&p, s + i, strlen(s) - i);
        CHECK(r == HTTP_DONE || p.state == HTTP_FINISHED);
        CHECK(p.request.body_length == 3 && !memcmp(p.request.body, "abc", 3));
        CHECK(!strcmp(p.request.authority, authority));
    }
    const char *body[] = {"1\r\naX\n0\r\n\r\n", "1;x=\"bad\r\na\r\n0\r\n\r\n",
                          "g\r\n", "401\r\n", "1;x=\r\na\r\n0\r\n\r\n"};
    for (size_t i = 0; i < sizeof body / sizeof *body; i++)
    {
        char r[512];
        snprintf(r, sizeof r,
                 "POST /echo HTTP/1.1\r\nHost: %s\r\nTransfer-Encoding: "
                 "chunked\r\n\r\n%s",
                 authority, body[i]);
        struct http_parser p;
        http_init(&p, authority);
        CHECK(finish(&p, r, strlen(r)) == HTTP_ERROR);
    }
    for (size_t i = 1; i < strlen(s); i++)
    {
        struct http_parser p;
        http_init(&p, authority);
        finish(&p, s, i);
        CHECK(http_eof(&p) == HTTP_ERROR);
    }
    return 0;
}

static int response(void)
{
    struct http_parser p;
    http_init(&p, authority);
    const char *s = "HEAD /health HTTP/1.1\r\nHost: 127.0.0.1:8080\r\n\r\n";
    CHECK(finish(&p, s, strlen(s)) == HTTP_DONE);
    char out[1600];
    size_t n;
    CHECK(!http_response(&p.request, "Sat, 10 Oct 2026 00:00:00 GMT", 0, out,
                         sizeof out - 1, &n));
    out[n] = 0;
    CHECK(strstr(out, "Content-Length: 3\r\n"));
    CHECK(!strcmp(out + n - 4, "\r\n\r\n"));
    CHECK(http_response(&p.request, "injected\r\n", 0, out, sizeof out, &n) !=
          0);
    CHECK(http_response(&p.request, "Sat, 10 Oct 2026 00:00:00 GMT", 0, out, 8,
                        &n) != 0);
    return 0;
}

struct edu_sink
{
    char out[32];
    size_t n;
    unsigned call;
};

static int write_fake(void *v, const void *p, size_t n)
{
    struct edu_sink *s = v;
    s->call++;
    if (s->call == 1)
        return -1;
    if (s->call == 3)
        return -2;
    size_t k = n > 2 ? 2 : n;
    memcpy(s->out + s->n, p, k);
    s->n += k;
    return (int)k;
}

static int io(void)
{
    struct edu_sink s = {0};
    size_t off = 0;
    CHECK(http_drain(write_fake, &s, "abcdef", 6, &off) == 0);
    CHECK(off == 2);
    CHECK(http_drain(write_fake, &s, "abcdef", 6, &off) == 1);
    CHECK(off == 6 && s.n == 6 && !memcmp(s.out, "abcdef", 6));
    return 0;
}

static int deadlines(void)
{
    CHECK(!http_expired(4999, 0, 0, 15000, 5000));
    CHECK(http_expired(5000, 0, 0, 15000, 5000));
    CHECK(http_expired(15000, 0, 14999, 15000, 5000));
    CHECK(http_expired(10, 20, 20, 15000, 5000));
    return 0;
}

int main(int argc, char **argv)
{
    if (argc != 2)
        return 2;
    int r = !strcmp(argv[1], "headers")     ? headers()
            : !strcmp(argv[1], "framing")   ? framing()
            : !strcmp(argv[1], "chunks")    ? chunks()
            : !strcmp(argv[1], "response")  ? response()
            : !strcmp(argv[1], "io")        ? io()
            : !strcmp(argv[1], "deadlines") ? deadlines()
                                            : 2;
    if (!r)
        printf("PASS http_%s\n", argv[1]);
    return r;
}
