#include "model.h"
#include <stdio.h>
#include <string.h>

/* One process consumes a model batch via each frontend and the direct API.
 * Driver protocol uses bounded trusted oracle data, not the learner grammar. */
int main(void)
{
    struct cm_model model;
    cm_init(&model);
    char op;
    while (scanf(" %c", &op) == 1)
    {
        int rc = CM_INVALID;
        if (op == 'A')
        {
            unsigned count;
            struct cm_edit edits[16];
            if (scanf("%u", &count) != 1 || count > 16)
                return 2;
            for (unsigned i = 0; i < count; ++i)
            {
                struct cm_edit *e = &edits[i];
                int b, w0, w1;
                if (scanf("%u%u%u%u%d%d%d", &e->id, &e->kind, &e->a, &e->b, &w0,
                          &w1, &b) != 7)
                    return 2;
                e->w0 = w0;
                e->w1 = w1;
                e->bias = b;
            }
            rc = cm_apply(&model, edits, count);
        }
        else if (op == 'L' || op == 'C')
        {
            char text[1025];
            unsigned n;
            if (scanf("%u", &n) != 1 || n > 1024)
                return 2;
            (void)getchar();
            if (fread(text, 1, n, stdin) != n)
                return 2;
            rc = cm_load(&model, (const unsigned char *)text, n, op == 'C');
        }
        else if (op == 'E')
        {
            char expr[257];
            unsigned n;
            if (scanf("%u", &n) != 1 || n > 256)
                return 2;
            (void)getchar();
            if (fread(expr, 1, n, stdin) != n)
                return 2;
            int32_t value = 12345;
            rc = cm_eval(&model, (const unsigned char *)expr, n, &value);
            printf("E %d %ld\n", rc, (long)value);
            continue;
        }
        else if (op == 'R')
        {
            cm_init(&model);
            rc = CM_OK;
        }
        else
            return 2;
        printf("M %d %lu", rc, (unsigned long)model.revision);
        for (unsigned i = 0; i < 16; ++i)
            printf(" %u:%ld", model.nodes[i].kind, (long)model.nodes[i].value);
        putchar('\n');
    }
    return 0;
}
