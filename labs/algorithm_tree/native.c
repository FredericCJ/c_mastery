#include "tree.h"
#include <stdio.h>
#include <string.h>

int at_failure(const char *s, unsigned line)
{
    fprintf(stderr, "AT_ASSERT:%u:%s\n", line, s);
    return 41;
}

void at_emit(const char *kind, const struct at_certificate *c,
             const struct at_work *w)
{
    printf("TREE %s %d %u %u %u\n", kind, c->root, c->count,
           w ? w->rotations : 0, w ? w->repair_count : 0);
    for (unsigned i = 0; i < c->capacity; ++i)
    {
        if (c->nodes[i].live)
        {
            struct at_node n = c->nodes[i];
            printf("NODE %u %d %d %d %d %d\n", i, n.key, n.left, n.right,
                   n.parent, n.height);
        }
    }
    puts("ENDTREE");
}

int main(int argc, char **argv)
{
    const char *names[] = {"rotations", "prior", "reject", "policies", "costs"};
    if (argc != 2)
    {
        return 2;
    }
    for (unsigned i = 0; i < 5; ++i)
    {
        if (!strcmp(argv[1], names[i]))
        {
            int r = at_test(i);
            if (!r)
            {
                printf("OBS %s\n", names[i]);
            }
            return r;
        }
    }
    return 2;
}
