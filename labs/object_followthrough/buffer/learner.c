#include "buffer.h"
int ob_construct(ObConstruction *a,int b,void *c,char d,unsigned e){(void)a;(void)b;(void)c;(void)d;(void)e;return -1;}
int ob_record_equal(const ObRecord *a,const ObRecord *b){(void)a;(void)b;return 0;}
int ob_record_encode(const ObRecord *a,unsigned char *b,size_t c){(void)a;(void)b;(void)c;return -1;}
int ob_cleanup(ObConstruction *a,ObAllocator b){(void)a;(void)b;return -1;}
int ob_size(size_t a,size_t b,size_t c,size_t d,size_t *e){(void)a;(void)b;(void)c;(void)d;(void)e;return -1;}
int ob_create(ObTable *a,ObAllocator b,uint32_t c,size_t d){(void)a;(void)b;(void)c;(void)d;return -1;}
int ob_resize(ObTable *a,size_t b){(void)a;(void)b;return -1;}
int ob_append(ObTable *a,unsigned char b){(void)a;(void)b;return -1;}
int ob_erase(ObTable *a,size_t b){(void)a;(void)b;return -1;}
void ob_reset(ObTable *a){(void)a;}
int ob_handle(const ObTable *a,size_t b,ObHandle *c){(void)a;(void)b;(void)c;return -1;}
int ob_resolve(const ObTable *a,ObHandle b,unsigned char *c){(void)a;(void)b;(void)c;return -1;}
int ob_table_move(ObTable *a,ObTable *b){(void)a;(void)b;return -1;}
int ob_split_create(ObAllocator a,size_t b,ObSplit **c){(void)a;(void)b;(void)c;return -1;}
void ob_split_drop(ObSplit **a){(void)a;}
int ob_shared_create(ObAllocator a,const unsigned char *b,size_t c,ObShared **d){(void)a;(void)b;(void)c;(void)d;return -1;}
int ob_shared_retain(ObShared *a,ObShared **b){(void)a;(void)b;return -1;}
void ob_shared_drop(ObShared **a){(void)a;}
int ob_workspace(int a,int *b,size_t c,int *d,unsigned *e){(void)a;(void)b;(void)c;(void)d;(void)e;return -1;}
