#include "escape.h"
#include <stdio.h>
#include <string.h>
#define REQUIRE(x)                                              \
    do                                                          \
    {                                                           \
        if (!(x))                                               \
        {                                                       \
            fprintf(stderr, "ES_ASSERT:%d:%s\n", __LINE__, #x); \
            return 41;                                          \
        }                                                       \
    } while (0)

int main(void)
{
    struct es_text a, b, old;
    memset(&old, 0xa5, sizeof old);
    a = old;
    const unsigned char plain[] = "&<>\"'";
    const unsigned char coded[] = "&amp;&lt;&gt;&quot;&apos;";
    REQUIRE(es_encode(plain, 5, 384, &a) == ES_OK);
    REQUIRE(a.length == 25 && !memcmp(a.bytes, coded, 25));
    REQUIRE(es_decode(a.bytes, a.length, 64, &b) == ES_OK && b.length == 5 &&
            !memcmp(b.bytes, plain, 5));
    for (size_t cap = 0; cap < 25; cap++)
    {
        a = old;
        REQUIRE(es_encode(plain, 5, cap, &a) == ES_CAPACITY &&
                !memcmp(&a, &old, sizeof a));
    }
    for (size_t cap = 0; cap < 5; cap++)
    {
        a = old;
        REQUIRE(es_decode(coded, 25, cap, &a) == ES_CAPACITY &&
                !memcmp(&a, &old, sizeof a));
    }
    const unsigned char once[] = "&amp;lt;x&amp;gt;";
    REQUIRE(es_decode(once, sizeof once - 1, 64, &a) == ES_OK &&
            a.length == 9 && !memcmp(a.bytes, "&lt;x&gt;", 9));
    const char *bad[] = {"&",     "&a",     "&amp",    "&ampx;", "&lt",
                         "&#39;", "&#x27;", "&bogus;", "<",      ">",
                         "\"",    "'",      "&AMP;",   "&amp;&", "\r"};
    for (size_t i = 0; i < sizeof bad / sizeof *bad; i++)
    {
        a = old;
        REQUIRE(es_decode((const unsigned char *)bad[i], strlen(bad[i]), 0,
                          &a) == ES_SYNTAX &&
                !memcmp(&a, &old, sizeof a));
    }
    unsigned char raw[65];
    for (size_t i = 0; i < 65; i++)
        raw[i] = '"';
    REQUIRE(es_encode(raw, 64, 384, &a) == ES_OK && a.length == 384);
    REQUIRE(es_decode(a.bytes, a.length, 64, &b) == ES_OK && b.length == 64);
    a = old;
    REQUIRE(es_encode(raw, 65, 384, &a) == ES_RANGE &&
            !memcmp(&a, &old, sizeof a));
    for (size_t i = 0; i < 65; i++)
        raw[i] = 'a';
    REQUIRE(es_decode(raw, 65, 64, &a) == ES_RANGE &&
            !memcmp(&a, &old, sizeof a));
    REQUIRE(es_encode(NULL, 0, 0, &a) == ES_OK && a.length == 0);
    REQUIRE(es_decode(NULL, 0, 0, &a) == ES_OK && a.length == 0);
    a = old;
    REQUIRE(es_encode(NULL, 1, 0, &a) == ES_INVALID &&
            !memcmp(&a, &old, sizeof a));
    REQUIRE(es_encode(raw, 0, 385, &a) == ES_INVALID &&
            !memcmp(&a, &old, sizeof a));
    REQUIRE(es_decode(raw, 385, 0, &a) == ES_RANGE &&
            !memcmp(&a, &old, sizeof a));
    REQUIRE(es_recode_html((const unsigned char *)"&#39;&amp;#39;", 14, 384,
                           &a) == ES_OK);
    REQUIRE(a.length == 15 && !memcmp(a.bytes, "&apos;&amp;#39;", 15));
    REQUIRE(es_recode_html((const unsigned char *)"&apos;", 6, 384, &b) ==
            ES_SYNTAX);
    puts("ES_OK boundaries, single pass, exact dialect and unchanged failures");
    return 0;
}
