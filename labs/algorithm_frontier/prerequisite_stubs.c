/* Import actual completed learner producers instead of these link-only stubs. */
#include "frontier.h"
enum CDStatus cd_csr_build(CDCSR*g,unsigned n,const CDEdge*e,size_t m,unsigned p){(void)g;(void)n;(void)e;(void)m;(void)p;return CD_INVALID;}
enum ADStatus ad_bfs(const CDCSR*g,unsigned s,unsigned f,unsigned*q,size_t n,ADTraversal*t){(void)g;(void)s;(void)f;(void)q;(void)n;(void)t;return AD_INVALID;}
enum ADStatus ad_dijkstra(const CDCSR*g,unsigned s,uint64_t l,uint64_t d[8],unsigned p[8]){(void)g;(void)s;(void)l;(void)d;(void)p;return AD_INVALID;}
