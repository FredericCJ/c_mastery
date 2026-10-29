#include "dod_persistent.h"
int d_zip_open(struct d_tree *a,unsigned b,struct d_zipper *c){(void)a;(void)b;(void)c;return D_FAILED;}
int d_zip_down(struct d_zipper *a,unsigned b){(void)a;(void)b;return D_FAILED;}
int d_zip_up(struct d_zipper *a){(void)a;return D_FAILED;}
int d_zip_write(const struct d_zipper *a,int b,unsigned *c){(void)a;(void)b;(void)c;return D_FAILED;}
int d_tree_census(const struct d_tree *a,const unsigned *b,size_t c,unsigned *d,unsigned *e){(void)a;(void)b;(void)c;(void)d;(void)e;return D_FAILED;}
int d_hamt_delete(struct d_hamt *a,unsigned b,unsigned c,unsigned *d){(void)a;(void)b;(void)c;(void)d;return D_FAILED;}
void d_rc_init(struct d_rc_tree *a,struct d_text_ops b){(void)a;(void)b;}
int d_rc_retain(struct d_rc_tree *a,unsigned b){(void)a;(void)b;return D_FAILED;}
int d_rc_release(struct d_rc_tree *a,unsigned b){(void)a;(void)b;return D_FAILED;}
int d_rc_update(struct d_rc_tree *a,unsigned b,unsigned c,const unsigned char *d,size_t e,unsigned *f){(void)a;(void)b;(void)c;(void)d;(void)e;(void)f;return D_FAILED;}
int d_rc_get(const struct d_rc_tree *a,unsigned b,unsigned c,const unsigned char **d,size_t *e){(void)a;(void)b;(void)c;(void)d;(void)e;return D_FAILED;}
int d_rc_retire(struct d_rc_tree *a,unsigned b,struct d_rc_retire *c){(void)a;(void)b;(void)c;return D_FAILED;}
int d_rc_drain(struct d_rc_retire *a,unsigned b,unsigned *c){(void)a;(void)b;(void)c;return D_FAILED;}
