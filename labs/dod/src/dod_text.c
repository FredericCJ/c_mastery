#include "dod_text.h"
int d_rope_index(const struct d_rope *a,unsigned b,size_t c,unsigned char *d){(void)a;(void)b;(void)c;(void)d;return D_FAILED;}
int d_rope_split(struct d_rope *a,unsigned b,size_t c,unsigned *d,unsigned *e){(void)a;(void)b;(void)c;(void)d;(void)e;return D_FAILED;}
int d_rope_balance(struct d_rope *a,unsigned b,unsigned *c){(void)a;(void)b;(void)c;return D_FAILED;}
int d_utf8_offset(const unsigned char *a,size_t b,size_t c,size_t *d){(void)a;(void)b;(void)c;(void)d;return D_FAILED;}
void d_gap_init(struct d_gap *a){(void)a;}
int d_gap_insert(struct d_gap *a,size_t b,const unsigned char *c,size_t d){(void)a;(void)b;(void)c;(void)d;return D_FAILED;}
int d_gap_delete(struct d_gap *a,size_t b,size_t c){(void)a;(void)b;(void)c;return D_FAILED;}
int d_gap_flatten(const struct d_gap *a,unsigned char *b,size_t c,size_t *d){(void)a;(void)b;(void)c;(void)d;return D_FAILED;}
int d_piece_init(struct d_piece_table *a,const unsigned char *b,size_t c,unsigned char *d,size_t e){(void)a;(void)b;(void)c;(void)d;(void)e;return D_FAILED;}
int d_piece_insert(struct d_piece_table *a,size_t b,const unsigned char *c,size_t d){(void)a;(void)b;(void)c;(void)d;return D_FAILED;}
int d_piece_delete(struct d_piece_table *a,size_t b,size_t c){(void)a;(void)b;(void)c;return D_FAILED;}
int d_piece_flatten(const struct d_piece_table *a,unsigned char *b,size_t c,size_t *d){(void)a;(void)b;(void)c;(void)d;return D_FAILED;}
int d_piece_compact(const struct d_piece_table *a,unsigned char *b,size_t c,unsigned char *d,size_t e,struct d_piece_table *f){(void)a;(void)b;(void)c;(void)d;(void)e;(void)f;return D_FAILED;}
int d_gap_scalar_edit(struct d_gap *a,size_t b,size_t c,const unsigned char *d,size_t e){(void)a;(void)b;(void)c;(void)d;(void)e;return D_FAILED;}
int d_piece_scalar_edit(struct d_piece_table *a,size_t b,size_t c,const unsigned char *d,size_t e){(void)a;(void)b;(void)c;(void)d;(void)e;return D_FAILED;}

int d_rope_splice(struct d_rope *r,unsigned root,size_t at,size_t remove,const unsigned char *p,size_t n,unsigned *out){(void)r;(void)root;(void)at;(void)remove;(void)p;(void)n;(void)out;return D_FAILED;}
