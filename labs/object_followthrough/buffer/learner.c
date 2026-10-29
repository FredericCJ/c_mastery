#include "buffer.h"
int ob_construct(struct ob_construction *a,int b,void *c,char d,unsigned e){(void)a;(void)b;(void)c;(void)d;(void)e;return -1;}
int ob_record_equal(const struct ob_record *a,const struct ob_record *b){(void)a;(void)b;return 0;}
int ob_record_encode(const struct ob_record *a,unsigned char *b,size_t c){(void)a;(void)b;(void)c;return -1;}
int ob_cleanup(struct ob_construction *a,struct ob_allocator b){(void)a;(void)b;return -1;}
int ob_size(size_t a,size_t b,size_t c,size_t d,size_t *e){(void)a;(void)b;(void)c;(void)d;(void)e;return -1;}
int ob_create(struct ob_table *a,struct ob_allocator b,uint32_t c,size_t d){(void)a;(void)b;(void)c;(void)d;return -1;}
int ob_resize(struct ob_table *a,size_t b){(void)a;(void)b;return -1;}
int ob_append(struct ob_table *a,unsigned char b){(void)a;(void)b;return -1;}
int ob_erase(struct ob_table *a,size_t b){(void)a;(void)b;return -1;}
void ob_reset(struct ob_table *a){(void)a;}
int ob_handle(const struct ob_table *a,size_t b,struct ob_handle *c){(void)a;(void)b;(void)c;return -1;}
int ob_resolve(const struct ob_table *a,struct ob_handle b,unsigned char *c){(void)a;(void)b;(void)c;return -1;}
int ob_table_move(struct ob_table *a,struct ob_table *b){(void)a;(void)b;return -1;}
int ob_split_create(struct ob_allocator a,size_t b,struct ob_split **c){(void)a;(void)b;(void)c;return -1;}
void ob_split_drop(struct ob_split **a){(void)a;}
int ob_shared_create(struct ob_allocator a,const unsigned char *b,size_t c,struct ob_shared **d){(void)a;(void)b;(void)c;(void)d;return -1;}
int ob_shared_retain(struct ob_shared *a,struct ob_shared **b){(void)a;(void)b;return -1;}
void ob_shared_drop(struct ob_shared **a){(void)a;}
int ob_workspace(int a,int *b,size_t c,int *d,unsigned *e){(void)a;(void)b;(void)c;(void)d;(void)e;return -1;}
