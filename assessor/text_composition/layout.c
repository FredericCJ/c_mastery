#include "text.h"
#ifndef TC_MUTANT
#define TC_MUTANT 0
#endif
struct tc_frame
{
    unsigned node, indent, flat;
};

static int valid(const struct tc_doc *d)
{
    if (!d || !d->count || d->count > 128 || d->root >= d->count ||
        d->length > 512)
        return 0;
    for (unsigned i = 0; i < d->count; i++)
    {
        const struct tc_node *n = d->nodes + i;
        if (n->kind > TD_GROUP)
            return 0;
        if ((n->kind == TD_CAT || n->kind == TD_NEST || n->kind == TD_GROUP) &&
            n->left >= i)
            return 0;
        if (n->kind == TD_CAT && n->right >= i)
            return 0;
        if (n->kind == TD_NEST && n->value > 120)
            return 0;
        if (n->kind == TD_TEXT)
        {
            if (n->offset > d->length || n->length > d->length - n->offset)
                return 0;
            for (size_t j = 0; j < n->length; j++)
                if (d->text[n->offset + j] < 32 || d->text[n->offset + j] > 126)
                    return 0;
        }
    }
    return 1;
}

static int tick(unsigned *fuel)
{
    if (!*fuel)
        return 0;
    --*fuel;
    return 1;
}

static int fits(const struct tc_doc *d, struct tc_frame *stack, unsigned count,
                unsigned column, unsigned width, unsigned *fuel)
{
    while (count)
    {
        if (!tick(fuel))
            return -1;
        struct tc_frame f = stack[--count];
        const struct tc_node *n = d->nodes + f.node;
        if (column > width)
            return 0;
        switch (n->kind)
        {
            case TD_NIL:
                break;
            case TD_TEXT:
                if (n->length > width - column)
                    return 0;
                column += (unsigned)n->length;
                break;
            case TD_LINE:
                if (!f.flat)
                    return 1;
                if (column == width)
                    return 0;
                ++column;
                break;
            case TD_CAT:
                if (count + 2 > 256)
                    return -2;
                stack[count++] = (struct tc_frame){n->right, f.indent, f.flat};
                stack[count++] = (struct tc_frame){n->left, f.indent, f.flat};
                break;
            case TD_NEST:
            case TD_GROUP:
                if (count == 256)
                    return -2;
                stack[count++] = (struct tc_frame){n->left, f.indent, f.flat};
                break;
        }
    }
    return column <= width;
}

static int emit(struct tc_text *out, unsigned char ch)
{
    if (out->length == 4096)
        return 0;
    out->bytes[out->length++] = ch;
    return 1;
}

int tc_layout(const struct tc_doc *d, unsigned width, unsigned budget,
              struct tc_text *out)
{
    if (!out || !valid(d))
        return TC_INVALID;
    if (width > 120)
        return TC_RANGE;
    struct tc_frame stack[256], probe[256];
    unsigned count = 1, column = 0;
    stack[0] = (struct tc_frame){d->root, 0, 0};
    struct tc_text result = {0};
    while (count)
    {
        if (!tick(&budget))
            return TC_FUEL;
        struct tc_frame f = stack[--count];
        const struct tc_node *n = d->nodes + f.node;
        switch (n->kind)
        {
            case TD_NIL:
                break;
            case TD_TEXT:
                for (size_t i = 0; i < n->length; i++)
                    if (!emit(&result, d->text[n->offset + i]))
                        return TC_RANGE;
                column += (unsigned)n->length;
                break;
            case TD_LINE:
                if (f.flat)
                {
                    if (!emit(&result, ' '))
                        return TC_RANGE;
                    ++column;
                }
                else
                {
                    if (!emit(&result, '\n'))
                        return TC_RANGE;
                    for (unsigned i = 0; i < f.indent; i++)
                        if (!emit(&result, ' '))
                            return TC_RANGE;
                    column = f.indent;
                }
                break;
            case TD_CAT:
                if (count + 2 > 256)
                    return TC_RANGE;
                stack[count++] = (struct tc_frame){n->right, f.indent, f.flat};
                stack[count++] = (struct tc_frame){n->left, f.indent, f.flat};
                break;
            case TD_NEST:
                if (f.indent > 120 - n->value)
                    return TC_RANGE;
                if (count == 256)
                    return TC_RANGE;
                stack[count++] = (struct tc_frame){
                    n->left, TC_MUTANT == 4 ? f.indent : f.indent + n->value,
                    f.flat};
                break;
            case TD_GROUP:
            {
                unsigned flat = f.flat;
                if (!flat)
                {
                    unsigned k = TC_MUTANT == 3 ? 0 : count;
                    for (unsigned i = 0; i < k; i++)
                        probe[i] = stack[i];
                    if (k == 256)
                        return TC_RANGE;
                    probe[k++] = (struct tc_frame){n->left, f.indent, 1};
                    int fit = fits(d, probe, k, column, width, &budget);
                    if (fit < 0)
                        return fit == -1 ? TC_FUEL : TC_RANGE;
                    flat = (unsigned)fit;
                }
                if (count == 256)
                    return TC_RANGE;
                stack[count++] = (struct tc_frame){n->left, f.indent, flat};
                break;
            }
        }
    }
    *out = result;
    return TC_OK;
}

