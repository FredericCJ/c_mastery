#include "adt.h"
static unsigned category(ADBrand x){return x.index;}
int main(void){ADPool p;ADBrand h={&p,2,1};
#if defined(WRONG)
 ADNodeIndex node={2};return (int)category(node);
#elif defined(REBUILD)
 ADBrand rebuilt={h.owner,h.index,h.generation};return category(rebuilt)==2?0:1;
#else
 ADBrand copied=h;return category(copied)==2?0:1;
#endif
}
