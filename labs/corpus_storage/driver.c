#include "storage.h"
#include <stdio.h>
#include <stdlib.h>

int main(void)
{
    struct ss_skip s;
    struct ss_tree t;
    struct ss_lsm l;
    struct ss_version versions[7] = {{0}};
    ss_skip_init(&s);
    ss_tree_init(&t);
    ss_lsm_init(&l);
    char family;
    unsigned op, a, b, c;
    while (scanf(" %c %u %u %u %u", &family, &op, &a, &b, &c) == 5)
    {
        int rc = SS_INVALID;
        unsigned cost = 0;
        if (family == 'S')
        {
            if (op == 0)
                rc = ss_skip_put(&s, a, (int32_t)b, c, 0);
            else if (op == 1)
                rc = ss_skip_erase(&s, a);
            else if (op == 9)
            {
                ss_skip_init(&s);
                rc = SS_OK;
            }
        }
        else if (family == 'T')
        {
            if (op == 0)
                rc = ss_tree_put(&t, a, (int32_t)b, c);
            else if (op == 1)
                rc = ss_tree_erase(&t, a, c);
            else if (op == 2 && a < 7)
                rc = ss_tree_snapshot(&t, &versions[a]);
            else if (op == 3 && a < 7)
                rc = ss_tree_release(&t, versions[a]);
            else if (op == 4)
                rc = ss_tree_collect(&t, &cost);
            else if (op == 5)
                rc = ss_tree_vacuum(&t, c);
            else if (op == 9)
            {
                ss_tree_init(&t);
                for (unsigned i = 0; i < 7; ++i)
                    versions[i] = (struct ss_version){0};
                rc = SS_OK;
            }
        }
        else if (family == 'L')
        {
            if (op == 0)
                rc = ss_lsm_write(&l, a, (int32_t)b, c);
            else if (op == 1)
                rc = ss_lsm_flush(&l, c);
            else if (op == 2)
                rc = ss_lsm_compact(&l, a, b, c);
            else if (op == 9)
            {
                ss_lsm_init(&l);
                rc = SS_OK;
            }
        }
        printf("%d", rc);
        for (unsigned key = 0; key < 32; ++key)
        {
            int32_t value = 0;
            int result = family == 'S'   ? ss_skip_get(&s, key, &value, &cost)
                         : family == 'T' ? ss_tree_get(&t, NULL, key, &value)
                                         : ss_lsm_get(&l, key, &value, &cost);
            printf(" %d:%d", result, (int)value);
        }
        if (family == 'T')
            for (unsigned v = 0; v < 7; ++v)
            {
                for (unsigned key = 0; key < 32; ++key)
                {
                    int32_t value = 0;
                    int result = ss_tree_get(&t, &versions[v], key, &value);
                    printf(" %d:%d", result, (int)value);
                }
            }
        putchar('\n');
    }
    return 0;
}
