#include "path.h"
#include <errno.h>
int pn_open_confined(int d,const char *p,int *o){(void)d;(void)p;(void)o;return ENOSYS;}
int pn_info(int d,const char *p,bool f,struct pn_info *o){(void)d;(void)p;(void)f;(void)o;return ENOSYS;}
int pn_creation_mode(unsigned r,unsigned m,bool a,unsigned d,unsigned *o){(void)r;(void)m;(void)a;(void)d;(void)o;return ENOSYS;}
int pn_authorize(struct pn_authority a,enum pn_operation o){(void)a;(void)o;return ENOSYS;}
