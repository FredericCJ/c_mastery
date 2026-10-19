#include "clinic.h"
int mc_acquire(McOwner *a,McAllocator b,int c){(void)a;(void)b;(void)c;return -1;}
int mc_read(const McOwner *a,int *b){(void)a;(void)b;return -1;}
int mc_snapshot(const McOwner *a,int *b){(void)a;(void)b;return -1;}
int mc_release(McOwner *a){(void)a;return -1;}
FDStatus mc_move(unsigned char *a,size_t b,size_t c,size_t d,size_t e){(void)a;(void)b;(void)c;(void)d;(void)e;return FD_INVALID;}
int mc_member(const McContainer *a,const int *b,int *c){(void)a;(void)b;(void)c;return -1;}
int mc_initial(const McFirst *a,const int *b,int *c){(void)a;(void)b;(void)c;return -1;}
