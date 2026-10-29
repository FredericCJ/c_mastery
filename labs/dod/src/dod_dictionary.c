#include "dod_dictionary.h"
int d_map_init(struct dh_map *m,struct dh_dictionary d){(void)m;(void)d;return D_FAILED;}
int d_map_update(struct dh_map *m,unsigned r,struct dh_key k,int v,unsigned *out,struct dh_metrics *cost){(void)m;(void)r;(void)k;(void)v;(void)out;(void)cost;return D_FAILED;}
int d_map_delete(struct dh_map *m,unsigned r,struct dh_key k,unsigned *out,struct dh_metrics *cost){(void)m;(void)r;(void)k;(void)out;(void)cost;return D_FAILED;}
int d_map_get(const struct dh_map *m,unsigned r,struct dh_key k,int *out,struct dh_metrics *cost){(void)m;(void)r;(void)k;(void)out;(void)cost;return D_FAILED;}
