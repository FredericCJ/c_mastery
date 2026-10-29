#include "text.h"
#include <stdio.h>
#include <string.h>

static size_t decode(const char *hex, unsigned char *bytes)
{
    if (!strcmp(hex, "-"))
    {
        return 0;
    }
    size_t n = strlen(hex) / 2;
    for (size_t i = 0; i < n; i++)
    {
        unsigned x = 0;
        (void)sscanf(hex + 2 * i, "%2x", &x);
        bytes[i] = (unsigned char)x;
    }
    return n;
}

static void result(int status, const struct tc_text *text)
{
    printf("%d %zu ", status, status == 0 ? text->length : 0);
    if (status == 0 && text->length)
    {
        for (size_t i = 0; i < text->length; i++)
        {
            printf("%02x", text->bytes[i]);
        }
    }
    else
    {
        printf("-");
    }
    puts("");
}

static void fixture(struct tc_doc *d, unsigned a, unsigned b, unsigned tail,
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

int main(void)
{
    char mode, hex[2049], inc[2049], v[2049];
    unsigned p, q;
    while (scanf(" %c %u %u", &mode, &p, &q) == 3)
    {
        if (mode == 'R')
        {
            if (scanf("%2048s", hex) != 1)
            {
                return 2;
            }
            unsigned char bytes[1024];
            size_t n = decode(hex, bytes);
            struct tc_batch b;
            int s = tc_recover(bytes, n, p, q, &b);
            printf("%d", s);
            if (s == TC_OK || s == TC_RECOVERED)
            {
                printf(" %u %u", b.count, b.errors);
                for (unsigned i = 0; i < b.count; i++)
                {
                    printf(" %ld:%zu", (long)b.values[i], b.begins[i]);
                }
                for (unsigned i = 0; i < b.errors; i++)
                {
                    printf(" %zu:%zu:%d", b.diagnostics[i].begin,
                           b.diagnostics[i].end, b.diagnostics[i].status);
                }
            }
            puts("");
        }
        else if (mode == 'L')
        {
            unsigned a, b, tail, indent;
            if (scanf("%u %u %u %u", &a, &b, &tail, &indent) != 4 ||
                a + b + tail > 512 || indent > 120)
            {
                return 2;
            }
            struct tc_doc d;
            struct tc_text out;
            fixture(&d, a, b, tail, indent);
            int s = tc_layout(&d, p, q, &out);
            result(s, &out);
        }
        else if (mode == 'T')
        {
            unsigned count, presence;
            if (scanf("%2048s %2048s %u %u", hex, inc, &count, &presence) !=
                    4 ||
                count > 4)
            {
                return 2;
            }
            struct tc_program bank[2];
            struct tc_model m = {0};
            unsigned char bytes[1024];
            size_t n = decode(hex, bytes);
            int s = tc_template_compile(bytes, n, bank);
            n = decode(inc, bytes);
            int t = tc_template_compile(bytes, n, bank + 1);
            if (!s)
            {
                s = t;
            }
            m.fields[0].count = count;
            for (unsigned i = 0; i < count; i++)
            {
                if (scanf("%2048s", v) != 1)
                {
                    return 2;
                }
                n = decode(v, bytes);
                if (n > 256)
                {
                    return 2;
                }
                memcpy(m.fields[0].values[i].bytes, bytes, n);
                m.fields[0].values[i].length = n;
            }
            if (presence)
            {
                m.fields[1].count = 1;
                m.fields[1].values[0].bytes[0] = 'B';
                m.fields[1].values[0].length = 1;
            }
            struct tc_text out;
            if (!s)
            {
                s = tc_template_render(bank, 2, 0, &m, p, q, &out);
            }
            result(s, &out);
        }
        else
        {
            return 2;
        }
    }
    return 0;
}
