#include "grammars.h"
#include "parsing.h"
#include <inttypes.h>
#include <stdio.h>

static int bytes(unsigned char *b, unsigned n)
{
    for (unsigned i = 0; i < n; ++i)
    {
        unsigned c;
        if (scanf("%u", &c) != 1 || c > 255)
            return 0;
        b[i] = (unsigned char)c;
    }
    return 1;
}

int main(void)
{
    char op;
    while (scanf(" %c", &op) == 1)
    {
        unsigned char input[257];
        unsigned n;
        struct pd_diagnostic diag;
        if (op == 'R')
        {
            unsigned plen, cap;
            unsigned char pattern[25];
            if (scanf("%u", &plen) != 1 || plen > 24 || !bytes(pattern, plen) ||
                scanf("%u", &n) != 1 || n > 256 || !bytes(input, n) ||
                scanf("%u", &cap) != 1)
                return 2;
            struct rx_nfa a;
            struct rx_dfa d;
            unsigned whole = 9, search = 9, dfa = 9;
            int rc = rx_compile(pattern, plen, &a, &diag), dc = 9;
            if (!rc)
            {
                (void)rx_match(&a, input, n, 0, &whole);
                (void)rx_match(&a, input, n, 1, &search);
                dc = rx_determinize(&a, cap, &d);
                if (!dc)
                    (void)rx_dfa_match(&d, input, n, &dfa);
            }
            printf("R %d %u %u %d %u\n", rc, whole, search, dc, dfa);
        }
        else if (op == 'P')
        {
            unsigned mode;
            if (scanf("%u %u", &mode, &n) != 2 || n > 256 || !bytes(input, n))
                return 2;
            struct pd_tokens tokens;
            struct pd_tree tree;
            int32_t value = 123;
            int lex = pd_lex(input, n, &tokens, &diag), parse = 9, eval = 9;
            if (!lex)
            {
                parse = pd_pratt(&tokens, mode, &tree, &diag);
                if (!parse)
                    eval = pd_evaluate(&tree, &value);
            }
            printf("P %d %d %d %" PRId32, lex, parse, eval, value);
            if (!lex && !parse)
            {
                printf(" %u", tree.count);
                for (unsigned i = 0; i < tree.count; ++i)
                {
                    struct pd_expr x = tree.nodes[i];
                    printf(" %u %u %u %" PRId32 " %zu", x.kind, x.left, x.right,
                           x.value, x.offset);
                }
            }
            puts("");
        }
        else if (op == 'L')
        {
            unsigned mode, budget;
            if (scanf("%u %u", &mode, &n) != 2 || n > 64 || !bytes(input, n) ||
                scanf("%u", &budget) != 1)
                return 2;
            struct lr_grammar g = pd_grammar(mode);
            struct lr_table table;
            struct lr_tree tree;
            int rc = lr_build(&g, &table, &diag), pc = 9;
            if (!rc)
                pc = lr_parse(&table, input, n, budget, &tree, &diag);
            printf("L %d %d", rc, pc);
            if (!rc && !pc)
            {
                printf(" %u %u", tree.count, tree.root);
                for (unsigned i = 0; i < tree.count; ++i)
                {
                    struct lr_node x = tree.nodes[i];
                    printf(" %u %u %u %zu %zu", x.symbol, x.production, x.count,
                           x.begin, x.end);
                    for (unsigned j = 0; j < x.count; ++j)
                        printf(" %u", x.children[j]);
                }
            }
            puts("");
        }
        else if (op == 'S')
        {
            unsigned count, eof, budget;
            struct ps_node nodes[64];
            if (scanf("%u", &count) != 1 || count > 64)
                return 2;
            for (unsigned i = 0; i < count; ++i)
                if (scanf("%u %u %u %u %u", &nodes[i].kind, &nodes[i].left,
                          &nodes[i].right, &nodes[i].low, &nodes[i].high) != 5)
                    return 2;
            if (scanf("%u", &n) != 1 || n > 256 || !bytes(input, n) ||
                scanf("%u %u", &eof, &budget) != 2)
                return 2;
            struct ps_result result = {.end = 999, .count = 0};
            int rc =
                ps_parse(nodes, count, input, n, eof, budget, &result, &diag);
            printf("S %d %zu %u", rc, result.end, result.count);
            for (unsigned i = 0; i < result.count; ++i)
                printf(" %u %zu %zu", result.captures[i].tag,
                       result.captures[i].begin, result.captures[i].end);
            puts("");
        }
        else
            return 2;
    }
    return 0;
}
