#include "text.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(x)                                                \
    do                                                          \
    {                                                           \
        if (!(x))                                               \
        {                                                       \
            fprintf(stderr, "TC_ASSERT:%d:%s\n", __LINE__, #x); \
            return 41;                                          \
        }                                                       \
    } while (0)

static int expression(const char *s, struct pd_tree *tree)
{
    struct pd_tokens t;
    struct pd_diagnostic d;
    int r = pd_lex((const unsigned char *)s, strlen(s), &t, &d);
    return r ? r : pd_pratt(&t, 1, tree, &d);
}

static int same_tree(const struct pd_tree *a, const struct pd_tree *b)
{
    if (a->count != b->count || a->root != b->root)
    {
        return 0;
    }
    for (unsigned i = 0; i < a->count; i++)
    {
        const struct pd_expr *x = a->nodes + i, *y = b->nodes + i;
        if (x->kind != y->kind || x->left != y->left || x->right != y->right ||
            x->value != y->value)
        {
            return 0;
        }
    }
    return 1;
}

static int recovery(void)
{
    struct tc_batch b;
    CHECK(tc_recover((const unsigned char *)"1+;22;3*4;", 10, 0, 200, &b) ==
          TC_RECOVERED);
    CHECK(b.count == 2 && b.errors == 1 && b.values[0] == 22 &&
          b.values[1] == 12 && b.begins[0] == 3);
    CHECK(tc_recover((const unsigned char *)"2147483647+1;9;", 15, 0, 200,
                     &b) == TC_RECOVERED &&
          b.count == 1 && b.values[0] == 9 && b.errors == 1);
    CHECK(tc_recover((const unsigned char *)"1\n2;", 4, 0, 200, &b) ==
              TC_RECOVERED &&
          b.count == 0);
    CHECK(tc_recover((const unsigned char *)"1\n2;", 4, 1, 200, &b) == TC_OK &&
          b.count == 2);
    CHECK(tc_recover((const unsigned char *)"(1+;2;", 6, 0, 200, &b) ==
              TC_RECOVERED &&
          b.count == 1 && b.values[0] == 2);
    CHECK(tc_recover((const unsigned char *)"1;2", 3, 0, 200, &b) ==
              TC_RECOVERED &&
          b.count == 1 && b.diagnostics[0].status == 105);
    CHECK(tc_recover((const unsigned char *)";;", 2, 0, 200, &b) ==
              TC_RECOVERED &&
          b.errors == 2);
    CHECK(tc_recover(NULL, 0, 0, 1, &b) == TC_OK && b.count == 0 &&
          b.errors == 0);
    memset(&b, 0x5a, sizeof b);
    struct tc_batch old = b;
    CHECK(tc_recover((const unsigned char *)"1;", 2, 0, 2, &b) == TC_FUEL &&
          memcmp(&b, &old, sizeof b) == 0);
    unsigned char many[34];
    for (unsigned i = 0; i < 17; i++)
    {
        many[i * 2] = '1';
        many[i * 2 + 1] = ';';
    }
    CHECK(tc_recover(many, 34, 0, 100, &b) == TC_RANGE &&
          memcmp(&b, &old, sizeof b) == 0);
    CHECK(tc_recover(NULL, 1, 0, 100, &b) == TC_INVALID &&
          memcmp(&b, &old, sizeof b) == 0);
    puts("OBS recovery");
    return 0;
}

/* group(nest(indent, a LINE b)) CAT tail; an unbreakable continuation matters.
 */
void tc_fixture_doc(struct tc_doc *d, unsigned a, unsigned b, unsigned tail,
                    unsigned indent)
{
    memset(d, 0, sizeof *d);
    for (unsigned i = 0; i < a; i++)
    {
        d->text[d->length++] = 'a';
    }
    for (unsigned i = 0; i < b; i++)
    {
        d->text[d->length++] = 'b';
    }
    for (unsigned i = 0; i < tail; i++)
    {
        d->text[d->length++] = 'z';
    }
    d->nodes[0] = (struct tc_node){TD_TEXT, 0, 0, 0, 0, a};
    d->nodes[1] = (struct tc_node){TD_LINE, 0, 0, 0, 0, 0};
    d->nodes[2] = (struct tc_node){TD_TEXT, 0, 0, 0, a, b};
    d->nodes[3] = (struct tc_node){TD_CAT, 0, 1, 0, 0, 0};
    d->nodes[4] = (struct tc_node){TD_CAT, 3, 2, 0, 0, 0};
    d->nodes[5] = (struct tc_node){TD_NEST, 4, 0, indent, 0, 0};
    d->nodes[6] = (struct tc_node){TD_GROUP, 5, 0, 0, 0, 0};
    d->nodes[7] = (struct tc_node){TD_TEXT, 0, 0, 0, a + b, tail};
    d->nodes[8] = (struct tc_node){TD_CAT, 6, 7, 0, 0, 0};
    d->count = 9;
    d->root = 8;
}

static int layout(void)
{
    struct tc_doc d;
    struct tc_text out;
    tc_fixture_doc(&d, 1, 1, 3, 2);
    CHECK(tc_layout(&d, 4, 1000, &out) == TC_OK && out.length == 8 &&
          memcmp(out.bytes, "a\n  bzzz", 8) == 0);
    CHECK(tc_layout(&d, 6, 1000, &out) == TC_OK && out.length == 6 &&
          memcmp(out.bytes, "a bzzz", 6) == 0);
    for (unsigned width = 0; width <= 30; width++)
    {
        for (unsigned indent = 0; indent < 4; indent++)
        {
            tc_fixture_doc(&d, 3, 4, 2, indent);
            CHECK(tc_layout(&d, width, 1000, &out) == TC_OK);
            CHECK(out.length == (width >= 10 ? 10 : 10 + indent));
            CHECK(out.bytes[3] == (width >= 10 ? ' ' : '\n'));
        }
    }
    const char *cases[] = {"1+2*3",   "2^3^2",   "-2^2",
                           "(8-3)-2", "8-(3-2)", "1+2147483647"};
    for (unsigned i = 0; i < sizeof cases / sizeof *cases; i++)
    {
        struct pd_tree a, b;
        CHECK(expression(cases[i], &a) == PD_OK &&
              tc_expression_doc(&a, &d) == TC_OK);
        for (unsigned width = 0; width < 24; width++)
        {
            CHECK(tc_layout(&d, width, 20000, &out) == TC_OK &&
                  out.length < 256);
            out.bytes[out.length] = 0;
            CHECK(expression((const char *)out.bytes, &b) == PD_OK &&
                  same_tree(&a, &b));
        }
    }
    memset(&out, 0x3c, sizeof out);
    struct tc_text old = out;
    CHECK(tc_layout(&d, 8, 0, &out) == TC_FUEL &&
          memcmp(&out, &old, sizeof out) == 0);
    d.nodes[0].kind = TD_CAT;
    d.nodes[0].left = 0;
    CHECK(tc_layout(&d, 8, 100, &out) == TC_INVALID &&
          memcmp(&out, &old, sizeof out) == 0);
    memset(&d, 0, sizeof d);
    d.length = 512;
    memset(d.text, 'x', 512);
    d.nodes[0] = (struct tc_node){TD_TEXT, 0, 0, 0, 0, 512};
    d.count = 5;
    d.root = 4;
    for (unsigned i = 1; i < 5; i++)
    {
        d.nodes[i] = (struct tc_node){TD_CAT, i - 1, i - 1, 0, 0, 0};
    }
    CHECK(tc_layout(&d, 120, 10000, &out) == TC_RANGE &&
          memcmp(&out, &old, sizeof out) == 0);
    CHECK(tc_layout(&d, 120, 2, &out) == TC_FUEL &&
          memcmp(&out, &old, sizeof out) == 0);
    struct pd_tree too_large = {0};
    too_large.count = 15;
    too_large.root = 14;
    struct tc_doc before = d;
    CHECK(tc_expression_doc(&too_large, &d) == TC_RANGE &&
          memcmp(&d, &before, sizeof d) == 0);
    printf("COST typed_doc=%zu text=%zu tree=%zu\n", sizeof(struct tc_doc),
           sizeof(struct tc_text), sizeof(struct pd_tree));
    puts("OBS layout");
    return 0;
}

static void set(struct tc_value *v, const char *s)
{
    v->length = strlen(s);
    memcpy(v->bytes, s, v->length);
}

static int templating(void)
{
    struct tc_program bank[4];
    struct tc_model m = {0};
    struct tc_text out;
    m.fields[0].count = 2;
    set(&m.fields[0].values[0], "<tag>&");
    set(&m.fields[0].values[1], "B");
    m.fields[1].count = 1;
    set(&m.fields[1].values[0], "yes");
    struct tc_model old = m;
    const char *src = "[{*a}{.},{/}]{?b}{>1}{/}";
    CHECK(tc_template_compile((const unsigned char *)src, strlen(src), bank) ==
          TC_OK);
    CHECK(tc_template_compile((const unsigned char *)"{b}", 3, bank + 1) ==
          TC_OK);
    CHECK(tc_template_render(bank, 2, 0, &m, 1, 1000, &out) == TC_OK);
    const char *expected = "[&lt;tag&gt;&amp;,B,]yes";
    CHECK(out.length == strlen(expected) &&
          memcmp(out.bytes, expected, out.length) == 0 &&
          memcmp(&m, &old, sizeof m) == 0);
    CHECK(tc_template_render(bank, 2, 0, &m, 0, 1000, &out) == TC_OK &&
          out.length == strlen("[<tag>&,B,]yes"));
    CHECK(tc_template_compile((const unsigned char *)"{{a}}", 5, bank) ==
              TC_OK &&
          tc_template_render(bank, 1, 0, &m, 1, 100, &out) == TC_OK &&
          out.length == 3 && memcmp(out.bytes, "{a}", 3) == 0);
    CHECK(tc_template_compile((const unsigned char *)"{>0}", 4, bank) == TC_OK);
    memset(&out, 0x71, sizeof out);
    struct tc_text saved = out;
    CHECK(tc_template_render(bank, 1, 0, &m, 1, 100, &out) == TC_CYCLE &&
          memcmp(&out, &saved, sizeof out) == 0);
    CHECK(tc_template_compile((const unsigned char *)"{?c}{>0}{/}", 11, bank) ==
              TC_OK &&
          tc_template_render(bank, 1, 0, &m, 1, 100, &out) == TC_OK &&
          out.length == 0);
    CHECK(tc_template_compile((const unsigned char *)"{a}", 3, bank) == TC_OK);
    out = saved;
    CHECK(tc_template_render(bank, 1, 0, &m, 1, 1, &out) == TC_FUEL &&
          memcmp(&out, &saved, sizeof out) == 0);
    struct tc_program p = bank[0];
    CHECK(tc_template_compile((const unsigned char *)"{?a}", 4, bank) ==
              TC_SYNTAX &&
          memcmp(bank, &p, sizeof p) == 0);
    CHECK(tc_template_compile((const unsigned char *)"{a*2}", 5, bank) ==
          TC_SYNTAX);
    CHECK(tc_template_compile((const unsigned char *)"{/}", 3, bank) ==
          TC_SYNTAX);
    unsigned char large[129];
    memset(large, 'x', sizeof large);
    CHECK(tc_template_compile(large, sizeof large, bank) == TC_RANGE);
    src = "{*a}{*a}{*a}{.}{/}{/}{/}";
    CHECK(tc_template_compile((const unsigned char *)src, strlen(src), bank) ==
          TC_OK);
    m.fields[0].count = 4;
    for (unsigned i = 0; i < 4; i++)
    {
        memset(m.fields[0].values[i].bytes, 'x', 256);
        m.fields[0].values[i].length = 256;
    }
    out = saved;
    CHECK(tc_template_render(bank, 1, 0, &m, 0, 100000, &out) == TC_RANGE &&
          memcmp(&out, &saved, sizeof out) == 0);
    for (unsigned program = 0; program < 4; program++)
    {
        unsigned char deep[256];
        size_t n = 0;
        for (unsigned i = 0; i < 36; i++)
        {
            memcpy(deep + n, "{?a}", 4);
            n += 4;
        }
        if (program < 3)
        {
            deep[n++] = '{';
            deep[n++] = '>';
            deep[n++] = (unsigned char)('1' + program);
            deep[n++] = '}';
        }
        else
        {
            memcpy(deep + n, "{a}", 3);
            n += 3;
        }
        for (unsigned i = 0; i < 36; i++)
        {
            memcpy(deep + n, "{/}", 3);
            n += 3;
        }
        CHECK(tc_template_compile(deep, n, bank + program) == TC_OK);
    }
    CHECK(tc_template_render(bank, 4, 0, &m, 0, 100000, &out) == TC_RANGE &&
          memcmp(&out, &saved, sizeof out) == 0);
    printf("COST model=%zu program=%zu bank4=%zu\n", sizeof(struct tc_model),
           sizeof(struct tc_program), sizeof bank);
    puts("OBS template");
    return 0;
}
#ifndef TC_NO_MAIN
int main(int argc, char **argv)
{
    if (argc != 2)
    {
        return 2;
    }
    unsigned c = (unsigned)strtoul(argv[1], NULL, 10);
    return c == 1 ? recovery() : c == 2 ? layout() : c == 3 ? templating() : 2;
}
#endif
