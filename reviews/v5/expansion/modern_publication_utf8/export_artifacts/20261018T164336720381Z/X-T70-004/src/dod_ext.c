#include "dod_ext.h"
int d_rows_to_columns(const DRow *a,size_t b,DColumns8 *c){(void)a;(void)b;(void)c;return D_FAILED;}
int d_columns_to_rows(const DColumns8 *a,DRow *b,size_t c){(void)a;(void)b;(void)c;return D_FAILED;}
int d_rows_step(DRow *a,size_t b,int32_t c){(void)a;(void)b;(void)c;return D_FAILED;}
int d_columns8_step(DColumns8 *a,int32_t b){(void)a;(void)b;return D_FAILED;}
int d_rows_step_many(DRow *a,size_t b,int32_t c){(void)a;(void)b;(void)c;return D_FAILED;}
int d_columns_step_many(int32_t *a,const int32_t *b,size_t c,int32_t d){(void)a;(void)b;(void)c;(void)d;return D_FAILED;}
int d_rows_to_arrays(const DRow *r,size_t n,int32_t *x,int32_t *v,unsigned *c,size_t cap){(void)r;(void)n;(void)x;(void)v;(void)c;(void)cap;return D_FAILED;}
int d_arrays_to_rows(const int32_t *x,const int32_t *v,const unsigned *c,size_t n,DRow *r,size_t cap){(void)x;(void)v;(void)c;(void)n;(void)r;(void)cap;return D_FAILED;}
int d_rows_to_tiles(const DRow *a,size_t b,unsigned c,DTiles *d){(void)a;(void)b;(void)c;(void)d;return D_FAILED;}
int d_tiles_to_rows(const DTiles *a,DRow *b,size_t c){(void)a;(void)b;(void)c;return D_FAILED;}
uint32_t d_swar16_add(uint32_t a,uint32_t b){(void)a;(void)b;return 0;}
void d_segment_init(DSegment *a,DDestroy b,void *c){(void)a;(void)b;(void)c;}
int d_segment_add(DSegment *a,int b,void *c,DSegmentHandle *d){(void)a;(void)b;(void)c;(void)d;return D_FAILED;}
int d_segment_get(const DSegment *a,DSegmentHandle b,int *c){(void)a;(void)b;(void)c;return D_FAILED;}
int d_segment_borrow(const DSegment *a,DSegmentHandle b,const int **c){(void)a;(void)b;(void)c;return D_FAILED;}
int d_segment_remove(DSegment *a,DSegmentHandle b){(void)a;(void)b;return D_FAILED;}
void d_query_world_init(DQueryWorld *a){(void)a;}
int d_query_set(DQueryWorld *a,unsigned b,unsigned c,int d,int e){(void)a;(void)b;(void)c;(void)d;(void)e;return D_FAILED;}
int d_query_get(const DQueryWorld *a,unsigned b,unsigned c,DQuery *d){(void)a;(void)b;(void)c;(void)d;return D_FAILED;}
