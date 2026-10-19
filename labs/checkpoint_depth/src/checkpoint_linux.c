#include "checkpoint_linux.h"
void cpl_init(CPLReader *r,unsigned s,unsigned l){(void)r;(void)s;(void)l;}
int cpl_open(CPLReader *r,int e,int f,int t){(void)r;(void)e;(void)f;(void)t;return -1;}
uint64_t cpl_token(const CPLReader *r){(void)r;return 0;}
CPLStatus cpl_pump(CPLReader *r,uint64_t t,CPAQueue *q){(void)r;(void)t;(void)q;return CPL_ERROR;}
int cpl_close(CPLReader *r){(void)r;return -1;}
