#include "text.h"
#ifndef TC_MUTANT
#define TC_MUTANT 0
#endif
static int white(unsigned char c)
{
    return c == ' ' || c == '\t' || c == '\n';
}

int tc_recover(const unsigned char *input, size_t length, unsigned newline_sync,
               unsigned budget, struct tc_batch *out)
{
    if (!out || (!input && length) || newline_sync > 1)
    {
        return TC_INVALID;
    }
    if (length > 512)
    {
        return TC_RANGE;
    }
    if (budget <= length)
    {
        return TC_FUEL;
    }
    struct tc_batch result = {0};
    size_t begin = 0;
    for (size_t i = 0; i <= length; i++)
    {
        if (budget == 0)
        {
            return TC_FUEL;
        }
        --budget;
        if (i != length && input[i] != ';' &&
            !(newline_sync && input[i] == '\n'))
        {
            continue;
        }
        size_t start = begin, end = i;
        while (start < end && white(input[start]))
        {
            start++;
        }
        while (end > start && white(input[end - 1]))
        {
            end--;
        }
        if (start == end && i == length)
        {
            break;
        }
        struct pd_tokens tokens;
        struct pd_tree tree;
        struct pd_diagnostic diagnostic = {0};
        int32_t value = 0;
        int status = PD_NO_MATCH;
        if (i == length)
        {
            status = TC_SYNTAX + 100;
            diagnostic.offset = end - start;
        }
        else if (end - start > 256)
        {
            status = PD_RANGE;
        }
        else if (start < end)
        {
            status = pd_lex(input + start, end - start, &tokens, &diagnostic);
            if (status == PD_OK)
            {
                status = pd_pratt(&tokens, 1, &tree, &diagnostic);
            }
            if (status == PD_OK)
            {
                status = pd_evaluate(&tree, &value);
                if (TC_MUTANT == 2)
                {
                    status = PD_OK;
                }
            }
        }
        if (status == PD_OK)
        {
            if (result.count == 16)
            {
                return TC_RANGE;
            }
            result.begins[result.count] = start;
            result.values[result.count++] = value;
        }
        else
        {
            if (result.errors == 16)
            {
                return TC_RANGE;
            }
            result.diagnostics[result.errors++] =
                (struct tc_error){start, i, start + diagnostic.offset, status};
        }
        begin = i + 1;
        if (TC_MUTANT == 1 && status != PD_OK && begin < length)
        {
            ++begin;
        }
    }
    *out = result;
    return result.errors ? TC_RECOVERED : TC_OK;
}
