#include "algorithms.h"
#include <stdio.h>
#include <string.h>
#include <limits.h>
#define CHECK(x)                                                              \
    do                                                                        \
    {                                                                         \
        if (!(x))                                                             \
        {                                                                     \
            fprintf(stderr, "ALGORITHM_ASSERT parser line%d: %s\n", __LINE__, \
                    #x);                                                      \
            return 1;                                                         \
        }                                                                     \
    } while (0)

static enum ad_status lookup(void *context, unsigned char name, int32_t *out)
{
    if (name != 'x')
    {
        return AD_MISSING;
    }
    *out = *(int32_t *)context;
    return AD_OK;
}

static int evaluate(const char *source, unsigned extensions, int32_t expected)
{
    struct ad_token tokens[65];
    size_t count;
    struct ad_diagnostic diagnostic;
    struct ad_ast ast;
    int32_t value = 99, variable = 4;
    CHECK(ad_lex((const unsigned char *)source, strlen(source), tokens, 65,
                 &count, &diagnostic) == AD_OK);
    CHECK(ad_parse(tokens, count, 16, extensions, 32, UINT_MAX, &ast,
                   &diagnostic) == AD_OK);
    CHECK(ad_evaluate(&ast, lookup, &variable, &value, &diagnostic) == AD_OK &&
          value == expected);
    return 0;
}

int main(void)
{
    unsigned char source[] = "2+3*(4+1)";
    struct ad_token tokens[65];
    size_t count;
    struct ad_diagnostic diagnostic;
    struct ad_ast ast;
    CHECK(ad_lex(source, 9, tokens, 65, &count, &diagnostic) == AD_OK &&
          count == 10);
    for (unsigned i = 0; i < 9; ++i)
    {
        CHECK(tokens[i].offset == i && tokens[i].length == 1);
    }
    CHECK(tokens[9].kind == AD_T_END && tokens[9].offset == 9);
    CHECK(!evaluate((const char *)source, 0, 17));
    CHECK(!evaluate("2^3^2", 1, 512));
    CHECK(!evaluate("x*(2+3)", 1, 20));
    CHECK(!evaluate("0^0", 1, 1));
    unsigned char recycled[] = "123+4", owned[4];
    size_t copied;
    CHECK(ad_lex(recycled, 5, tokens, 65, &count, &diagnostic) == AD_OK);
    CHECK(ad_token_copy(recycled, 5, tokens[0], owned, 4, &copied) == AD_OK &&
          copied == 3);
    memset(recycled, '?', 5);
    CHECK(!memcmp(owned, "123", 3));

    struct
    {
        const char *text;
        size_t offset;
    } malformed[] = {{"2+*3", 2}, {"(2+3", 4}, {"2 3", 2},
                     {"", 0},     {"2^3", 1},  {"x+1", 0}};

    for (size_t i = 0; i < sizeof malformed / sizeof *malformed; ++i)
    {
        CHECK(ad_lex((const unsigned char *)malformed[i].text,
                     strlen(malformed[i].text), tokens, 65, &count,
                     &diagnostic) == AD_OK);
        memset(&ast, 77, sizeof ast);
        struct ad_ast old = ast;
        CHECK(ad_parse(tokens, count, 16, 0, 32, UINT_MAX, &ast, &diagnostic) ==
                  AD_INVALID &&
              diagnostic.offset == malformed[i].offset &&
              !memcmp(&old, &ast, sizeof ast));
    }
    CHECK(ad_lex(source, 9, tokens, 65, &count, &diagnostic) == AD_OK);
    CHECK(ad_parse(tokens, count, 16, 0, 32, UINT_MAX, &ast, &diagnostic) ==
          AD_OK);
    unsigned nodes = ast.count;
    for (unsigned failure = 0; failure < nodes; ++failure)
    {
        struct ad_ast old = ast;
        CHECK(ad_parse(tokens, count, 16, 0, 32, failure, &ast, &diagnostic) ==
                  AD_SPACE &&
              !memcmp(&old, &ast, sizeof ast));
    }
    const char *nested = "(((1)))";
    CHECK(ad_lex((const unsigned char *)nested, 7, tokens, 65, &count,
                 &diagnostic) == AD_OK);
    CHECK(ad_parse(tokens, count, 3, 0, 32, UINT_MAX, &ast, &diagnostic) ==
              AD_SPACE &&
          diagnostic.offset == 3);
    CHECK(ad_parse(tokens, count, 4, 0, 32, UINT_MAX, &ast, &diagnostic) ==
          AD_OK);

    struct
    {
        const char *text;
        enum ad_status status;
        size_t offset;
    } semantic[] = {{"2147483647+1", AD_OVERFLOW, 10},
                    {"50000*50000", AD_OVERFLOW, 5},
                    {"y+1", AD_MISSING, 0},
                    {"2^32", AD_INVALID, 1}};

    for (size_t i = 0; i < sizeof semantic / sizeof *semantic; ++i)
    {
        CHECK(ad_lex((const unsigned char *)semantic[i].text,
                     strlen(semantic[i].text), tokens, 65, &count,
                     &diagnostic) == AD_OK);
        CHECK(ad_parse(tokens, count, 16, 1, 32, UINT_MAX, &ast, &diagnostic) ==
              AD_OK);
        int32_t value = 99, variable = 4;
        CHECK(ad_evaluate(&ast, lookup, &variable, &value, &diagnostic) ==
                  semantic[i].status &&
              value == 99 && diagnostic.offset == semantic[i].offset);
    }
    CHECK(ad_lex((const unsigned char *)"2147483648", 10, tokens, 65, &count,
                 &diagnostic) == AD_OVERFLOW &&
          diagnostic.offset == 9);
    CHECK(ad_lex((const unsigned char *)"1@2", 3, tokens, 65, &count,
                 &diagnostic) == AD_INVALID &&
          diagnostic.offset == 1);
    struct ad_token old_tokens[65];
    memcpy(old_tokens, tokens, sizeof tokens);
    size_t old_count = count;
    CHECK(ad_lex(source, 9, tokens, 9, &count, &diagnostic) == AD_SPACE &&
          count == old_count && !memcmp(old_tokens, tokens, sizeof tokens));
    size_t cases = 0;
    for (int a = 0; a < 10; ++a)
    {
        for (int b = 0; b < 10; ++b)
        {
            for (int c = 0; c < 10; ++c)
            {
                char expression[64];
                snprintf(expression, sizeof expression, "%d+%d*%d", a, b, c);
                CHECK(!evaluate(expression, 0, a + b * c));
                snprintf(expression, sizeof expression, "(%d+%d)*%d", a, b, c);
                CHECK(!evaluate(expression, 0, (a + b) * c));
                cases += 2;
            }
        }
    }
    printf(
        "ALGORITHM_PARSER_PASS grammar_cases=%zu every_AST_failure=%u lexical_syntax_semantic_separation\n",
        cases, nodes);
    return 0;
}
