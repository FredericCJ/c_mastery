#include "adt.h"

int probe(struct ad_pool *pool)
{
    struct ad_node_index index = {0};
    uint32_t out;
    return ad_pool_get(pool, index, &out);
}
