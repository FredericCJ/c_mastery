#include "cursor.h"
int oc_rows_read(void *a,size_t b,size_t c,int *d){(void)a;(void)b;(void)c;(void)d;return -1;}
int oc_table_read(void *a,size_t b,size_t c,int *d){(void)a;(void)b;(void)c;(void)d;return -1;}
int oc_read(const OcMatrix *a,size_t b,size_t c,int *d){(void)a;(void)b;(void)c;(void)d;return -1;}
int oc_representation(int (*a)[2][3],unsigned char *b,size_t c){(void)a;(void)b;(void)c;return -1;}
FDStatus oc_take(FDCursor *a,size_t b,const unsigned char **c){(void)a;(void)b;(void)c;return FD_NOT_IMPLEMENTED;}
FDStatus oc_subview(const FDCursor *a,size_t b,size_t c,FDCursor *d){(void)a;(void)b;(void)c;(void)d;return FD_NOT_IMPLEMENTED;}
FDStatus oc_previous(FDCursor *a,size_t b,const unsigned char **c){(void)a;(void)b;(void)c;return FD_NOT_IMPLEMENTED;}
int oc_difference(const unsigned char *a,size_t b,size_t c,size_t d,ptrdiff_t *e){(void)a;(void)b;(void)c;(void)d;(void)e;return -1;}
int oc_order(const size_t *a,size_t b,OcPosition c,OcPosition d,int *e){(void)a;(void)b;(void)c;(void)d;(void)e;return -1;}
int oc_encode_index(int *a,size_t b,const int *c,unsigned char *d){(void)a;(void)b;(void)c;(void)d;return -1;}
int oc_decode_index(int *a,size_t b,const unsigned char *c,int **d){(void)a;(void)b;(void)c;(void)d;return -1;}
int oc_roundtrip(int *a){(void)a;return -1;}