static unsigned node(struct tc_doc *d, unsigned kind, unsigned left,
                     unsigned right, unsigned value)
{
    if (d->count == 128)
        return 128;
    unsigned k = d->count++;
    d->nodes[k] = (struct tc_node){kind, left, right, value, 0, 0};
    return k;
}

static unsigned text(struct tc_doc *d, const unsigned char *s, size_t n)
{
    if (n > 512 - d->length)
        return 128;
    unsigned k = node(d, TD_TEXT, 0, 0, 0);
    if (k == 128)
        return k;
    d->nodes[k].offset = d->length;
    d->nodes[k].length = n;
    for (size_t i = 0; i < n; i++)
        d->text[d->length++] = s[i];
    return k;
}

int tc_expression_doc(const struct pd_tree *tree, struct tc_doc *out)
{
    if (!tree || !out || !tree->count || tree->root >= tree->count)
        return TC_INVALID;
    if (tree->count > 14)
        return TC_RANGE;
    struct tc_doc d = {0};
    unsigned roots[14];
    for (unsigned i = 0; i < tree->count; i++)
    {
        const struct pd_expr *n = tree->nodes + i;
        unsigned r = 128;
        if (n->kind == 0)
        {
            unsigned char digits[11], s[11];
            size_t len = 0;
            int64_t v = n->value;
            if (v < 0)
                return TC_INVALID;
            do
            {
                digits[len++] = (unsigned char)('0' + v % 10);
                v /= 10;
            } while (v);
            for (size_t j = 0; j < len; j++)
                s[j] = digits[len - 1 - j];
            r = text(&d, s, len);
        }
        else
        {
            if (n->left >= i || (n->kind != 5 && n->right >= i))
                return TC_INVALID;
            if (n->kind != 1 && n->kind != 2 && n->kind != 5 && n->kind != 6 &&
                n->kind != 8)
                return TC_INVALID;
            unsigned open = text(&d, (const unsigned char *)"(", 1),
                     close = text(&d, (const unsigned char *)")", 1);
            if (n->kind == 5)
            {
                unsigned minus = text(&d, (const unsigned char *)"-", 1);
                r = node(&d, TD_CAT, minus, roots[n->left], 0);
            }
            else
            {
                unsigned char op = n->kind == 1   ? '+'
                                   : n->kind == 2 ? '*'
                                   : n->kind == 6 ? '^'
                                                  : '-';
                unsigned line = node(&d, TD_LINE, 0, 0, 0),
                         token = text(&d, &op, 1),
                         space = text(&d, (const unsigned char *)" ", 1);
                r = node(&d, TD_CAT, space, roots[n->right], 0);
                r = node(&d, TD_CAT, token, r, 0);
                r = node(&d, TD_CAT, line, r, 0);
                r = node(&d, TD_CAT, roots[n->left], r, 0);
            }
            r = node(&d, TD_NEST, r, 0, 2);
            r = node(&d, TD_CAT, open, r, 0);
            r = node(&d, TD_CAT, r, close, 0);
            r = node(&d, TD_GROUP, r, 0, 0);
        }
        if (r == 128)
            return TC_RANGE;
        roots[i] = r;
    }
    d.root = roots[tree->root];
    *out = d;
    return TC_OK;
}
