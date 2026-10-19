#include "dod_persistent.h"
int d_zip_open(DTree *a,unsigned b,DZipper *c){(void)a;(void)b;(void)c;return D_FAILED;}
int d_zip_down(DZipper *a,unsigned b){(void)a;(void)b;return D_FAILED;}
int d_zip_up(DZipper *a){(void)a;return D_FAILED;}
int d_zip_write(const DZipper *a,int b,unsigned *c){(void)a;(void)b;(void)c;return D_FAILED;}
int d_tree_census(const DTree *a,const unsigned *b,size_t c,unsigned *d,unsigned *e){(void)a;(void)b;(void)c;(void)d;(void)e;return D_FAILED;}
int d_hamt_delete(DHamt *a,unsigned b,unsigned c,unsigned *d){(void)a;(void)b;(void)c;(void)d;return D_FAILED;}
void d_rc_init(DRcTree *a,DTextOps b){(void)a;(void)b;}
int d_rc_retain(DRcTree *a,unsigned b){(void)a;(void)b;return D_FAILED;}
int d_rc_release(DRcTree *a,unsigned b){(void)a;(void)b;return D_FAILED;}
int d_rc_update(DRcTree *a,unsigned b,unsigned c,const unsigned char *d,size_t e,unsigned *f){(void)a;(void)b;(void)c;(void)d;(void)e;(void)f;return D_FAILED;}
int d_rc_get(const DRcTree *a,unsigned b,unsigned c,const unsigned char **d,size_t *e){(void)a;(void)b;(void)c;(void)d;(void)e;return D_FAILED;}
int d_rc_retire(DRcTree *a,unsigned b,DRcRetire *c){(void)a;(void)b;(void)c;return D_FAILED;}
int d_rc_drain(DRcRetire *a,unsigned b,unsigned *c){(void)a;(void)b;(void)c;return D_FAILED;}
