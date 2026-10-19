#include "dod_dictionary.h"
int d_map_init(DHMap *m,DHDictionary d){(void)m;(void)d;return D_FAILED;}
int d_map_update(DHMap *m,unsigned r,DHKey k,int v,unsigned *out,DHMetrics *cost){(void)m;(void)r;(void)k;(void)v;(void)out;(void)cost;return D_FAILED;}
int d_map_delete(DHMap *m,unsigned r,DHKey k,unsigned *out,DHMetrics *cost){(void)m;(void)r;(void)k;(void)out;(void)cost;return D_FAILED;}
int d_map_get(const DHMap *m,unsigned r,DHKey k,int *out,DHMetrics *cost){(void)m;(void)r;(void)k;(void)out;(void)cost;return D_FAILED;}
