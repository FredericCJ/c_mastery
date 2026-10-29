#include "parser_follow.h"
#include <stdio.h>
#include <string.h>
#define C(x)                                                   \
    do                                                         \
    {                                                          \
        if (!(x))                                              \
        {                                                      \
            fprintf(stderr, "CP_CHECK:%d:%s\n", __LINE__, #x); \
            return 1;                                          \
        }                                                      \
    } while (0)

static int cut(void)
{
    /* ('a' cut then 'b') / ('a' then 'c'); cut is a wrapper around the first literal. */
    struct cp_node n[] = {{CP_CHOICE, 1, 2, 0},    {CP_SEQUENCE, 3, 4, 0},
                          {CP_SEQUENCE, 5, 6, 0},  {CP_CUT, 7, 0, 0},
                          {CP_LITERAL, 0, 0, 'b'}, {CP_LITERAL, 0, 0, 'a'},
                          {CP_LITERAL, 0, 0, 'c'}, {CP_LITERAL, 0, 0, 'a'}};
    const unsigned char input[] = "ac";
    size_t end = 91;
    struct cp_diagnostic d;
    C(cp_parse(n, 8, input, 2, 1, 100, &end, &d) == CP_COMMITTED && end == 91 &&
      d.offset == 1 && d.count == 1 && d.expected[0] == 'b' && d.committed);
    for (size_t available = 0; available < 2; available++)
    {
        C(cp_parse(n, 8, input, available, 0, 100, &end, &d) == CO_PENDING &&
          end == 91 && d.offset == available);
    }
    C(cp_parse(n, 8, (const unsigned char *)"ab", 2, 1, 100, &end, &d) ==
          CO_OK &&
      end == 2);
    end = 91;
    C(cp_parse(n, 8, input, 1, 1, 100, &end, &d) == CP_COMMITTED && end == 91);
    n[3] = (struct cp_node){CP_LITERAL, 0, 0, 'a'};
    C(cp_parse(n, 8, input, 2, 1, 100, &end, &d) == CO_OK && end == 2);
    struct cp_node choices[] = {{CP_CHOICE, 1, 2, 0},   {CP_LITERAL, 0, 0, 'z'},
                                {CP_CHOICE, 3, 4, 0},   {CP_LITERAL, 0, 0, 'a'},
                                {CP_CHOICE, 5, 6, 0},   {CP_LITERAL, 0, 0, 'b'},
                                {CP_CHOICE, 7, 8, 0},   {CP_LITERAL, 0, 0, 'c'},
                                {CP_LITERAL, 0, 0, 'd'}};
    end = 91;
    C(cp_parse(choices, 9, (const unsigned char *)"x", 1, 1, 100, &end, &d) ==
          CO_NO_MATCH &&
      end == 91 && d.count == 4 && d.truncated &&
      !memcmp(d.expected, "abcd", 4));
    C(cp_parse(choices, 9, input, 1, 1, 0, &end, &d) == CO_RANGE &&
      d.steps == 0 && end == 91);
    return 0;
}

static int memo(void)
{
    struct ce_memo m = {0};
    unsigned char in[] = "1+2+3";
    size_t end = 91;
    int32_t value = 92;
    unsigned calls;
    C(cp_left_expression(&m, in, 5, 1, 1, 1023, 64, &end, &value, &calls) ==
          0 &&
      end == 5 && value == 6 && calls == 4);
    C(cp_left_expression(&m, in, 5, 1, 1, 1023, 0, &end, &value, &calls) == 0 &&
      calls == 0);
    end = 91;
    value = 92;
    C(cp_left_expression(&m, in, 5, 1, 2, 1023 ^ (1u << 2), 64, &end, &value,
                         &calls) == CO_NO_MATCH &&
      end == 91 && value == 92);
    C(cp_left_expression(&m, in, 5, 1, 3, 1023, 64, &end, &value, &calls) ==
          0 &&
      value == 6);
    in[2] = '4';
    C(cp_left_expression(&m, in, 5, 2, 3, 1023, 64, &end, &value, &calls) ==
          0 &&
      value == 8);
    m = (struct ce_memo){0};
    end = 91;
    value = 92;
    C(cp_left_expression(&m, in, 5, 2, 3, 1023, 1, &end, &value, &calls) ==
          CO_RANGE &&
      !m.valid && end == 91 && value == 92);
    const unsigned char alphabet[] = "01+x";
    unsigned power = 1, total = 0;
    for (size_t n = 0; n <= 7; n++, power *= 4)
    {
        for (unsigned code = 0; code < power; code++)
        {
            unsigned char text[8];
            unsigned v = code;
            int expected = n % 2 == 1;
            int32_t sum = 0;
            for (size_t i = 0; i < n; i++)
            {
                text[i] = alphabet[v % 4];
                v /= 4;
                if (i % 2)
                {
                    expected &= text[i] == '+';
                }
                else
                {
                    expected &= text[i] == '0' || text[i] == '1';
                    if (text[i] == '1')
                    {
                        sum++;
                    }
                }
            }
            m = (struct ce_memo){0};
            end = 91;
            value = 92;
            int r = cp_left_expression(&m, text, n, 1, 1, 1023, 64, &end,
                                       &value, &calls);
            C(r == (expected ? CO_OK : CO_NO_MATCH));
            if (expected)
            {
                C(end == n && value == sum);
            }
            else
            {
                C(end == 91 && value == 92);
            }
            C(calls <= n / 2 + 2);
            total++;
        }
    }
    printf("CP_MEMO_CASES:%u\n", total);
    return 0;
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        return 2;
    }
    if (!strcmp(argv[1], "cut"))
    {
        return cut();
    }
    if (!strcmp(argv[1], "memo"))
    {
        return memo();
    }
    return 2;
}
