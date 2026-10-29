#include "arena_flow.h"
enum adf_status adf_arena(const struct ad_node *n,size_t count,struct adf_arena *out){(void)n;(void)count;(void)out;return ADF_INVALID;}
enum adf_status adf_root(const struct adf_arena *a,struct ad_node_index r,uint32_t *o){(void)a;(void)r;(void)o;return ADF_INVALID;}
enum adf_status adf_start(const struct adf_node *n,unsigned c,struct adf_session *o){(void)n;(void)c;(void)o;return ADF_INVALID;}
enum adf_status adf_step(struct adf_session *s,unsigned b,struct adf_result *o){(void)s;(void)b;(void)o;return ADF_INVALID;}
enum adf_status adf_boolean(const struct adf_boolean *n,unsigned c,unsigned b,unsigned *o){(void)n;(void)c;(void)b;(void)o;return ADF_INVALID;}
