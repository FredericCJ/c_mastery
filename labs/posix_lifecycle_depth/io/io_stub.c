#include "io.h"
#include <errno.h>
void pc_init(struct pc_copy *c){(void)c;}
enum pc_status pc_resume(struct pc_copy *c,struct pc_ops o,unsigned b){(void)c;(void)o;(void)b;return PC_ERROR;}
uintmax_t pc_off_max(void){return 0;}
int pc_extent(uintmax_t o,uintmax_t n,uintmax_t f,off_t *p){(void)o;(void)n;(void)f;(void)p;return ENOSYS;}
int pc_write_record(int f,uintmax_t o,const unsigned char *d,size_t n,uintmax_t s,size_t *c){(void)f;(void)o;(void)d;(void)n;(void)s;(void)c;return ENOSYS;}
int pc_close_transition(bool *o,int e,enum pc_close_policy p){(void)o;(void)e;(void)p;return ENOSYS;}
