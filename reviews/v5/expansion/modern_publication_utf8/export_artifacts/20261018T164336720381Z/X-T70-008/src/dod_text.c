#include "dod_text.h"
int d_rope_index(const DRope *a,unsigned b,size_t c,unsigned char *d){(void)a;(void)b;(void)c;(void)d;return D_FAILED;}
int d_rope_split(DRope *a,unsigned b,size_t c,unsigned *d,unsigned *e){(void)a;(void)b;(void)c;(void)d;(void)e;return D_FAILED;}
int d_rope_balance(DRope *a,unsigned b,unsigned *c){(void)a;(void)b;(void)c;return D_FAILED;}
int d_utf8_offset(const unsigned char *a,size_t b,size_t c,size_t *d){(void)a;(void)b;(void)c;(void)d;return D_FAILED;}
void d_gap_init(DGap *a){(void)a;}
int d_gap_insert(DGap *a,size_t b,const unsigned char *c,size_t d){(void)a;(void)b;(void)c;(void)d;return D_FAILED;}
int d_gap_delete(DGap *a,size_t b,size_t c){(void)a;(void)b;(void)c;return D_FAILED;}
int d_gap_flatten(const DGap *a,unsigned char *b,size_t c,size_t *d){(void)a;(void)b;(void)c;(void)d;return D_FAILED;}
int d_piece_init(DPieceTable *a,const unsigned char *b,size_t c,unsigned char *d,size_t e){(void)a;(void)b;(void)c;(void)d;(void)e;return D_FAILED;}
int d_piece_insert(DPieceTable *a,size_t b,const unsigned char *c,size_t d){(void)a;(void)b;(void)c;(void)d;return D_FAILED;}
int d_piece_delete(DPieceTable *a,size_t b,size_t c){(void)a;(void)b;(void)c;return D_FAILED;}
int d_piece_flatten(const DPieceTable *a,unsigned char *b,size_t c,size_t *d){(void)a;(void)b;(void)c;(void)d;return D_FAILED;}
int d_piece_compact(const DPieceTable *a,unsigned char *b,size_t c,unsigned char *d,size_t e,DPieceTable *f){(void)a;(void)b;(void)c;(void)d;(void)e;(void)f;return D_FAILED;}
int d_gap_scalar_edit(DGap *a,size_t b,size_t c,const unsigned char *d,size_t e){(void)a;(void)b;(void)c;(void)d;(void)e;return D_FAILED;}
int d_piece_scalar_edit(DPieceTable *a,size_t b,size_t c,const unsigned char *d,size_t e){(void)a;(void)b;(void)c;(void)d;(void)e;return D_FAILED;}

int d_rope_splice(DRope *r,unsigned root,size_t at,size_t remove,const unsigned char *p,size_t n,unsigned *out){(void)r;(void)root;(void)at;(void)remove;(void)p;(void)n;(void)out;return D_FAILED;}
