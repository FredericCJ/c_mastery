#include "adt.h"
static unsigned category(struct ad_brand x){return x.index;}
int main(void){struct ad_pool p;struct ad_brand h={&p,2,1};
#if defined(WRONG)
 struct ad_node_index node={2};return (int)category(node);
#elif defined(REBUILD)
 struct ad_brand rebuilt={h.owner,h.index,h.generation};return category(rebuilt)==2?0:1;
#else
 struct ad_brand copied=h;return category(copied)==2?0:1;
#endif
}
