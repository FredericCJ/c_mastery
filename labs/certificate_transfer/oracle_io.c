/* Host-side transport only; independent expectations live in assessor tooling. */
#include "transfer.h"
#include <stdio.h>

int main(void)
{
    unsigned policy, capacity, count;
    int root;
    while (scanf("%u%u%u%d", &policy, &capacity, &count, &root) == 4)
    {
        struct at_certificate c = {
            .root = root, .capacity = capacity, .count = count};
        struct ct_workspace w;
        int height = 777;
        if (capacity > 64)
        {
            return 2;
        }
        for (unsigned i = 0; i < capacity; ++i)
        {
            int key;
            if (scanf("%d%d%d%d%d%u%u", &key, &c.nodes[i].left,
                      &c.nodes[i].right, &c.nodes[i].parent, &c.nodes[i].height,
                      &c.nodes[i].live, &c.nodes[i].red) != 7)
            {
                return 2;
            }
            c.nodes[i].key = key;
        }
        int status = ct_tree(&c, policy, &w, 64, &height);
        printf("%d %d\n", status, height);
    }
    return 0;
}
