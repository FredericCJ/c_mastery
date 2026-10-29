#include "money.h"
#include <stdio.h>
#include <inttypes.h>

static struct cm_money m(int64_t x)
{
    struct cm_money v = {x, {'U', 'S', 'D'}, 2};
    return v;
}

int main(void)
{
    char op;
    while (scanf(" %c", &op) == 1)
    {
        int64_t a;
        unsigned n, d, mode;
        struct cm_money result = m(73);
        enum cm_status s;
        if (op == 'R')
        {
            if (scanf("%" SCNd64 " %u %u %u", &a, &n, &d, &mode) != 4)
            {
                return 2;
            }
            s = cm_ratio(m(a), n, d, (enum cm_round)mode, &result);
            printf("%d %" PRId64 "\n", s, result.minor);
        }
        else if (op == 'A')
        {
            uint32_t w[CM_MAX];
            struct cm_money out[CM_MAX];
            if (scanf("%" SCNd64 " %u", &a, &n) != 2 || n > CM_MAX)
            {
                return 2;
            }
            for (unsigned i = 0; i < n; i++)
            {
                if (scanf("%" SCNu32, &w[i]) != 1)
                {
                    return 2;
                }
                out[i] = m(73);
            }
            s = cm_allocate(m(a), w, n, out);
            printf("%d", s);
            for (unsigned i = 0; i < n; i++)
            {
                printf(" %" PRId64, out[i].minor);
            }
            puts("");
        }
        else if (op == 'S')
        {
            struct cm_money accounts[CM_MAX];
            size_t dest[CM_MAX];
            uint32_t w[CM_MAX];
            unsigned k, cut;
            if (scanf("%" SCNd64 " %u %u %u", &a, &n, &k, &cut) != 4 ||
                n > CM_MAX || k >= n)
            {
                return 2;
            }
            for (unsigned i = 0; i < n; i++)
            {
                int64_t b;
                if (scanf("%" SCNd64, &b) != 1)
                {
                    return 2;
                }
                accounts[i] = m(b);
            }
            for (unsigned i = 0; i < k; i++)
            {
                if (scanf("%" SCNu32, &w[i]) != 1)
                {
                    return 2;
                }
                dest[i] = i + 1;
            }
            s = cm_settle(accounts, n, 0, dest, w, k, m(a),
                          cut == 99 ? SIZE_MAX : cut);
            printf("%d", s);
            for (unsigned i = 0; i < n; i++)
            {
                printf(" %" PRId64, accounts[i].minor);
            }
            puts("");
        }
        else
        {
            return 2;
        }
    }
    return 0;
}
