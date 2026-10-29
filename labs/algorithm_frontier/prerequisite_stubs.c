/* Import actual completed learner producers instead of these link-only stubs. */
#include "frontier.h"
enum cd_status cd_csr_build(struct cd_csr*g,unsigned n,const struct cd_edge*e,size_t m,unsigned p){(void)g;(void)n;(void)e;(void)m;(void)p;return CD_INVALID;}
enum ad_status ad_bfs(const struct cd_csr*g,unsigned s,unsigned f,unsigned*q,size_t n,struct ad_traversal*t){(void)g;(void)s;(void)f;(void)q;(void)n;(void)t;return AD_INVALID;}
enum ad_status ad_dijkstra(const struct cd_csr*g,unsigned s,uint64_t l,uint64_t d[8],unsigned p[8]){(void)g;(void)s;(void)l;(void)d;(void)p;return AD_INVALID;}
